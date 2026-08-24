#pragma once

#include "roui/core/Geometry.h"
#include "roui/core/VisualTypes.h"

#include <d2d1.h>
#include <dwrite.h>
#include <windows.h>
#include <wrl/client.h>

#include <string_view>

namespace real_optimizer::roui::rendering
{
    class Renderer final
    {
    public:
        Renderer() = default;
        ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        [[nodiscard]] bool Initialize(HWND windowHandle);

        [[nodiscard]] bool BeginFrame();
        void EndFrame();

        void DrawText(
            std::wstring_view text,
            const Rect& bounds,
            float fontSize,
            FontWeight fontWeight,
            const Color& color,
            HorizontalAlignment horizontalAlignment,
            VerticalAlignment verticalAlignment
        );

        void Resize(UINT width, UINT height);
        void Shutdown();

        [[nodiscard]] bool IsInitialized() const noexcept;
        
        void FillRectangle(
        const Rect& bounds,
        const Color& color
        );

        void DrawRectangle(
            const Rect& bounds,
            float strokeWidth,
            const Color& color
        );

        void DrawRoundedRectangle(
            const Rect& bounds,
            float cornerRadius,
            float strokeWidth,
            const Color& color
        );


        void FillRoundedRectangle(
        const Rect& bounds,
        float cornerRadius,
        const Color& color
);

    private:
        [[nodiscard]] bool CreateIndependentResources();
        [[nodiscard]] bool CreateWindowResources();

        void DiscardWindowResources();

        HWND windowHandle_ = nullptr;
        bool initialized_ = false;
        bool frameActive_ = false;

        Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory_;
        Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> renderTarget_;
        Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory_;
    };
}