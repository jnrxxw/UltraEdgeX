#include "roui/window/Window.h"

#include <cstdlib>
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
        const std::wstring& title,
        int width,
        int height
    )
    {
        if (
            instance == nullptr ||
            width <= 0 ||
            height <= 0
        )
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
            height_
        };

        if (!AdjustWindowRect(
                &windowRectangle,
                windowStyle,
                FALSE
            ))
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
            this
        );

        if (handle_ == nullptr)
        {
            return false;
        }

        if (!renderer_.Initialize(handle_))
        {
            DestroyWindow(handle_);
            handle_ = nullptr;

            return false;
        }

        rootPanel_.SetBounds({
            0.0F,
            0.0F,
            static_cast<float>(width_),
            static_cast<float>(height_)
        });

        if (!dashboardPage_.Initialize(rootPanel_))
        {
            renderer_.Shutdown();

            DestroyWindow(handle_);
            handle_ = nullptr;

            return false;
        }

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
            showCommand
        );

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
                0
            );

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
            message.wParam
        );
    }

    HWND Window::GetHandle() const noexcept
    {
        return handle_;
    }

    bool Window::IsCreated() const noexcept
    {
        return handle_ != nullptr;
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

        windowClass.hCursor =
            LoadCursorW(
                nullptr,
                IDC_ARROW
            );

        windowClass.hbrBackground =
            nullptr;

        windowClass.lpszClassName =
            className_.c_str();

        const ATOM registrationResult =
            RegisterClassExW(
                &windowClass
            );

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
        LPARAM lParam
    )
    {
        Window* window = nullptr;

        if (message == WM_NCCREATE)
        {
            const auto* createStructure =
                reinterpret_cast<
                    CREATESTRUCTW*
                >(
                    lParam
                );

            window =
                static_cast<Window*>(
                    createStructure->
                        lpCreateParams
                );

            SetWindowLongPtrW(
                windowHandle,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(
                    window
                )
            );

            window->handle_ =
                windowHandle;
        }
        else
        {
            window =
                reinterpret_cast<Window*>(
                    GetWindowLongPtrW(
                        windowHandle,
                        GWLP_USERDATA
                    )
                );
        }

        if (window != nullptr)
        {
            return window->HandleMessage(
                message,
                wParam,
                lParam
            );
        }

        return DefWindowProcW(
            windowHandle,
            message,
            wParam,
            lParam
        );
    }

    LRESULT Window::HandleMessage(
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    )
    {
        switch (message)
        {
        case WM_CLOSE:
            DestroyWindow(handle_);
            return 0;

        case WM_DESTROY:
            renderer_.Shutdown();

            handle_ = nullptr;

            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
        {
            const UINT width =
                static_cast<UINT>(
                    LOWORD(lParam)
                );

            const UINT height =
                static_cast<UINT>(
                    HIWORD(lParam)
                );

            renderer_.Resize(
                width,
                height
            );

            rootPanel_.SetSize(
                static_cast<float>(width),
                static_cast<float>(height)
            );

            dashboardPage_.Resize(
                static_cast<float>(width),
                static_cast<float>(height)
            );

            InvalidateRect(
                handle_,
                nullptr,
                FALSE
            );

            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_MOUSEMOVE:
        {
            const Point mousePosition{
                static_cast<float>(
                    GET_X_LPARAM(lParam)
                ),
                static_cast<float>(
                    GET_Y_LPARAM(lParam)
                )
            };

            rootPanel_.OnMouseMove(
                mousePosition
            );

            TRACKMOUSEEVENT trackingEvent{};

            trackingEvent.cbSize =
                sizeof(TRACKMOUSEEVENT);

            trackingEvent.dwFlags =
                TME_LEAVE;

            trackingEvent.hwndTrack =
                handle_;

            TrackMouseEvent(
                &trackingEvent
            );

            InvalidateRect(
                handle_,
                nullptr,
                FALSE
            );

            return 0;
        }

        case WM_MOUSELEAVE:
            rootPanel_.OnMouseLeave();

            InvalidateRect(
                handle_,
                nullptr,
                FALSE
            );

            return 0;

        case WM_LBUTTONDOWN:
        {
            const Point mousePosition{
                static_cast<float>(
                    GET_X_LPARAM(lParam)
                ),
                static_cast<float>(
                    GET_Y_LPARAM(lParam)
                )
            };

            SetCapture(handle_);

            rootPanel_.OnMouseDown(
                mousePosition
            );

            InvalidateRect(
                handle_,
                nullptr,
                FALSE
            );

            return 0;
        }

        case WM_LBUTTONUP:
        {
            const Point mousePosition{
                static_cast<float>(
                    GET_X_LPARAM(lParam)
                ),
                static_cast<float>(
                    GET_Y_LPARAM(lParam)
                )
            };

            ReleaseCapture();

            rootPanel_.OnMouseUp(
                mousePosition
            );

            InvalidateRect(
                handle_,
                nullptr,
                FALSE
            );

            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT paintStructure{};

            BeginPaint(
                handle_,
                &paintStructure
            );

            if (renderer_.BeginFrame())
            {
                rootPanel_.Render(
                    renderer_
                );

                renderer_.EndFrame();
            }

            EndPaint(
                handle_,
                &paintStructure
            );

            return 0;
        }

        default:
            return DefWindowProcW(
                handle_,
                message,
                wParam,
                lParam
            );
        }
    }
}