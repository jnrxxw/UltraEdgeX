#include "system/Services.h"

#include "core/Logger.h"

#include <windows.h>

#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

namespace real_optimizer::system
{
    namespace
    {
        struct ScHandle
        {
            SC_HANDLE handle = nullptr;

            explicit ScHandle(SC_HANDLE h) : handle(h) {}
            ~ScHandle()
            {
                if (handle != nullptr)
                {
                    CloseServiceHandle(handle);
                }
            }

            ScHandle(const ScHandle&) = delete;
            ScHandle& operator=(const ScHandle&) = delete;
        };

        ServiceStartType FromWindows(DWORD startType, bool delayed)
        {
            switch (startType)
            {
            case SERVICE_AUTO_START:
                return delayed ? ServiceStartType::AutomaticDelayed
                               : ServiceStartType::Automatic;
            case SERVICE_DEMAND_START:
                return ServiceStartType::Manual;
            case SERVICE_DISABLED:
                return ServiceStartType::Disabled;
            case SERVICE_BOOT_START:
            case SERVICE_SYSTEM_START:
                return ServiceStartType::Automatic;
            default:
                return ServiceStartType::Unknown;
            }
        }

        bool ReadConfig(SC_HANDLE service, DWORD& startType, bool& delayed)
        {
            DWORD needed = 0;

            QueryServiceConfigW(service, nullptr, 0, &needed);

            if (needed == 0)
            {
                return false;
            }

            std::vector<BYTE> buffer(needed);

            auto* config =
                reinterpret_cast<QUERY_SERVICE_CONFIGW*>(buffer.data());

            if (!QueryServiceConfigW(service, config, needed, &needed))
            {
                return false;
            }

            startType = config->dwStartType;
            delayed = false;

            SERVICE_DELAYED_AUTO_START_INFO info{};
            DWORD infoSize = 0;

            if (QueryServiceConfig2W(
                    service,
                    SERVICE_CONFIG_DELAYED_AUTO_START_INFO,
                    reinterpret_cast<LPBYTE>(&info),
                    sizeof(info),
                    &infoSize))
            {
                delayed = info.fDelayedAutostart != FALSE;
            }

            return true;
        }

        bool IsRunning(SC_HANDLE service)
        {
            SERVICE_STATUS_PROCESS status{};
            DWORD needed = 0;

            if (!QueryServiceStatusEx(
                    service,
                    SC_STATUS_PROCESS_INFO,
                    reinterpret_cast<LPBYTE>(&status),
                    sizeof(status),
                    &needed))
            {
                return false;
            }

            return status.dwCurrentState != SERVICE_STOPPED;
        }

        // Tenta parar o serviço (até ~10 s). Falhar não é erro:
        // a configuração já foi alterada e vale a partir do reinício.
        void StopIfRunning(SC_HANDLE service)
        {
            if (!IsRunning(service))
            {
                return;
            }

            SERVICE_STATUS status{};
            ControlService(service, SERVICE_CONTROL_STOP, &status);

            for (int i = 0; i < 40 && IsRunning(service); ++i)
            {
                Sleep(250);
            }
        }

        bool HasActiveDependents(SC_HANDLE service)
        {
            DWORD bytes = 0;
            DWORD count = 0;

            const BOOL ok = EnumDependentServicesW(
                service,
                SERVICE_ACTIVE,
                nullptr,
                0,
                &bytes,
                &count);

            // Sem dependentes ativos a chamada "dá certo" com 0 itens.
            return !ok && GetLastError() == ERROR_MORE_DATA;
        }

        // ---- Backup da configuração original ----

        struct BackupEntry
        {
            DWORD startType = SERVICE_DEMAND_START;
            bool delayed = false;
        };

