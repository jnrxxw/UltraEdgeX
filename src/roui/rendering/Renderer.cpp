#include "roui/rendering/Renderer.h"

#include "roui/core/Theme.h"

#include <d2d1helper.h>

#include <cmath>
#include <vector>

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

        if (FAILED(result))
        {
            return false;
        }

        if (wicFactory_ == nullptr)
        {
            CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(wicFactory_.GetAddressOf()));
        }

        return true;
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

        const Color &background =
            Theme::Current().background;

        renderTarget_->Clear(
            D2D1::ColorF(
                background.red,
                background.green,
                background.blue,
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

    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>
    Renderer::CreateBrush(const Color &color) const
    {
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;

        if (renderTarget_ == nullptr)
        {
            return brush;
        }

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
            brush.Reset();
        }

        return brush;
    }

    void Renderer::DrawCircle(
        const Point &center,
        float radius,
        float strokeWidth,
        const Color &color)
    {
        if (
            !frameActive_ ||
            renderTarget_ == nullptr ||
            radius <= 0.0F ||
            strokeWidth <= 0.0F)
        {
            return;
        }

        const auto brush = CreateBrush(color);

        if (brush == nullptr)
        {
            return;
        }

        renderTarget_->DrawEllipse(
            D2D1::Ellipse(
                D2D1::Point2F(center.x, center.y),
                radius,
                radius),
            brush.Get(),
            strokeWidth);
    }

    void Renderer::PushClip(const Rect &bounds)
    {
        if (!frameActive_ || renderTarget_ == nullptr)
        {
            return;
        }

        renderTarget_->PushAxisAlignedClip(
            D2D1::RectF(
                bounds.x,
                bounds.y,
                bounds.x + bounds.width,
                bounds.y + bounds.height),
            D2D1_ANTIALIAS_MODE_ALIASED);
    }

    void Renderer::PopClip()
    {
        if (!frameActive_ || renderTarget_ == nullptr)
        {
            return;
        }

        renderTarget_->PopAxisAlignedClip();
    }

    void Renderer::DrawLine(
        const Point &from,
        const Point &to,
        float strokeWidth,
        const Color &color)
    {
        if (
            !frameActive_ ||
            renderTarget_ == nullptr ||
            d2dFactory_ == nullptr ||
            strokeWidth <= 0.0F)
        {
            return;
        }

        const auto brush = CreateBrush(color);

        if (brush == nullptr)
        {
            return;
        }

        Microsoft::WRL::ComPtr<ID2D1StrokeStyle> strokeStyle;

        d2dFactory_->CreateStrokeStyle(
            D2D1::StrokeStyleProperties(
                D2D1_CAP_STYLE_ROUND,
                D2D1_CAP_STYLE_ROUND,
                D2D1_CAP_STYLE_ROUND,
                D2D1_LINE_JOIN_ROUND),
            nullptr,
            0,
            strokeStyle.GetAddressOf());

        renderTarget_->DrawLine(
            D2D1::Point2F(from.x, from.y),
            D2D1::Point2F(to.x, to.y),
            brush.Get(),
            strokeWidth,
            strokeStyle.Get());
    }

    void Renderer::DrawArc(
        const Point &center,
        float radius,
        float startAngleDegrees,
        float sweepDegrees,
        float strokeWidth,
        const Color &color)
    {
        if (
            !frameActive_ ||
            renderTarget_ == nullptr ||
            d2dFactory_ == nullptr ||
            radius <= 0.0F ||
            strokeWidth <= 0.0F ||
            sweepDegrees <= 0.0F)
        {
            return;
        }

        if (sweepDegrees >= 359.9F)
        {
            DrawCircle(center, radius, strokeWidth, color);
            return;
        }

        constexpr float Pi = 3.14159265358979F;

        const auto pointAt =
            [&center, radius](float degrees)
        {
            const float radians = degrees * Pi / 180.0F;

            return D2D1::Point2F(
                center.x + radius * std::cos(radians),
                center.y + radius * std::sin(radians));
        };

        const auto brush = CreateBrush(color);

        if (brush == nullptr)
        {
            return;
        }

        Microsoft::WRL::ComPtr<ID2D1PathGeometry> geometry;

        if (FAILED(d2dFactory_->CreatePathGeometry(
                geometry.GetAddressOf())))
        {
            return;
        }

        Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;

        if (FAILED(geometry->Open(sink.GetAddressOf())))
        {
            return;
        }

        // O arco é desenhado como uma linha com muitos segmentos
        // curtos (1 a cada ~2°). Fica idêntico a um arco para o olho
        // e não depende do suporte a ArcSegment do driver gráfico.
        const int segments =
            static_cast<int>(sweepDegrees / 2.0F) + 1;

        sink->BeginFigure(
            pointAt(startAngleDegrees),
            D2D1_FIGURE_BEGIN_HOLLOW);

        std::vector<D2D1_POINT_2F> points;
        points.reserve(static_cast<std::size_t>(segments));

        for (int i = 1; i <= segments; ++i)
        {
            const float angle =
                startAngleDegrees +
                sweepDegrees * static_cast<float>(i) /
                    static_cast<float>(segments);

            points.push_back(pointAt(angle));
        }

        sink->AddLines(
            points.data(),
            static_cast<UINT32>(points.size()));

        sink->EndFigure(D2D1_FIGURE_END_OPEN);

        if (FAILED(sink->Close()))
        {
            return;
        }

        Microsoft::WRL::ComPtr<ID2D1StrokeStyle> strokeStyle;

        d2dFactory_->CreateStrokeStyle(
            D2D1::StrokeStyleProperties(
                D2D1_CAP_STYLE_ROUND,
                D2D1_CAP_STYLE_ROUND,
                D2D1_CAP_STYLE_ROUND,
                D2D1_LINE_JOIN_ROUND),
            nullptr,
            0,
            strokeStyle.GetAddressOf());

        renderTarget_->DrawGeometry(
            geometry.Get(),
            brush.Get(),
            strokeWidth,
            strokeStyle.Get());
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

        wicFactory_.Reset();
        dwriteFactory_.Reset();
        d2dFactory_.Reset();

        windowHandle_ = nullptr;
        initialized_ = false;
    }

    bool Renderer::IsInitialized() const noexcept
    {
        return initialized_;
    }

    Microsoft::WRL::ComPtr<ID2D1Bitmap> Renderer::CreateBitmapFromResource(
        HMODULE module,
        int resourceId)
    {
        if (resourceId == 0)
        {
            return nullptr;
        }

        if (module == nullptr)
        {
            module = GetModuleHandleW(nullptr);
        }

        HRSRC imageResHandle = FindResourceW(
            module,
            MAKEINTRESOURCEW(resourceId),
            RT_RCDATA);

        if (imageResHandle == nullptr)
        {
            return nullptr;
        }

        HGLOBAL imageResDataHandle = LoadResource(module, imageResHandle);
        if (imageResDataHandle == nullptr)
        {
            return nullptr;
        }

        const void* imageResource = LockResource(imageResDataHandle);
        const DWORD imageFileSize = SizeofResource(module, imageResHandle);
        if (imageResource == nullptr || imageFileSize == 0)
        {
            return nullptr;
        }

        return CreateBitmapFromMemory(imageResource, imageFileSize);
    }

    Microsoft::WRL::ComPtr<ID2D1Bitmap> Renderer::CreateBitmapFromFile(
        const std::wstring& filePath)
    {
        if (filePath.empty())
        {
            return nullptr;
        }

        if (wicFactory_ == nullptr)
        {
            CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(wicFactory_.GetAddressOf()));
        }

        if (wicFactory_ == nullptr || !CreateWindowResources() || renderTarget_ == nullptr)
        {
            return nullptr;
        }

        Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
        HRESULT hr = wicFactory_->CreateDecoderFromFilename(
            filePath.c_str(),
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnDemand,
            decoder.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }

        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
        hr = decoder->GetFrame(0, frame.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }

        Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
        hr = wicFactory_->CreateFormatConverter(converter.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }

        hr = converter->Initialize(
            frame.Get(),
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
        {
            return nullptr;
        }

        Microsoft::WRL::ComPtr<ID2D1Bitmap> d2dBitmap;
        hr = renderTarget_->CreateBitmapFromWicBitmap(
            converter.Get(),
            nullptr,
            d2dBitmap.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }

        return d2dBitmap;
    }

    Microsoft::WRL::ComPtr<ID2D1Bitmap> Renderer::CreateBitmapFromMemory(
        const void* data,
        size_t size)
    {
        if (data == nullptr || size == 0)
        {
            return nullptr;
        }

        if (wicFactory_ == nullptr)
        {
            CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(wicFactory_.GetAddressOf()));
        }

        if (wicFactory_ == nullptr || !CreateWindowResources() || renderTarget_ == nullptr)
        {
            return nullptr;
        }

        Microsoft::WRL::ComPtr<IWICStream> stream;
        HRESULT hr = wicFactory_->CreateStream(stream.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }

        hr = stream->InitializeFromMemory(
            reinterpret_cast<WICInProcPointer>(const_cast<void*>(data)),
            static_cast<DWORD>(size));
        if (FAILED(hr))
        {
            return nullptr;
        }

        Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
        hr = wicFactory_->CreateDecoderFromStream(
            stream.Get(),
            nullptr,
            WICDecodeMetadataCacheOnDemand,
            decoder.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }

        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
        hr = decoder->GetFrame(0, frame.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }

        Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
        hr = wicFactory_->CreateFormatConverter(converter.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }

        hr = converter->Initialize(
            frame.Get(),
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom);
        if (FAILED(hr))
        {
            return nullptr;
        }

        Microsoft::WRL::ComPtr<ID2D1Bitmap> d2dBitmap;
        hr = renderTarget_->CreateBitmapFromWicBitmap(
            converter.Get(),
            nullptr,
            d2dBitmap.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }

        return d2dBitmap;
    }

    bool Renderer::DrawBitmap(
        ID2D1Bitmap* bitmap,
        const Rect& bounds,
        float opacity)
    {
        if (!frameActive_ || renderTarget_ == nullptr || bitmap == nullptr || bounds.IsEmpty())
        {
            return false;
        }

        const D2D1_RECT_F destination = D2D1::RectF(
            bounds.Left(),
            bounds.Top(),
            bounds.Right(),
            bounds.Bottom());

        renderTarget_->DrawBitmap(
            bitmap,
            destination,
            opacity,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);

        return true;
    }
}