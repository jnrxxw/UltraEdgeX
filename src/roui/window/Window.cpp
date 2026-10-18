#include "roui/window/Window.h"

#include "resources/Resource.h"

#include <cstdlib>
#include <dwmapi.h>
#include <windowsx.h>

namespace real_optimizer::roui
{
    Window::~Window()
    {
        renderer_.Shutdown();

        if (handle_ != nullptr)
        {
            DestroyWindow(handle_);
            handle_ = nullptr;
        }
    }

    bool Window::Create(
        HINSTANCE instance,
        const std::wstring &title,
        int width,
        int height)
    {
        if (
            instance == nullptr ||
            width <= 0 ||
            height <= 0)
        {
            return false;
        }

        instance_ = instance;
        title_ = title;
        width_ = width;
        height_ = height;

        if (!RegisterWindowClass())
        {
            return false;
        }

        const DWORD windowStyle =
            WS_OVERLAPPEDWINDOW;

        RECT windowRectangle{
            0,
            0,
            width_,
            height_};

        if (!AdjustWindowRect(
                &windowRectangle,
                windowStyle,
                FALSE))
        {
            return false;
        }

        const int finalWidth =
            windowRectangle.right -
            windowRectangle.left;

        const int finalHeight =
            windowRectangle.bottom -
            windowRectangle.top;

        handle_ = CreateWindowExW(
            0,
            className_.c_str(),
            title_.c_str(),
            windowStyle,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            finalWidth,
            finalHeight,
            nullptr,
            nullptr,
            instance_,
            this);

        if (handle_ == nullptr)
        {
            return false;
        }

        HICON appIcon = LoadIconW(
            instance_,
            MAKEINTRESOURCEW(IDI_APP_ICON));

        if (appIcon != nullptr)
        {
            SendMessageW(
                handle_,
                WM_SETICON,
                ICON_BIG,
                reinterpret_cast<LPARAM>(appIcon));

            SendMessageW(
                handle_,
                WM_SETICON,
                ICON_SMALL,
                reinterpret_cast<LPARAM>(appIcon));
        }

        if (!renderer_.Initialize(handle_))
        {
            DestroyWindow(handle_);
            handle_ = nullptr;

            return false;
        }

        rootPanel_.SetBounds({0.0F,
                              0.0F,
                              static_cast<float>(width_),
                              static_cast<float>(height_)});

        dispatcher_.Attach(handle_);

        if (!shell_.Initialize(rootPanel_, dispatcher_))
        {
            renderer_.Shutdown();

            DestroyWindow(handle_);
            handle_ = nullptr;

            return false;
        }

        ApplyTitleBarTheme();

        // Posiciona os elementos para o tamanho inicial.
        shell_.Resize(
            static_cast<float>(width_),
            static_cast<float>(height_));

        SetTimer(
            handle_,
            1,
            1000,
            nullptr);

        return true;
    }

    void Window::Show(int showCommand)
    {
        if (handle_ == nullptr)
        {
            return;
        }

        ShowWindow(
            handle_,
            showCommand);

        UpdateWindow(handle_);
    }

    int Window::RunMessageLoop()
    {
        MSG message{};

        while (true)
        {
            const BOOL result = GetMessageW(
                &message,
                nullptr,
                0,
                0);

            if (result == -1)
            {
                return EXIT_FAILURE;
            }

            if (result == 0)
            {
                break;
            }

            TranslateMessage(&message);
            DispatchMessageW(&message);
        }

        return static_cast<int>(
            message.wParam);
    }

    HWND Window::GetHandle() const noexcept
    {
        return handle_;
    }

    bool Window::IsCreated() const noexcept
    {
        return handle_ != nullptr;
    }