        std::wstring BackupPath()
        {
            wchar_t buffer[MAX_PATH]{};

            const DWORD length = GetEnvironmentVariableW(
                L"LOCALAPPDATA", buffer, MAX_PATH);

            if (length == 0 || length >= MAX_PATH)
            {
                return {};
            }

            return std::wstring(buffer) +
                   L"\\UltraEdgeX\\services-backup.txt";
        }

        std::map<std::string, BackupEntry> ReadBackup()
        {
            std::map<std::string, BackupEntry> entries;

            const std::wstring path = BackupPath();

            if (path.empty())
            {
                return entries;
            }

            std::ifstream input{std::filesystem::path(path)};
            std::string line;

            while (std::getline(input, line))
            {
                std::istringstream parser(line);

                std::string name;
                unsigned long type = 0;
                int delayed = 0;

                if (parser >> name >> type >> delayed)
                {
                    entries[name] = BackupEntry{
                        static_cast<DWORD>(type), delayed != 0};
                }
            }

            return entries;
        }

        bool WriteBackup(const std::map<std::string, BackupEntry>& entries)
        {
            const std::wstring path = BackupPath();

            if (path.empty())
            {
                return false;
            }

            CreateDirectoryW(
                path.substr(0, path.find_last_of(L'\\')).c_str(),
                nullptr);

            std::ofstream output(
                std::filesystem::path(path),
                std::ios::trunc);

            if (!output)
            {
                return false;
            }

            for (const auto& [name, entry] : entries)
            {
                output << name << ' ' << entry.startType << ' '
                       << (entry.delayed ? 1 : 0) << '\n';
            }

            return static_cast<bool>(output);
        }

        std::string Narrow(const std::wstring& text)
        {
            std::string result;

            for (const wchar_t c : text)
            {
                result.push_back(c < 128 ? static_cast<char>(c) : '?');
            }

            return result;
        }

        std::wstring Widen(const std::string& text)
        {
            return std::wstring(text.begin(), text.end());
        }

        DWORD ToWindows(ServiceStartType type)
        {
            switch (type)
            {
            case ServiceStartType::Automatic:
            case ServiceStartType::AutomaticDelayed:
                return SERVICE_AUTO_START;
            case ServiceStartType::Disabled:
                return SERVICE_DISABLED;
            case ServiceStartType::Manual:
            case ServiceStartType::Unknown:
            default:
                return SERVICE_DEMAND_START;
            }
        }

        // Aplica um tipo de inicialização (e o "atrasado") a um serviço.
        bool ApplyStartType(
            SC_HANDLE service,
            DWORD startType,
            bool delayed,
            DWORD& error)
        {
            if (!ChangeServiceConfigW(
                    service,
                    SERVICE_NO_CHANGE,
                    startType,
                    SERVICE_NO_CHANGE,
                    nullptr, nullptr, nullptr,
                    nullptr, nullptr, nullptr, nullptr))
            {
                error = GetLastError();
                return false;
            }

            if (startType == SERVICE_AUTO_START)
            {
                SERVICE_DELAYED_AUTO_START_INFO info{};
                info.fDelayedAutostart = delayed ? TRUE : FALSE;

                ChangeServiceConfig2W(
                    service,
                    SERVICE_CONFIG_DELAYED_AUTO_START_INFO,
                    &info);
            }

            return true;
        }

