#include "system/StartupManager.h"

#include "core/Logger.h"

#include <windows.h>
#include <appmodel.h>

#include <algorithm>
#include <cstring>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace
{
    using real_optimizer::system::StartupApp;
    using real_optimizer::system::StartupChange;
    using real_optimizer::system::StartupOutcome;
    using real_optimizer::system::StartupRequest;
    using real_optimizer::system::StartupSource;
    using real_optimizer::system::StartupTarget;

    constexpr const wchar_t *RunKeyPath =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

    constexpr const wchar_t *RunKey32Path =
        L"Software\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Run";

    constexpr const wchar_t *ApprovedRunPath =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\"
        L"Explorer\\StartupApproved\\Run";

    constexpr const wchar_t *ApprovedRun32Path =
        L"Software\\Microsoft\\Windows\\CurrentVersion\\"
        L"Explorer\\StartupApproved\\Run32";

    // Apps da Microsoft Store: cada tarefa de inicialização tem uma
    // subchave <pacote>\<tarefa> com o valor "State":
    // 1 = desativado pelo usuário, 2 = ativado (0, 3 e 4 são
    // controlados pelo app ou por política).
    constexpr const wchar_t *PackagedRootPath =
        L"Software\\Classes\\Local Settings\\Software\\Microsoft\\"
        L"Windows\\CurrentVersion\\AppModel\\SystemAppData";

    HKEY RootFor(StartupSource source) noexcept
    {
        return (source == StartupSource::CurrentUser ||
                source == StartupSource::PackagedApp)
                   ? HKEY_CURRENT_USER
                   : HKEY_LOCAL_MACHINE;
    }

    // As chaves de máquina sempre são abertas na visão de 64 bits;
    // o caminho WOW6432Node é escrito por extenso.
    REGSAM ViewFor(StartupSource source) noexcept
    {
        return (source == StartupSource::CurrentUser ||
                source == StartupSource::PackagedApp)
                   ? 0
                   : KEY_WOW64_64KEY;
    }

    const wchar_t *RunPathFor(StartupSource source) noexcept
    {
        return source == StartupSource::LocalMachine32
                   ? RunKey32Path
                   : RunKeyPath;
    }

    const wchar_t *ApprovedPathFor(StartupSource source) noexcept
    {
        return source == StartupSource::LocalMachine32
                   ? ApprovedRun32Path
                   : ApprovedRunPath;
    }

    //
    // LEITURA
    //

    // StartupApproved: o primeiro byte par (02, 06) = habilitado,
    // ímpar (03, 07) = desabilitado. Sem registro = habilitado.
    bool IsStartupAppEnabled(
        StartupSource source,
        const std::wstring &appName)
    {
        HKEY key = nullptr;

        const LONG openResult =
            RegOpenKeyExW(
                RootFor(source),
                ApprovedPathFor(source),
                0,
                KEY_READ | ViewFor(source),
                &key);

        if (openResult != ERROR_SUCCESS)
        {
            return true;
        }

        BYTE data[32]{};
        DWORD dataSize = sizeof(data);
        DWORD valueType = 0;

        const LONG queryResult =
            RegQueryValueExW(
                key,
                appName.c_str(),
                nullptr,
                &valueType,
                data,
                &dataSize);

        RegCloseKey(key);

        if (
            queryResult != ERROR_SUCCESS ||
            valueType != REG_BINARY ||
            dataSize == 0)
        {
            return true;
        }

        return (data[0] & 0x01) == 0;
    }

    void ReadStartupRegistryKey(
        StartupSource source,
        std::vector<StartupApp> &apps)
    {
        HKEY key = nullptr;

        const LONG openResult =
            RegOpenKeyExW(
                RootFor(source),
                RunPathFor(source),
                0,
                KEY_READ | ViewFor(source),
                &key);

        if (openResult != ERROR_SUCCESS)
        {
            return;
        }

        DWORD valueIndex = 0;

        while (true)
        {
            wchar_t valueName[512]{};
            wchar_t valueData[4096]{};

            DWORD valueNameLength =
                static_cast<DWORD>(
                    std::size(valueName));

            DWORD valueDataSize =
                sizeof(valueData) - sizeof(wchar_t);

            DWORD valueType = 0;

            const LONG result =
                RegEnumValueW(
                    key,
                    valueIndex,
                    valueName,
                    &valueNameLength,
                    nullptr,
                    &valueType,
                    reinterpret_cast<LPBYTE>(
                        valueData),
                    &valueDataSize);

            if (result == ERROR_NO_MORE_ITEMS)
            {
                break;
            }

            if (
                result == ERROR_SUCCESS &&
                (valueType == REG_SZ ||
                 valueType == REG_EXPAND_SZ))
            {
                StartupApp app{};

                app.name = valueName;
                app.command = valueData;
                app.source = source;
                app.enabled = IsStartupAppEnabled(source, app.name);

                apps.push_back(std::move(app));
            }

            ++valueIndex;
        }

        RegCloseKey(key);
    }

    // O Windows mantém chaves de inicialização de pacotes que já foram
    // desinstalados. O Gerenciador de Tarefas não mostra esses restos,
    // então o UeX também não: só entram pacotes instalados para este
    // usuário. Se a checagem não for possível, a entrada é mantida.
    bool IsPackageInstalled(const wchar_t *familyName)
    {
        using Fn = LONG(WINAPI *)(PCWSTR, UINT32 *, PWSTR *, UINT32 *, PWSTR);

        static const Fn function = []() -> Fn
        {
            const HMODULE kernel = GetModuleHandleW(L"kernel32.dll");

            if (kernel == nullptr)
            {
                return nullptr;
            }

            return reinterpret_cast<Fn>(reinterpret_cast<void *>(
                GetProcAddress(kernel, "GetPackagesByPackageFamily")));
        }();

        if (function == nullptr)
        {
            return true;
        }

        UINT32 count = 0;
        UINT32 length = 0;

        const LONG result =
            function(familyName, &count, nullptr, &length, nullptr);

        if (result == ERROR_SUCCESS)
        {
            return count > 0;
        }

        return true;
    }

    bool ReadPackagedState(
        HKEY parent,
        const wchar_t *subKey,
        DWORD &state)
    {
        HKEY key = nullptr;

        if (RegOpenKeyExW(parent, subKey, 0, KEY_READ, &key) !=
            ERROR_SUCCESS)
        {
            return false;
        }

        DWORD type = 0;
        DWORD size = sizeof(state);

        const LONG result = RegQueryValueExW(
            key,
            L"State",
            nullptr,
            &type,
            reinterpret_cast<LPBYTE>(&state),
            &size);

        RegCloseKey(key);

        return result == ERROR_SUCCESS && type == REG_DWORD;
    }

    void ReadPackagedApps(std::vector<StartupApp> &apps)
    {
        HKEY root = nullptr;

        if (RegOpenKeyExW(
                HKEY_CURRENT_USER,
                PackagedRootPath,
                0,
                KEY_READ,
                &root) != ERROR_SUCCESS)
        {
            return;
        }

        for (DWORD i = 0;; ++i)
        {
            wchar_t package[256]{};
            DWORD packageLength = static_cast<DWORD>(std::size(package));

            const LONG enumResult = RegEnumKeyExW(
                root, i, package, &packageLength,
                nullptr, nullptr, nullptr, nullptr);

            if (enumResult == ERROR_NO_MORE_ITEMS)
            {
                break;
            }

            if (enumResult != ERROR_SUCCESS)
            {
                continue;
            }

            // O Windows deixa chaves de pacotes que já foram desinstalados.
            // O Gerenciador de Tarefas não mostra esses; o UeX também não.
            // Se a consulta falhar por outro motivo, o pacote é mantido.
            {
                UINT32 packageCount = 0;
                UINT32 bufferLength = 0;

                const LONG familyResult = GetPackagesByPackageFamily(
                    package,
                    &packageCount,
                    nullptr,
                    &bufferLength,
                    nullptr);

                if (familyResult == ERROR_SUCCESS && packageCount == 0)
                {
                    continue;
                }
            }

            if (!IsPackageInstalled(package))
            {
                continue;
            }

            HKEY packageKey = nullptr;

            if (RegOpenKeyExW(root, package, 0, KEY_READ, &packageKey) !=
                ERROR_SUCCESS)
            {
                continue;
            }

            for (DWORD j = 0;; ++j)
            {
                wchar_t task[256]{};
                DWORD taskLength = static_cast<DWORD>(std::size(task));

                const LONG taskResult = RegEnumKeyExW(
                    packageKey, j, task, &taskLength,
                    nullptr, nullptr, nullptr, nullptr);

                if (taskResult == ERROR_NO_MORE_ITEMS)
                {
                    break;
                }

                if (taskResult != ERROR_SUCCESS)
                {
                    continue;
                }

                DWORD state = 0;

                if (!ReadPackagedState(packageKey, task, state))
                {
                    continue;
                }

                StartupApp app{};

                app.name = std::wstring(package) + L"\\" + task;
                app.source = StartupSource::PackagedApp;
                app.enabled = state == 2 || state == 4;
                // 0 = ainda não ligado (o usuário pode ligar, como em
                // Configurações > Aplicativos > Inicialização),
                // 1 = desativado pelo usuário, 2 = ativado.
                // 3 e 4 vêm de política: o UeX não mexe.
                app.locked = state != 0 && state != 1 && state != 2;

                if (app.locked)
                {
                    // Mostra o número do estado para facilitar o diagnóstico.
                    app.lockReason =
                        (state == 3 || state == 4)
                            ? L"Por política (estado " + std::to_wstring(state) + L")"
                            : L"Estado desconhecido (" + std::to_wstring(state) + L")";
                }

                apps.push_back(std::move(app));
            }

            RegCloseKey(packageKey);
        }

        RegCloseKey(root);
    }

    LONG WritePackagedState(const std::wstring &name, bool enable)
    {
        const std::wstring path =
            std::wstring(PackagedRootPath) + L"\\" + name;

        HKEY key = nullptr;

        const LONG openResult = RegOpenKeyExW(
            HKEY_CURRENT_USER, path.c_str(), 0, KEY_SET_VALUE, &key);

        if (openResult != ERROR_SUCCESS)
        {
            return openResult;
        }

        const DWORD state = enable ? 2 : 1;

        const LONG result = RegSetValueExW(
            key,
            L"State",
            0,
            REG_DWORD,
            reinterpret_cast<const BYTE *>(&state),
            sizeof(state));

        RegCloseKey(key);

        return result;
    }

    bool EqualsInsensitive(
        const std::wstring &a,
        const std::wstring &b)
    {
        if (a.size() != b.size())
        {
            return false;
        }

        for (std::size_t i = 0; i < a.size(); ++i)
        {
            if (std::towlower(a[i]) != std::towlower(b[i]))
            {
                return false;
            }
        }

        return true;
    }

    bool SameTarget(
        const StartupTarget &a,
        const StartupTarget &b)
    {
        // O Windows compara nomes de valor sem diferenciar maiúsculas.
        return a.source == b.source &&
               EqualsInsensitive(a.name, b.name);
    }

    //
    // GRAVAÇÃO (StartupApproved)
    //

    // Mesmo formato que o Gerenciador de Tarefas usa: 12 bytes,
    // primeiro DWORD 02 (habilitado) ou 03 (desabilitado) e, ao
    // desabilitar, a hora em FILETIME nos 8 bytes seguintes.
    LONG WriteApproved(
        StartupSource source,
        const std::wstring &name,
        bool enable)
    {
        HKEY key = nullptr;

        const LONG createResult =
            RegCreateKeyExW(
                RootFor(source),
                ApprovedPathFor(source),
                0,
                nullptr,
                REG_OPTION_NON_VOLATILE,
                KEY_SET_VALUE | ViewFor(source),
                nullptr,
                &key,
                nullptr);

        if (createResult != ERROR_SUCCESS)
        {
            return createResult;
        }

        BYTE data[12]{};

        if (enable)
        {
            data[0] = 0x02;
        }
        else
        {
            data[0] = 0x03;

            FILETIME now{};
            GetSystemTimeAsFileTime(&now);

            std::memcpy(data + 4, &now, sizeof(now));
        }

        const LONG setResult =
            RegSetValueExW(
                key,
                name.c_str(),
                0,
                REG_BINARY,
                data,
                static_cast<DWORD>(sizeof(data)));

        RegCloseKey(key);

        return setResult;
    }

    //
    // BACKUP DO QUE O UeX DESATIVOU
    //
    // Uma linha por entrada, em UTF-8:  <U|M|W><tab><nome>
    //

    std::wstring ExpandVariables(const std::wstring &text)
    {
        const DWORD needed =
            ExpandEnvironmentStringsW(text.c_str(), nullptr, 0);

        if (needed == 0)
        {
            return {};
        }

        std::wstring result(needed, L'\0');

        const DWORD written = ExpandEnvironmentStringsW(
            text.c_str(),
            result.data(),
            needed);

        if (written == 0 || written > needed)
        {
            return {};
        }

        result.resize(written - 1);
        return result;
    }

    std::wstring BackupPath()
    {
        return ExpandVariables(
            L"%LOCALAPPDATA%\\UltraEdgeX\\startup-backup.txt");
    }

    std::string ToUtf8(const std::wstring &text)
    {
        if (text.empty())
        {
            return {};
        }

        const int size = WideCharToMultiByte(
            CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
            nullptr, 0, nullptr, nullptr);

        if (size <= 0)
        {
            return {};
        }

        std::string result(static_cast<std::size_t>(size), '\0');

        WideCharToMultiByte(
            CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
            result.data(), size, nullptr, nullptr);

        return result;
    }

    std::wstring FromUtf8(const std::string &text)
    {
        if (text.empty())
        {
            return {};
        }

        const int size = MultiByteToWideChar(
            CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
            nullptr, 0);

        if (size <= 0)
        {
            return {};
        }

        std::wstring result(static_cast<std::size_t>(size), L'\0');

        MultiByteToWideChar(
            CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
            result.data(), size);

        return result;
    }

    char SourceCode(StartupSource source) noexcept
    {
        switch (source)
        {
        case StartupSource::CurrentUser:
            return 'U';
        case StartupSource::LocalMachine:
            return 'M';
        case StartupSource::PackagedApp:
            return 'P';
        case StartupSource::LocalMachine32:
        default:
            return 'W';
        }
    }

    bool SourceFromCode(char code, StartupSource &source) noexcept
    {
        switch (code)
        {
        case 'U':
            source = StartupSource::CurrentUser;
            return true;
        case 'M':
            source = StartupSource::LocalMachine;
            return true;
        case 'W':
            source = StartupSource::LocalMachine32;
            return true;
        case 'P':
            source = StartupSource::PackagedApp;
            return true;
        default:
            return false;
        }
    }

    std::vector<StartupTarget> ReadBackup()
    {
        std::vector<StartupTarget> entries;

        const std::wstring path = BackupPath();

        if (path.empty())
        {
            return entries;
        }

        std::ifstream input{std::filesystem::path(path)};
        std::string line;

        while (std::getline(input, line))
        {
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            if (line.size() < 3 || line[1] != '\t')
            {
                continue;
            }

            StartupTarget target;

            if (!SourceFromCode(line[0], target.source))
            {
                continue;
            }

            target.name = FromUtf8(line.substr(2));

            if (!target.name.empty())
            {
                entries.push_back(std::move(target));
            }
        }

        return entries;
    }

    bool WriteBackup(const std::vector<StartupTarget> &entries)
    {
        const std::wstring path = BackupPath();

        if (path.empty())
        {
            return false;
        }

        std::error_code ignored;

        std::filesystem::create_directories(
            std::filesystem::path(path).parent_path(),
            ignored);

        std::ofstream output(
            std::filesystem::path(path),
            std::ios::trunc | std::ios::binary);

        if (!output)
        {
            return false;
        }

        for (const StartupTarget &entry : entries)
        {
            output << SourceCode(entry.source) << '\t'
                   << ToUtf8(entry.name) << '\n';
        }

        return static_cast<bool>(output);
    }

    bool InBackup(
        const std::vector<StartupTarget> &backup,
        const StartupTarget &target)
    {
        return std::any_of(
            backup.begin(),
            backup.end(),
            [&](const StartupTarget &entry)
            { return SameTarget(entry, target); });
    }

    void RemoveFromBackup(
        std::vector<StartupTarget> &backup,
        const StartupTarget &target)
    {
        backup.erase(
            std::remove_if(
                backup.begin(),
                backup.end(),
                [&](const StartupTarget &entry)
                { return SameTarget(entry, target); }),
            backup.end());
    }

    bool CanWrite(StartupSource source, bool processIsAdmin) noexcept
    {
        return !(source == StartupSource::LocalMachine ||
                 source == StartupSource::LocalMachine32) ||
               processIsAdmin;
    }

    bool IsProcessElevated()
    {
        HANDLE token = nullptr;

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
        {
            return false;
        }

        TOKEN_ELEVATION elevation{};
        DWORD size = 0;

        const BOOL ok = GetTokenInformation(
            token,
            TokenElevation,
            &elevation,
            sizeof(elevation),
            &size);

        CloseHandle(token);

        return ok != FALSE && elevation.TokenIsElevated != 0;
    }
}