    void Window::ApplyTitleBarTheme()
    {
        if (handle_ == nullptr)
        {
            return;
        }

        const ThemeMode mode = Theme::GetMode();

        if (
            titleBarApplied_ &&
            mode == appliedTitleBarMode_)
        {
            return;
        }

        titleBarApplied_ = true;
        appliedTitleBarMode_ = mode;

        const Palette &palette = Theme::Current();

        const auto toColorRef =
            [](const Color &color) -> COLORREF
        {
            return RGB(
                static_cast<BYTE>(color.red * 255.0F + 0.5F),
                static_cast<BYTE>(color.green * 255.0F + 0.5F),
                static_cast<BYTE>(color.blue * 255.0F + 0.5F));
        };

        // Valores do DWMWINDOWATTRIBUTE (usados direto para
        // não depender da versão do Windows SDK).
        constexpr DWORD ImmersiveDarkMode = 20;
        constexpr DWORD BorderColor = 34;
        constexpr DWORD CaptionColor = 35;
        constexpr DWORD TextColor = 36;

        const BOOL useDark =
            mode == ThemeMode::Dark ? TRUE : FALSE;

        const COLORREF caption = toColorRef(palette.sidebar);
        const COLORREF text = toColorRef(palette.textPrimary);
        const COLORREF border = toColorRef(palette.border);

        // Windows 10 (2004+) e 11: botões e texto claro/escuro.
        DwmSetWindowAttribute(
            handle_, ImmersiveDarkMode, &useDark, sizeof(useDark));

        // Windows 11: cor própria da barra de título.
        DwmSetWindowAttribute(
            handle_, CaptionColor, &caption, sizeof(caption));

        DwmSetWindowAttribute(
            handle_, TextColor, &text, sizeof(text));

        DwmSetWindowAttribute(
            handle_, BorderColor, &border, sizeof(border));
    }

    bool Window::RegisterWindowClass()
    {
        WNDCLASSEXW windowClass{};

        windowClass.cbSize =
            sizeof(WNDCLASSEXW);

        windowClass.style =
            CS_HREDRAW |
            CS_VREDRAW;

        windowClass.lpfnWndProc =
            WindowProcedure;

        windowClass.hInstance =
            instance_;

        windowClass.hIcon =
            LoadIconW(
                instance_,
                MAKEINTRESOURCEW(IDI_APP_ICON));

        windowClass.hIconSm =
            LoadIconW(
                instance_,
                MAKEINTRESOURCEW(IDI_APP_ICON));

        windowClass.hCursor =
            LoadCursorW(
                nullptr,
                IDC_ARROW);

        windowClass.hbrBackground =
            nullptr;

        windowClass.lpszClassName =
            className_.c_str();

        const ATOM registrationResult =
            RegisterClassExW(
                &windowClass);

        if (registrationResult != 0)
        {
            return true;
        }

        return GetLastError() ==
               ERROR_CLASS_ALREADY_EXISTS;
    }

    LRESULT CALLBACK Window::WindowProcedure(
        HWND windowHandle,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        Window *window = nullptr;

        if (message == WM_NCCREATE)
        {
            const auto *createStructure =
                reinterpret_cast<
                    CREATESTRUCTW *>(
                    lParam);

            window =
                static_cast<Window *>(
                    createStructure->lpCreateParams);

            SetWindowLongPtrW(
                windowHandle,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(
                    window));

            window->handle_ =
                windowHandle;
        }
        else
        {
            window =
                reinterpret_cast<Window *>(
                    GetWindowLongPtrW(
                        windowHandle,
                        GWLP_USERDATA));
        }

        if (window != nullptr)
        {
            return window->HandleMessage(
                message,
                wParam,
                lParam);
        }

        return DefWindowProcW(
            windowHandle,
            message,
            wParam,
            lParam);
    }

