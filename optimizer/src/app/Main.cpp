#include "app/Application.h"

#include <windows.h>

int WINAPI wWinMain(
    HINSTANCE instance,
    HINSTANCE previousInstance,
    PWSTR commandLine,
    int showCommand
)
{
    static_cast<void>(previousInstance);
    static_cast<void>(commandLine);

    real_optimizer::app::Application application;

    return application.Run(
        instance,
        showCommand
    );
}