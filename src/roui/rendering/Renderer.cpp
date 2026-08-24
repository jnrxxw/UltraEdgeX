#include "roui/rendering/Renderer.h"

#include <d2d1helper.h>

namespace real_optimizer::roui::rendering
{
    namespace
    {
        DWRITE_FONT_WEIGHT ConvertFontWeight(
            FontWeight fontWeight) noexcept
        {
            switch (fontWeight)
            {
            case FontWeight::SemiBold:
                return DWRITE_FONT_WEIGHT_SEMI_BOLD;

            case FontWeight::Bold:
                return DWRITE_FONT_WEIGHT_BOLD;

            case FontWeight::Normal:
            default:
                return DWRITE_FONT_WEIGHT_NORMAL;
            }
        }

        DWRITE_TEXT_ALIGNMENT ConvertHorizontalAlignment(
            HorizontalAlignment alignment) noexcept
        {
            switch (alignment)
            {
            case HorizontalAlignment::Center:
                return DWRITE_TEXT_ALIGNMENT_CENTER;

            case HorizontalAlignment::Right:
                return DWRITE_TEXT_ALIGNMENT_TRAILING;

            case HorizontalAlignment::Left:
            default:
                return DWRITE_TEXT_ALIGNMENT_LEADING;
            }
        }

        DWRITE_PARAGRAPH_ALIGNMENT ConvertVerticalAlignment(
            VerticalAlignment alignment) noexcept
        {
            switch (alignment)
            {
            case VerticalAlignment::Center:
                return DWRITE_PARAGRAPH_ALIGNMENT_CENTER;

            case VerticalAlignment::Bottom:
                return DWRITE_PARAGRAPH_ALIGNMENT_FAR;

            case VerticalAlignment::Top:
            default:
                return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
            }
        }
    }

    bool Renderer::Initialize(HWND windowHandle)
    {
        if (windowHandle == nullptr)
        {
            return false;
        }

        if (initialized_)
        {
            return true;
        }

        windowHandle_ = windowHandle;

        if (!CreateIndependentResources())
        {
            Shutdown();
            return false;
        }

        initialized_ = true;
        return true;
    }

    bool Renderer::CreateIndependentResources()
    {
        HRESULT result = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            d2dFactory_.GetAddressOf());

        if (FAILED(result))
        {
            return false;
        }

