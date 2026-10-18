#pragma once

#include <string>
#include <windows.h>

#include "features/shell/MainShell.h"
#include "roui/controls/Panel.h"
#include "roui/core/Theme.h"
#include "roui/rendering/Renderer.h"
#include "roui/window/UiDispatcher.h"

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
        // Pinta a barra de título conforme o tema atual.
        void ApplyTitleBarTheme();

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

        bool titleBarApplied_ = false;
        ThemeMode appliedTitleBarMode_ = ThemeMode::Dark;

        int width_ = 0;
        int height_ = 0;

        rendering::Renderer renderer_;
        Panel rootPanel_;

        UiDispatcher dispatcher_;
        features::shell::MainShell shell_;
    };
}