#include "roui/controls/Checkbox.h"

#include "roui/core/Theme.h"
#include "roui/rendering/Renderer.h"

#include <utility>

namespace real_optimizer::roui
{
    void Checkbox::SetChecked(bool checked) noexcept
    {
        checked_ = checked;
    }

    bool Checkbox::IsChecked() const noexcept
    {
        return checked_;
    }

    void Checkbox::SetOnChanged(ChangedHandler handler)
    {
        onChanged_ = std::move(handler);
    }

    void Checkbox::SetStyle(Style style) noexcept
    {
        style_ = style;
    }

    void Checkbox::OnMouseMove(const Point& point)
    {
        hovered_ = ContainsPoint(point);
    }

    void Checkbox::OnMouseLeave()
    {
        hovered_ = false;
    }

    void Checkbox::OnMouseDown(const Point& point)
    {
        if (!ContainsPoint(point))
        {
            return;
        }

        pressedInside_ = true;
    }

    void Checkbox::OnMouseUp(const Point& point)
    {
        if (!pressedInside_)
        {
            return;
        }

        pressedInside_ = false;

        if (IsEnabled() && ContainsPoint(point))
        {
            checked_ = !checked_;

            if (onChanged_)
            {
                onChanged_();
            }
        }
    }

    void Checkbox::OnRender(
        rendering::Renderer& renderer
    )
    {
        const Palette& palette = Theme::Current();
        const Rect& bounds = GetBounds();
        const bool enabled = IsEnabled();

        if (hovered_ && enabled)
        {
            renderer.FillRoundedRectangle(
                bounds,
                10.0F,
                palette.hover);
        }

        if (style_ == Style::Switch)
        {
            const Rect track{
                bounds.x + 16.0F,
                bounds.y + (bounds.height - 22.0F) / 2.0F,
                40.0F,
                22.0F};

            Color trackColor = palette.track;

            if (checked_)
            {
                trackColor = enabled
                                 ? palette.accent
                                 : Color::Mix(palette.track, palette.accent, 0.35F);
            }

            renderer.FillRoundedRectangle(track, 11.0F, trackColor);

            if (!checked_)
            {
                renderer.DrawRoundedRectangle(
                    track,
                    11.0F,
                    1.0F,
                    enabled ? palette.outline : palette.track);
            }

            const float knobX =
                checked_
                    ? track.x + track.width - 3.0F - 16.0F
                    : track.x + 3.0F;

            renderer.FillRoundedRectangle(
                Rect{knobX, track.y + 3.0F, 16.0F, 16.0F},
                8.0F,
                checked_
                    ? palette.onAccent
                    : (enabled ? palette.textMuted : palette.outline));

            return;
        }

        const Rect box{
            bounds.x + 16.0F,
            bounds.y + (bounds.height - 20.0F) / 2.0F,
            20.0F,
            20.0F};

        if (checked_)
        {
            renderer.FillRoundedRectangle(
                box,
                6.0F,
                enabled
                    ? palette.accent
                    : palette.track);

            const Color mark =
                enabled
                    ? palette.onAccent
                    : palette.textMuted;

            renderer.DrawLine(
                Point{box.x + 5.0F, box.y + 10.5F},
                Point{box.x + 8.5F, box.y + 14.0F},
                2.0F,
                mark);

            renderer.DrawLine(
                Point{box.x + 8.5F, box.y + 14.0F},
                Point{box.x + 15.0F, box.y + 6.5F},
                2.0F,
                mark);
        }
        else
        {
            renderer.DrawRoundedRectangle(
                box,
                6.0F,
                1.5F,
                enabled
                    ? palette.outline
                    : palette.track);
        }
    }
}
