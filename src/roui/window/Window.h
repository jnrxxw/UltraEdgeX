#pragma once

#include <string>
#include <windows.h>

#include "features/dashboard/DashboardPage.h"
#include "roui/controls/Panel.h"
#include "roui/rendering/Renderer.h"

namespace real_optimizer::roui
{
    class Window final
    {
    public:
        Window() = default;
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        Window(Window &&) = delete;
        Window &operator=(Window &&) = delete;

        bool Create(
            HINSTANCE instance,
            const std::wstring &title,
            int width,
            int height);

        void Show(int showCommand);
        int RunMessageLoop();

        [[nodiscard]] HWND GetHandle() const noexcept;
        [[nodiscard]] bool IsCreated() const noexcept;

    private:
        static LRESULT CALLBACK WindowProcedure(
            HWND windowHandle,
            UINT message,
            WPARAM wParam,
            LPARAM lParam);

        LRESULT HandleMessage(
            UINT message,
            WPARAM wParam,
            LPARAM lParam);

        bool RegisterWindowClass();

        HINSTANCE instance_ = nullptr;
        HWND handle_ = nullptr;

        std::wstring title_;
        std::wstring className_ =
            L"RealOptimizerWindowClass";

        int width_ = 0;
        int height_ = 0;

        rendering::Renderer renderer_;
        Panel rootPanel_;

        features::dashboard::DashboardPage dashboardPage_;
    };
}