    LRESULT Window::HandleMessage(
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
    {
        switch (message)
        {
        case WM_CLOSE:
            DestroyWindow(handle_);
            return 0;

        case WM_DESTROY:

            KillTimer(
                handle_,
                1);

            // Para de receber tarefas e espera as que estão rodando.
            dispatcher_.Detach();
            shell_.Shutdown();

            renderer_.Shutdown();

            handle_ = nullptr;

            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
        {
            const UINT width =
                static_cast<UINT>(
                    LOWORD(lParam));

            const UINT height =
                static_cast<UINT>(
                    HIWORD(lParam));

            if (width == 0 || height == 0)
            {
                return 0;
            }

            renderer_.Resize(
                width,
                height);

            rootPanel_.SetSize(
                static_cast<float>(width),
                static_cast<float>(height));

            shell_.Resize(
                static_cast<float>(width),
                static_cast<float>(height));

            InvalidateRect(
                handle_,
                nullptr,
                FALSE);

            return 0;
        }

        case WM_TIMER:
        {
            if (wParam == 1)
            {
                shell_.RefreshRealtimeStats();

                InvalidateRect(
                    handle_,
                    nullptr,
                    FALSE);
            }

            return 0;
        }

        case UiDispatcher::Message:
            // Código agendado por threads de trabalho.
            UiDispatcher::Execute(lParam);

            InvalidateRect(
                handle_,
                nullptr,
                FALSE);

            return 0;

        case WM_KEYDOWN:
            if (shell_.OnKeyDown(static_cast<unsigned int>(wParam)))
            {
                InvalidateRect(
                    handle_,
                    nullptr,
                    FALSE);

                return 0;
            }

            return DefWindowProcW(
                handle_,
                message,
                wParam,
                lParam);

        case WM_GETMINMAXINFO:
        {
            // Tamanho mínimo da área de conteúdo,
            // convertido para o tamanho total da janela.
            RECT minimum{0, 0, 1180, 700};

            AdjustWindowRect(
                &minimum,
                WS_OVERLAPPEDWINDOW,
                FALSE);

            auto *info =
                reinterpret_cast<MINMAXINFO *>(lParam);

            info->ptMinTrackSize.x =
                minimum.right - minimum.left;

            info->ptMinTrackSize.y =
                minimum.bottom - minimum.top;

            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_MOUSEMOVE:
        {
            const Point mousePosition{
                static_cast<float>(
                    GET_X_LPARAM(lParam)),
                static_cast<float>(
                    GET_Y_LPARAM(lParam))};

            rootPanel_.OnMouseMove(
                mousePosition);

            TRACKMOUSEEVENT trackingEvent{};

            trackingEvent.cbSize =
                sizeof(TRACKMOUSEEVENT);

            trackingEvent.dwFlags =
                TME_LEAVE;

            trackingEvent.hwndTrack =
                handle_;

            TrackMouseEvent(
                &trackingEvent);

            InvalidateRect(
                handle_,
                nullptr,
                FALSE);

            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            // A posição vem em coordenadas da tela.
            POINT cursor{
                GET_X_LPARAM(lParam),
                GET_Y_LPARAM(lParam)};

            ScreenToClient(handle_, &cursor);

            rootPanel_.OnMouseWheel(
                Point{
                    static_cast<float>(cursor.x),
                    static_cast<float>(cursor.y)},
                static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) /
                    static_cast<float>(WHEEL_DELTA));

            InvalidateRect(
                handle_,
                nullptr,
                FALSE);

            return 0;
        }

        case WM_MOUSELEAVE:
            rootPanel_.OnMouseLeave();

            InvalidateRect(
                handle_,
                nullptr,
                FALSE);

            return 0;

        case WM_LBUTTONDOWN:
        {
            const Point mousePosition{
                static_cast<float>(
                    GET_X_LPARAM(lParam)),
                static_cast<float>(
                    GET_Y_LPARAM(lParam))};

            SetCapture(handle_);

            rootPanel_.OnMouseDown(
                mousePosition);

            InvalidateRect(
                handle_,
                nullptr,
                FALSE);

            return 0;
        }

        case WM_LBUTTONUP:
        {
            const Point mousePosition{
                static_cast<float>(
                    GET_X_LPARAM(lParam)),
                static_cast<float>(
                    GET_Y_LPARAM(lParam))};

            ReleaseCapture();

            rootPanel_.OnMouseUp(
                mousePosition);

            // O clique pode ter trocado o tema.
            ApplyTitleBarTheme();

            InvalidateRect(
                handle_,
                nullptr,
                FALSE);

            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT paintStructure{};

            BeginPaint(
                handle_,
                &paintStructure);

            if (renderer_.BeginFrame())
            {
                rootPanel_.Render(
                    renderer_);

                renderer_.EndFrame();
            }

            EndPaint(
                handle_,
                &paintStructure);

            return 0;
        }

        default:
            return DefWindowProcW(
                handle_,
                message,
                wParam,
                lParam);
        }
    }
}