        std::vector<ServiceDefinition> BuildDefinitions()
        {
            using S = SafetyLevel;
            using T = ServiceStartType;

            return {
                {L"DiagTrack", L"Telemetria do Windows",
                 L"Envia dados de uso e diagnóstico à Microsoft. Desativar não afeta o uso do PC.",
                 S::Safe, T::Automatic},
                {L"dmwappushservice", L"Mensagens push do WAP",
                 L"Apoia a telemetria e mensagens de operadora. Sem uso em PCs comuns.",
                 S::Safe, T::Manual},
                {L"WerSvc", L"Relatório de erros do Windows",
                 L"Envia relatórios de travamento à Microsoft. Sem ele, o Windows não sugere soluções.",
                 S::Safe, T::Manual},
                {L"Fax", L"Fax",
                 L"Envio e recebimento de fax pelo computador. Quase ninguém usa.",
                 S::Safe, T::Manual},
                {L"RetailDemo", L"Demonstração de varejo",
                 L"Modo de demonstração para vitrines de loja. Sem uso em PCs de casa.",
                 S::Safe, T::Manual},
                {L"MapsBroker", L"Mapas baixados",
                 L"Atualiza mapas offline do app Mapas. Só importa se você usa mapas offline.",
                 S::Safe, T::AutomaticDelayed},
                {L"wisvc", L"Windows Insider",
                 L"Necessário só para receber versões de teste do Windows. Fora disso, não faz nada.",
                 S::Safe, T::Manual},
                {L"WpcMonSvc", L"Controle dos pais",
                 L"Filtra e monitora contas de crianças. Sem uso se não há controle dos pais.",
                 S::Safe, T::Manual},

                {L"SysMain", L"SysMain (Superfetch)",
                 L"Pré-carrega apps usados com frequência. Em HDD, desativar deixa os programas mais lentos.",
                 S::Attention, T::Automatic},
                {L"DPS", L"Política de diagnóstico",
                 L"Base das soluções de problemas do Windows. Sem ele elas deixam de funcionar.",
                 S::Attention, T::Automatic},
                {L"WSearch", L"Windows Search",
                 L"Indexa arquivos para a busca. Desativado, a busca do Explorer e do Iniciar fica mais lenta.",
                 S::Attention, T::AutomaticDelayed},
                {L"Spooler", L"Spooler de impressão",
                 L"Gerencia impressoras. Desativado, você não imprime nem usa Imprimir em PDF.",
                 S::Attention, T::Automatic},
                {L"DoSvc", L"Otimização de entrega",
                 L"Compartilha downloads de atualizações. Desativado, o Windows baixa direto da Microsoft.",
                 S::Attention, T::AutomaticDelayed},
                {L"bthserv", L"Suporte a Bluetooth",
                 L"Necessário para fones, mouses e teclados Bluetooth. Desativado, eles param de funcionar.",
                 S::Attention, T::Manual},
                {L"lfsvc", L"Geolocalização",
                 L"Dá localização aos apps. Desativado, apps como Mapas e Clima não sabem onde você está.",
                 S::Attention, T::Manual},
            };
        }

        ServiceChange Failure(
            const std::wstring& name,
            DWORD error)
        {
            ServiceChange change;
            change.name = name;
            change.outcome = ServiceOutcome::Failed;
            change.error = error;
            return change;
        }

        const wchar_t* OutcomeText(ServiceOutcome outcome)
        {
            switch (outcome)
            {
            case ServiceOutcome::Changed: return L"alterado";
            case ServiceOutcome::Unchanged: return L"já estava assim";
            case ServiceOutcome::NotInstalled: return L"não existe neste Windows";
            case ServiceOutcome::InUse: return L"outros serviços dependem dele";
            case ServiceOutcome::Failed: return L"falhou";
            }

            return L"";
        }
    }

    const std::vector<ServiceDefinition>& Services::Definitions()
    {
        static const std::vector<ServiceDefinition> definitions =
            BuildDefinitions();

        return definitions;
    }

    const ServiceDefinition* Services::Find(const std::wstring& name)
    {
        for (const auto& definition : Definitions())
        {
            if (definition.name == name)
            {
                return &definition;
            }
        }

        return nullptr;
    }

    ServiceState Services::Query(const std::wstring& serviceName)
    {
        ServiceState state;

        const ScHandle manager(
            OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT));

        if (manager.handle == nullptr)
        {
            state.readable = false;
            return state;
        }

        const ScHandle service(OpenServiceW(
            manager.handle,
            serviceName.c_str(),
            SERVICE_QUERY_CONFIG | SERVICE_QUERY_STATUS));

