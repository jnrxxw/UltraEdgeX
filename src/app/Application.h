#pragma once

#include <windows.h>
#include "roui/window/Window.h"

namespace real_optimizer::app
{
    class Application final
    {
    public:
        Application() = default;
        ~Application() = default;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

        [[nodiscard]] int Run(
            HINSTANCE instance,
            int showCommand
        );

    private:
        bool Initialize(
            HINSTANCE instance,
            int showCommand
        );

        void Shutdown();

        bool comInitialized_ = false;
        real_optimizer::roui::Window mainWindow_;
    };
}