namespace real_optimizer::system
{
    std::vector<StartupApp>
    StartupManager::GetStartupApps()
    {
        std::vector<StartupApp> apps;

        // Aplicativos do usuário atual.
        ReadStartupRegistryKey(StartupSource::CurrentUser, apps);

        // Aplicativos configurados para toda a máquina.
        ReadStartupRegistryKey(StartupSource::LocalMachine, apps);

        // Aplicativos de 32 bits configurados para toda a máquina.
        ReadStartupRegistryKey(StartupSource::LocalMachine32, apps);

        // Aplicativos da Microsoft Store (Terminal, Xbox, Teams novo...).
        ReadPackagedApps(apps);

        return apps;
    }

    bool StartupManager::RequiresAdministrator(
        StartupSource source) noexcept
    {
        return source == StartupSource::LocalMachine ||
               source == StartupSource::LocalMachine32;
    }

    std::vector<StartupTarget> StartupManager::BackedUp()
    {
        return ReadBackup();
    }

    std::vector<StartupChange> StartupManager::Execute(
        const StartupRequest &request)
    {
        std::vector<StartupChange> changes;

        const bool elevated = IsProcessElevated();

        // Estado atual real: só entradas que existem são alteradas.
        const std::vector<StartupApp> current = GetStartupApps();

        auto findCurrent = [&](const StartupTarget &target)
            -> const StartupApp *
        {
            for (const StartupApp &app : current)
            {
                if (app.source == target.source &&
                    EqualsInsensitive(app.name, target.name))
                {
                    return &app;
                }
            }

            return nullptr;
        };

        std::vector<StartupTarget> backup = ReadBackup();
        bool backupDirty = false;

        auto apply = [&](const StartupTarget &target, bool enable)
        {
            StartupChange change;
            change.target = target;

            const StartupApp *app = findCurrent(target);

            if (app == nullptr)
            {
                change.outcome = StartupOutcome::NotFound;

                // Entrada que sumiu não precisa mais de restauração.
                if (enable)
                {
                    RemoveFromBackup(backup, target);
                    backupDirty = true;
                }

                changes.push_back(std::move(change));
                return;
            }

            if (app->locked)
            {
                // Controlado pelo sistema: o UeX não mexe.
                change.outcome = StartupOutcome::Failed;
                change.error = ERROR_ACCESS_DENIED;
                changes.push_back(std::move(change));
                return;
            }

            if (!CanWrite(target.source, elevated))
            {
                change.outcome = StartupOutcome::Failed;
                change.error = ERROR_ACCESS_DENIED;
                changes.push_back(std::move(change));
                return;
            }

            if (app->enabled == enable)
            {
                change.outcome = StartupOutcome::Unchanged;

                if (enable)
                {
                    RemoveFromBackup(backup, target);
                    backupDirty = true;
                }

                changes.push_back(std::move(change));
                return;
            }

            bool addedToBackup = false;

            if (!enable)
            {
                // Backup ANTES de alterar. Sem backup, não altera.
                if (!InBackup(backup, target))
                {
                    backup.push_back(target);
                    addedToBackup = true;
                }

                if (!WriteBackup(backup))
                {
                    if (addedToBackup)
                    {
                        RemoveFromBackup(backup, target);
                    }

                    change.outcome = StartupOutcome::Failed;
                    change.error = ERROR_WRITE_FAULT;
                    changes.push_back(std::move(change));
                    return;
                }
            }

            const LONG result =
                target.source == StartupSource::PackagedApp
                    ? WritePackagedState(target.name, enable)
                    : WriteApproved(target.source, target.name, enable);

            if (result != ERROR_SUCCESS)
            {
                change.outcome = StartupOutcome::Failed;
                change.error = static_cast<unsigned long>(result);

                // Não mudou nada: tira do backup o que acabou de entrar.
                if (addedToBackup)
                {
                    RemoveFromBackup(backup, target);
                    WriteBackup(backup);
                }

                changes.push_back(std::move(change));
                return;
            }

            change.outcome = StartupOutcome::Changed;

            if (enable)
            {
                RemoveFromBackup(backup, target);
                WriteBackup(backup);
            }

            core::Logger::Info(
                std::wstring(enable ? L"Inicialização reativada: "
                                    : L"Inicialização desativada: ") +
                target.name);

            changes.push_back(std::move(change));
        };

        for (const StartupTarget &target : request.disable)
        {
            apply(target, false);
        }

        for (const StartupTarget &target : request.enable)
        {
            apply(target, true);
        }

        // Restaurar: reativa o que o UeX desativou, no escopo pedido.
        const std::vector<StartupTarget> toRestore = backup;

        for (const StartupTarget &target : toRestore)
        {
            // Escopo "todos os usuários" (HKLM) ou escopo do usuário
            // (chaves Run do usuário e apps da Microsoft Store).
            const bool wanted =
                (target.source == StartupSource::LocalMachine ||
                 target.source == StartupSource::LocalMachine32)
                    ? request.restoreMachine
                    : request.restoreCurrentUser;

            if (wanted)
            {
                apply(target, true);
            }
        }

        if (backupDirty)
        {
            WriteBackup(backup);
        }

        return changes;
    }
}
