#include "app/Application.h"

#include "core/Logger.h"
#include "roui/core/Theme.h"

#include <cstdlib>

namespace real_optimizer::app
{
    int Application::Run(
        HINSTANCE instance,
        int showCommand)
    {
        if (!Initialize(instance, showCommand))
        {
            MessageBoxW(
                nullptr,
                L"Não foi possível inicializar o UltraEdgeX.",
                L"UltraEdgeX",
                MB_OK | MB_ICONERROR);

            return EXIT_FAILURE;
        }

        const int exitCode =
            mainWindow_.RunMessageLoop();

        Shutdown();

        return exitCode;
    }

    bool Application::Initialize(
        HINSTANCE instance,
        int showCommand)
    {
        const HRESULT comResult = CoInitializeEx(
            nullptr,
            COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        comInitialized_ = SUCCEEDED(comResult);

        real_optimizer::core::Logger::Info(L"UeX iniciado.");

        // Carrega o tema (escuro/claro) salvo pelo usuário.
        real_optimizer::roui::Theme::LoadPreference();

        const bool created =
            mainWindow_.Create(
                instance,
                L"UltraEdgeX",
                1280,
                950);

        if (!created)
        {
            return false;
        }

        mainWindow_.Show(showCommand);

        return true;
    }

    void Application::Shutdown()
    {
        real_optimizer::core::Logger::Info(L"UeX encerrado.");

        if (comInitialized_)
        {
            CoUninitialize();
            comInitialized_ = false;
        }
    }
}