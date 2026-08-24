#include "system/StartupManager.h"

#include <windows.h>

#include <iterator>
#include <string>
#include <utility>
#include <vector>

namespace
{
    using real_optimizer::system::StartupApp;
    using real_optimizer::system::StartupSource;

    //
    // Verifica o estado de um aplicativo
    // no StartupApproved do Windows.
    //
    bool IsStartupAppEnabled(
        HKEY rootKey,
        const std::wstring &appName)
    {
        constexpr const wchar_t *StartupApprovedKey =
            L"Software\\Microsoft\\Windows\\CurrentVersion\\"
            L"Explorer\\StartupApproved\\Run";

        HKEY key = nullptr;

        const LONG openResult =
            RegOpenKeyExW(
                rootKey,
                StartupApprovedKey,
                0,
                KEY_READ,
                &key);

        //
        // Se não houver informação no StartupApproved,
        // consideramos a entrada habilitada.
        //
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

        //
        // Sem registro no StartupApproved:
        // a entrada existente em Run é considerada ativa.
        //
        if (queryResult != ERROR_SUCCESS)
        {
            return true;
        }

        if (
            valueType != REG_BINARY ||
            dataSize == 0)
        {
            return true;
        }

        //
        // StartupApproved:
        //
        // 02 = habilitado
        // 03 = desabilitado
        //
        // Existem outros estados usados pelo Windows,
        // portanto tratamos explicitamente o estado
        // desabilitado.
        //
        if (data[0] == 0x03)
        {
            return false;
        }

        return true;
    }

    void ReadStartupRegistryKey(
        HKEY rootKey,
        const wchar_t *subKey,
        StartupSource source,
        std::vector<StartupApp> &apps)
    {
        HKEY key = nullptr;

        const LONG openResult =
            RegOpenKeyExW(
                rootKey,
                subKey,
                0,
                KEY_READ,
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
                sizeof(valueData);

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

            if (result == ERROR_SUCCESS)
            {
                if (
                    valueType == REG_SZ ||
                    valueType == REG_EXPAND_SZ)
                {
                    StartupApp app{};

                    app.name =
                        valueName;

                    app.command =
                        valueData;

                    app.source =
                        source;

                    //
                    // Agora consultamos se a entrada
                    // está realmente habilitada.
                    //
                    app.enabled =
                        IsStartupAppEnabled(
                            rootKey,
                            app.name);

                    apps.push_back(
                        std::move(app));
                }
            }

            ++valueIndex;
        }

        RegCloseKey(key);
    }
}

namespace real_optimizer::system
{
    std::vector<StartupApp>
    StartupManager::GetStartupApps()
    {
        std::vector<StartupApp> apps;

        constexpr const wchar_t *RunKey =
            L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

        //
        // Aplicativos do usuário atual.
        //
        ReadStartupRegistryKey(
            HKEY_CURRENT_USER,
            RunKey,
            StartupSource::CurrentUser,
            apps);

        //
        // Aplicativos configurados
        // para toda a máquina.
        //
        ReadStartupRegistryKey(
            HKEY_LOCAL_MACHINE,
            RunKey,
            StartupSource::LocalMachine,
            apps);

        return apps;
    }
}