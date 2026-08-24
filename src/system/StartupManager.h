#pragma once

#include <string>
#include <vector>

namespace real_optimizer::system
{
    enum class StartupSource
    {
        CurrentUser,
        LocalMachine
    };

    struct StartupApp
    {
        std::wstring name;
        std::wstring command;

        StartupSource source =
            StartupSource::CurrentUser;

        //
        // Indica se o aplicativo está
        // habilitado para iniciar com o Windows.
        //
        bool enabled = true;
    };

    class StartupManager final
    {
    public:
        StartupManager() = delete;

        [[nodiscard]]
        static std::vector<StartupApp>
        GetStartupApps();
    };
}