        if (service.handle == nullptr)
        {
            if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
            {
                return state; // installed = false
            }

            state.installed = true;
            state.readable = false;
            return state;
        }

        state.installed = true;

        DWORD startType = 0;
        bool delayed = false;

        if (!ReadConfig(service.handle, startType, delayed))
        {
            state.readable = false;
            return state;
        }

        state.startType = FromWindows(startType, delayed);
        state.running = IsRunning(service.handle);

        return state;
    }

    bool Services::IsRecommendedToDisable(
        const ServiceDefinition& definition,
        const ServiceState& state)
    {
        if (
            definition.safety != SafetyLevel::Safe ||
            !state.installed ||
            !state.readable ||
            state.startType == ServiceStartType::Disabled)
        {
            return false;
        }

        const bool startsWithWindows =
            state.startType == ServiceStartType::Automatic ||
            state.startType == ServiceStartType::AutomaticDelayed;

        return state.running || startsWithWindows;
    }

    std::size_t Services::CountRecommendedToDisable()
    {
        std::size_t count = 0;

        for (const ServiceDefinition& definition : Definitions())
        {
            if (IsRecommendedToDisable(definition, Query(definition.name)))
            {
                ++count;
            }
        }

        return count;
    }

    ServiceChange Services::Disable(const ServiceDefinition& definition)
    {
        const ScHandle manager(
            OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT));

        if (manager.handle == nullptr)
        {
            return Failure(definition.name, GetLastError());
        }

        const ScHandle service(OpenServiceW(
            manager.handle,
            definition.name.c_str(),
            SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG |
                SERVICE_QUERY_STATUS | SERVICE_STOP |
                SERVICE_ENUMERATE_DEPENDENTS));

        if (service.handle == nullptr)
        {
            const DWORD error = GetLastError();

            ServiceChange change = Failure(definition.name, error);

            if (error == ERROR_SERVICE_DOES_NOT_EXIST)
            {
                change.outcome = ServiceOutcome::NotInstalled;
            }

            return change;
        }

        DWORD startType = 0;
        bool delayed = false;

        if (!ReadConfig(service.handle, startType, delayed))
        {
            return Failure(definition.name, GetLastError());
        }

        ServiceChange change;
        change.name = definition.name;

        if (startType == SERVICE_DISABLED)
        {
            change.outcome = ServiceOutcome::Unchanged;
            return change;
        }

        if (HasActiveDependents(service.handle))
        {
            change.outcome = ServiceOutcome::InUse;
            return change;
        }

        // Backup ANTES de alterar. Sem backup, não altera.
        auto backup = ReadBackup();
        const std::string key = Narrow(definition.name);

        if (backup.find(key) == backup.end())
        {
            backup[key] = BackupEntry{startType, delayed};

            if (!WriteBackup(backup))
            {
                return Failure(definition.name, ERROR_WRITE_FAULT);
            }
        }

        DWORD error = 0;

        if (!ApplyStartType(service.handle, SERVICE_DISABLED, false, error))
        {
            return Failure(definition.name, error);
        }

        StopIfRunning(service.handle);

        change.outcome = ServiceOutcome::Changed;
        return change;
    }

    ServiceChange Services::Enable(const ServiceDefinition& definition)
    {
        const ScHandle manager(
            OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT));

        if (manager.handle == nullptr)
        {
            return Failure(definition.name, GetLastError());
        }

        const ScHandle service(OpenServiceW(
            manager.handle,
            definition.name.c_str(),
            SERVICE_QUERY_CONFIG | SERVICE_CHANGE_CONFIG));

        if (service.handle == nullptr)
        {
            const DWORD error = GetLastError();

            ServiceChange change = Failure(definition.name, error);

            if (error == ERROR_SERVICE_DOES_NOT_EXIST)
            {
                change.outcome = ServiceOutcome::NotInstalled;
            }

            return change;
        }

        DWORD startType = 0;
        bool delayed = false;

        if (!ReadConfig(service.handle, startType, delayed))
        {
            return Failure(definition.name, GetLastError());
        }

        ServiceChange change;
        change.name = definition.name;

        if (startType != SERVICE_DISABLED)
        {
            change.outcome = ServiceOutcome::Unchanged;
            return change;
        }

        // Volta ao que era antes do UeX mexer; sem backup, usa o
        // padrão do Windows.
        auto backup = ReadBackup();
        const std::string key = Narrow(definition.name);
        const auto saved = backup.find(key);

        DWORD targetType = ToWindows(definition.windowsDefault);
        bool targetDelayed =
            definition.windowsDefault == ServiceStartType::AutomaticDelayed;

        if (saved != backup.end())
        {
            targetType = saved->second.startType;
            targetDelayed = saved->second.delayed;
        }

        DWORD error = 0;

        if (!ApplyStartType(service.handle, targetType, targetDelayed, error))
        {
            return Failure(definition.name, error);
        }

        if (saved != backup.end())
        {
            backup.erase(saved);
            WriteBackup(backup);
        }

        change.outcome = ServiceOutcome::Changed;
        return change;
    }

    std::vector<ServiceChange> Services::RestoreAll()
    {
        std::vector<ServiceChange> changes;

        auto backup = ReadBackup();

        for (auto it = backup.begin(); it != backup.end();)
        {
            const std::wstring name = Widen(it->first);

            // Só serviços da lista fixa.
            if (Find(name) == nullptr)
            {
                it = backup.erase(it);
                continue;
            }

            ServiceChange change;
            change.name = name;

            const ScHandle manager(
                OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT));

            ScHandle service(nullptr);

            if (manager.handle != nullptr)
            {
                service.handle = OpenServiceW(
                    manager.handle,
                    name.c_str(),
                    SERVICE_CHANGE_CONFIG);
            }

            if (service.handle == nullptr)
            {
                const DWORD error = GetLastError();

                change = Failure(name, error);

                if (error == ERROR_SERVICE_DOES_NOT_EXIST)
                {
                    change.outcome = ServiceOutcome::NotInstalled;
                    it = backup.erase(it);
                }
                else
                {
                    ++it;
                }

                changes.push_back(change);
                continue;
            }

            DWORD error = 0;

            if (ApplyStartType(
                    service.handle,
                    it->second.startType,
                    it->second.delayed,
                    error))
            {
                change.outcome = ServiceOutcome::Changed;
                it = backup.erase(it);
            }
            else
            {
                change = Failure(name, error);
                ++it;
            }

            changes.push_back(change);
        }

        WriteBackup(backup);

        return changes;
    }

    std::vector<ServiceChange> Services::Execute(
        const ServiceRequest& request)
    {
        std::vector<ServiceChange> changes;

        const auto record = [&](const ServiceChange& change)
        {
            core::Logger::Info(
                L"Serviço " + change.name + L": " +
                OutcomeText(change.outcome) +
                (change.outcome == ServiceOutcome::Failed
                     ? L" (erro " + std::to_wstring(change.error) + L")"
                     : L""));

            changes.push_back(change);
        };

        if (request.restore)
        {
            for (const ServiceChange& change : RestoreAll())
            {
                record(change);
            }
        }

        for (const std::wstring& name : request.disable)
        {
            if (const ServiceDefinition* definition = Find(name))
            {
                record(Disable(*definition));
            }
        }

        for (const std::wstring& name : request.enable)
        {
            if (const ServiceDefinition* definition = Find(name))
            {
                record(Enable(*definition));
            }
        }

        return changes;
    }

    std::vector<std::wstring> Services::BackedUpNames()
    {
        std::vector<std::wstring> names;

        for (const auto& [name, entry] : ReadBackup())
        {
            static_cast<void>(entry);

            const std::wstring wide = Widen(name);

            if (Find(wide) != nullptr)
            {
                names.push_back(wide);
            }
        }

        return names;
    }
}
