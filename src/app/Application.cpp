#include "app/Application.h"

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
        const bool created =
            mainWindow_.Create(
                instance,
                L"UltraEdgeX",
                1000,
                1050);

        if (!created)
        {
            return false;
        }

        mainWindow_.Show(showCommand);

        return true;
    }

    void Application::Shutdown()
    {
    }
}