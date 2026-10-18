#include "roui/controls/Button.h"

#include "roui/core/Theme.h"
#include "roui/rendering/Renderer.h"

#include <utility>

namespace real_optimizer::roui
{
    void Button::SetText(std::wstring text)
    {
        text_ = std::move(text);
    }

    void Button::SetOnClick(ClickHandler handler)
    {
        onClick_ = std::move(handler);
    }

    void Button::SetStyle(Style style) noexcept
    {
        style_ = style;
    }

    void Button::SetActive(bool active) noexcept
    {
        active_ = active;
    }

    void Button::SetBadge(std::wstring badge)
    {
        badge_ = std::move(badge);
    }

    void Button::OnMouseMove(const Point &point)
    {
        if (!IsEnabled())
        {
            return;
        }

        const bool inside = ContainsPoint(point);

        if (pressedInside_)
        {
            state_ = inside
                         ? State::Pressed
                         : State::Normal;

            return;
        }

        state_ = inside
                     ? State::Hovered
                     : State::Normal;
    }

    void Button::OnMouseLeave()
    {
        if (!pressedInside_)
        {
            state_ = State::Normal;
        }
    }

    void Button::OnMouseDown(const Point &point)
    {
        if (
            !IsEnabled() ||
            !ContainsPoint(point))
        {
            return;
        }

        pressedInside_ = true;
        state_ = State::Pressed;
    }

    void Button::OnMouseUp(const Point &point)
    {
        if (!pressedInside_)
        {
            return;
        }

        const bool shouldClick =
            IsEnabled() &&
            ContainsPoint(point);

        pressedInside_ = false;

        state_ = shouldClick
                     ? State::Hovered
                     : State::Normal;

        if (
            shouldClick &&
            onClick_)
        {
            onClick_();
        }
    }

    void Button::OnRender(
        rendering::Renderer &renderer)
    {
        const Palette &palette = Theme::Current();
        const Rect &bounds = GetBounds();

        const bool enabled = IsEnabled();
        const bool hovered = enabled && state_ == State::Hovered;
        const bool pressed = enabled && state_ == State::Pressed;
        const bool highlighted = hovered || pressed;

        switch (style_)
        {
        case Style::Primary:
        {
            Color fill = palette.accent;

            if (!enabled)
            {
                fill = Color::Mix(
                    palette.surface,
                    palette.accent,
                    0.35F);
            }
            else if (pressed)
            {
                fill = Color::Mix(
                    palette.accent,
                    Color::FromRgb(0, 0, 0),
                    0.20F);
            }
            else if (hovered)
            {
                fill = Color::Mix(
                    palette.accent,
                    Color::FromRgb(255, 255, 255),
                    0.15F);
            }

            renderer.FillRoundedRectangle(
                bounds,
                12.0F,
                fill);

            renderer.DrawText(
                text_,
                bounds,
                14.0F,
                FontWeight::Bold,
                palette.onAccent,
                HorizontalAlignment::Center,
                VerticalAlignment::Center);

            break;
        }

        case Style::Secondary:
        {
            if (pressed)
            {
                renderer.FillRoundedRectangle(
                    bounds,
                    12.0F,
                    palette.accentSoft);
            }

            renderer.DrawRoundedRectangle(
                bounds,
                12.0F,
                1.0F,
                !enabled
                    ? palette.track
                    : (highlighted ? palette.accent : palette.outline));

            renderer.DrawText(
                text_,
                bounds,
                13.0F,
                FontWeight::SemiBold,
                !enabled
                    ? palette.textMuted
                    : (highlighted ? palette.accentText
                                   : palette.textPrimary),
                HorizontalAlignment::Center,
                VerticalAlignment::Center);

            break;
        }

        case Style::Nav:
        {
            if (active_)
            {
                renderer.FillRoundedRectangle(
                    bounds,
                    10.0F,
                    palette.accentSoft);
            }
            else if (highlighted)
            {
                renderer.FillRoundedRectangle(
                    bounds,
                    10.0F,
                    palette.hover);
            }

            Color textColor = palette.textSecondary;

            if (active_)
            {
                textColor = palette.accentText;
            }
            else if (!enabled)
            {
                textColor = palette.textMuted;
            }

            const Rect label{
                bounds.x + 14.0F,
                bounds.y,
                bounds.width - 28.0F,
                bounds.height};

            renderer.DrawText(
                text_,
                label,
                14.0F,
                active_
                    ? FontWeight::SemiBold
                    : FontWeight::Normal,
                textColor,
                HorizontalAlignment::Left,
                VerticalAlignment::Center);

            if (!badge_.empty())
            {
                constexpr float BadgeWidth = 66.0F;
                constexpr float BadgeHeight = 22.0F;

                const Rect badge{
                    bounds.Right() - BadgeWidth - 10.0F,
                    bounds.y + (bounds.height - BadgeHeight) / 2.0F,
                    BadgeWidth,
                    BadgeHeight};

                renderer.DrawRoundedRectangle(
                    badge,
                    BadgeHeight / 2.0F,
                    1.0F,
                    palette.outline);

                renderer.DrawText(
                    badge_,
                    badge,
                    11.0F,
                    FontWeight::Normal,
                    palette.textMuted,
                    HorizontalAlignment::Center,
                    VerticalAlignment::Center);
            }

            break;
        }

        case Style::Segment:
        {
            if (active_)
            {
                renderer.FillRoundedRectangle(
                    bounds,
                    9.0F,
                    palette.accentSoft);
            }
            else if (highlighted)
            {
                renderer.FillRoundedRectangle(
                    bounds,
                    9.0F,
                    palette.hover);
            }

            renderer.DrawText(
                text_,
                bounds,
                13.0F,
                FontWeight::SemiBold,
                active_
                    ? palette.accentText
                    : palette.textSecondary,
                HorizontalAlignment::Center,
                VerticalAlignment::Center);

            break;
        }
        }
    }
}