        result = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown **>(
                dwriteFactory_.GetAddressOf()));

        return SUCCEEDED(result);
    }

    bool Renderer::CreateWindowResources()
    {
        if (renderTarget_ != nullptr)
        {
            return true;
        }

        if (
            d2dFactory_ == nullptr ||
            windowHandle_ == nullptr)
        {
            return false;
        }

        RECT clientRectangle{};

        if (!GetClientRect(
                windowHandle_,
                &clientRectangle))
        {
            return false;
        }

        const UINT width = static_cast<UINT>(
            clientRectangle.right -
            clientRectangle.left);

        const UINT height = static_cast<UINT>(
            clientRectangle.bottom -
            clientRectangle.top);

        const HRESULT result =
            d2dFactory_->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                    windowHandle_,
                    D2D1::SizeU(width, height)),
                renderTarget_.GetAddressOf());

        return SUCCEEDED(result);
    }

    bool Renderer::BeginFrame()
    {
        if (!initialized_ || frameActive_)
        {
            return false;
        }

        if (!CreateWindowResources())
        {
            return false;
        }

        renderTarget_->BeginDraw();

        renderTarget_->Clear(
            D2D1::ColorF(
                13.0F / 255.0F,
                13.0F / 255.0F,
                13.0F / 255.0F,
                1.0F));

        frameActive_ = true;
        return true;
    }

    void Renderer::EndFrame()
    {
        if (
            !frameActive_ ||
            renderTarget_ == nullptr)
        {
            return;
        }

        const HRESULT result =
            renderTarget_->EndDraw();

        frameActive_ = false;

        if (result == D2DERR_RECREATE_TARGET)
        {
            DiscardWindowResources();
        }
    }

    void Renderer::DrawText(
        std::wstring_view text,
        const Rect &bounds,
        float fontSize,
        FontWeight fontWeight,
        const Color &color,
        HorizontalAlignment horizontalAlignment,
        VerticalAlignment verticalAlignment)
    {
        if (
            !frameActive_ ||
            renderTarget_ == nullptr ||
            dwriteFactory_ == nullptr ||
            text.empty() ||
            bounds.IsEmpty() ||
            fontSize <= 0.0F)
        {
            return;
        }

        Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;

        HRESULT result =
            dwriteFactory_->CreateTextFormat(
                L"Segoe UI",
                nullptr,
                ConvertFontWeight(fontWeight),
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                fontSize,
                L"pt-BR",
                textFormat.GetAddressOf());

        if (FAILED(result))
        {
            return;
        }

        textFormat->SetTextAlignment(
            ConvertHorizontalAlignment(
                horizontalAlignment));

        textFormat->SetParagraphAlignment(
            ConvertVerticalAlignment(
                verticalAlignment));

        Microsoft::WRL::ComPtr<
            ID2D1SolidColorBrush>
            brush;

        result =
            renderTarget_->CreateSolidColorBrush(
                D2D1::ColorF(
                    color.red,
                    color.green,
                    color.blue,
                    color.alpha),
                brush.GetAddressOf());

        if (FAILED(result))
        {
            return;
        }

        const D2D1_RECT_F drawingBounds =
            D2D1::RectF(
                bounds.Left(),
                bounds.Top(),
                bounds.Right(),
                bounds.Bottom());

        renderTarget_->DrawTextW(
            text.data(),
            static_cast<UINT32>(text.size()),
            textFormat.Get(),
            drawingBounds,
            brush.Get());
    }

    void Renderer::FillRectangle(
        const Rect &bounds,
        const Color &color)
    {
        if (
            !frameActive_ ||
            renderTarget_ == nullptr ||
            bounds.IsEmpty())
        {
            return;
        }

        Microsoft::WRL::ComPtr<
            ID2D1SolidColorBrush>
            brush;

        const HRESULT result =
            renderTarget_->CreateSolidColorBrush(
                D2D1::ColorF(
                    color.red,
                    color.green,
                    color.blue,
                    color.alpha),
                brush.GetAddressOf());

        if (FAILED(result))
        {
            return;
        }

        const D2D1_RECT_F rectangle =
            D2D1::RectF(
                bounds.Left(),
                bounds.Top(),
                bounds.Right(),
                bounds.Bottom());

        renderTarget_->FillRectangle(
            rectangle,
            brush.Get());
    }

    void Renderer::DrawRectangle(
        const Rect &bounds,
        float strokeWidth,
        const Color &color)
    {
        if (
            !frameActive_ ||
            renderTarget_ == nullptr ||
            bounds.IsEmpty() ||
            strokeWidth <= 0.0F)
        {
            return;
        }

        Microsoft::WRL::ComPtr<
            ID2D1SolidColorBrush>
            brush;

        const HRESULT result =
            renderTarget_->CreateSolidColorBrush(
                D2D1::ColorF(
                    color.red,
                    color.green,
                    color.blue,
                    color.alpha),
                brush.GetAddressOf());

        if (FAILED(result))
        {
            return;
        }

        const D2D1_RECT_F rectangle =
            D2D1::RectF(
                bounds.Left(),
                bounds.Top(),
                bounds.Right(),
                bounds.Bottom());

        renderTarget_->DrawRectangle(
            rectangle,
            brush.Get(),
            strokeWidth);
    }

    void Renderer::FillRoundedRectangle(
        const Rect &bounds,
        float cornerRadius,
        const Color &color)
    {
        if (
            !frameActive_ ||
            renderTarget_ == nullptr ||
            bounds.IsEmpty() ||
            cornerRadius < 0.0F)
        {
            return;
        }

        Microsoft::WRL::ComPtr<
            ID2D1SolidColorBrush>
            brush;

        const HRESULT result =
            renderTarget_->CreateSolidColorBrush(
                D2D1::ColorF(
                    color.red,
                    color.green,
                    color.blue,
                    color.alpha),
                brush.GetAddressOf());

        if (FAILED(result))
        {
            return;
        }

        const D2D1_ROUNDED_RECT rectangle{
            D2D1::RectF(
                bounds.Left(),
                bounds.Top(),
                bounds.Right(),
                bounds.Bottom()),
            cornerRadius,
            cornerRadius};

        renderTarget_->FillRoundedRectangle(
            rectangle,
            brush.Get());
    }

    void Renderer::DrawRoundedRectangle(
        const Rect &bounds,
        float cornerRadius,
        float strokeWidth,
        const Color &color)
    {
        if (
            !frameActive_ ||
            renderTarget_ == nullptr ||
            bounds.IsEmpty() ||
            cornerRadius < 0.0F ||
            strokeWidth <= 0.0F)
        {
            return;
        }

        Microsoft::WRL::ComPtr<
            ID2D1SolidColorBrush>
            brush;

        const HRESULT result =
            renderTarget_->CreateSolidColorBrush(
                D2D1::ColorF(
                    color.red,
                    color.green,
                    color.blue,
                    color.alpha),
                brush.GetAddressOf());

        if (FAILED(result))
        {
            return;
        }

        const D2D1_ROUNDED_RECT rectangle{
            D2D1::RectF(
                bounds.Left(),
                bounds.Top(),
                bounds.Right(),
                bounds.Bottom()),
            cornerRadius,
            cornerRadius};

        renderTarget_->DrawRoundedRectangle(
            rectangle,
            brush.Get(),
            strokeWidth);
    }

    void Renderer::Resize(
        UINT width,
        UINT height)
    {
        if (
            renderTarget_ == nullptr ||
            width == 0 ||
            height == 0)
        {
            return;
        }

        const HRESULT result =
            renderTarget_->Resize(
                D2D1::SizeU(width, height));

        if (FAILED(result))
        {
            DiscardWindowResources();
        }
    }

    void Renderer::DiscardWindowResources()
    {
        frameActive_ = false;
        renderTarget_.Reset();
    }

    void Renderer::Shutdown()
    {
        if (frameActive_)
        {
            EndFrame();
        }

        DiscardWindowResources();

        dwriteFactory_.Reset();
        d2dFactory_.Reset();

        windowHandle_ = nullptr;
        initialized_ = false;
    }

    bool Renderer::IsInitialized() const noexcept
    {
        return initialized_;
    }
}