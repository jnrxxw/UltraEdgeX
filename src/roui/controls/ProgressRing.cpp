#include "roui/controls/ProgressRing.h"

#include "roui/core/Theme.h"
#include "roui/rendering/Renderer.h"

#include <utility>

namespace real_optimizer::roui
{
    void ProgressRing::SetValue(float percent) noexcept
    {
        if (percent > 100.0F)
        {
            percent = 100.0F;
        }

        percent_ = percent;
    }

    void ProgressRing::SetCaption(std::wstring caption)
    {
        caption_ = std::move(caption);
    }

    void ProgressRing::OnRender(
        rendering::Renderer& renderer
    )
    {
        const Palette& palette = Theme::Current();
        const Rect& bounds = GetBounds();

        constexpr float Stroke = 12.0F;

        const Point center{
            bounds.x + bounds.width / 2.0F,
            bounds.y + bounds.height / 2.0F
        };

        const float smallestSide =
            bounds.width < bounds.height
                ? bounds.width
                : bounds.height;

        const float radius =
            smallestSide / 2.0F - Stroke / 2.0F;

        renderer.DrawCircle(
            center,
            radius,
            Stroke,
            palette.track);

        if (percent_ > 0.0F)
        {
            renderer.DrawArc(
                center,
                radius,
                -90.0F,
                percent_ * 3.6F,
                Stroke,
                palette.accent);
        }

        const std::wstring valueText =
            percent_ < 0.0F
                ? std::wstring(L"--")
                : std::to_wstring(
                      static_cast<int>(percent_ + 0.5F));

        renderer.DrawText(
            valueText,
            Rect{center.x - 50.0F, center.y - 30.0F, 100.0F, 44.0F},
            36.0F,
            FontWeight::Bold,
            palette.accentText,
            HorizontalAlignment::Center,
            VerticalAlignment::Center);

        renderer.DrawText(
            caption_,
            Rect{center.x - 50.0F, center.y + 14.0F, 100.0F, 18.0F},
            12.0F,
            FontWeight::Normal,
            palette.textSecondary,
            HorizontalAlignment::Center,
            VerticalAlignment::Center);
    }
}
