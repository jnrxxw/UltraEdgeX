#include "roui/controls/Button.h"

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

    void Button::OnMouseMove(const Point& point)
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

    void Button::OnMouseDown(const Point& point)
    {
        if (!ContainsPoint(point))
        {
            return;
        }

        pressedInside_ = true;
        state_ = State::Pressed;
    }

    void Button::OnMouseUp(const Point& point)
    {
        if (!pressedInside_)
        {
            return;
        }

        const bool shouldClick =
            ContainsPoint(point);

        pressedInside_ = false;

        state_ = shouldClick
            ? State::Hovered
            : State::Normal;

        if (shouldClick && onClick_)
        {
            onClick_();
        }
    }

    void Button::OnRender(
        rendering::Renderer& renderer
    )
    {
        Color backgroundColor =
            Color::FromRgb(22, 158, 242);

        switch (state_)
        {
        case State::Hovered:
            backgroundColor =
                Color::FromRgb(15, 201, 242);
            break;

        case State::Pressed:
            backgroundColor =
                Color::FromRgb(7, 170, 210);
            break;

        case State::Normal:
        default:
            break;
        }

        renderer.FillRoundedRectangle(
            GetBounds(),
            10.0F,
            backgroundColor
        );

        renderer.DrawText(
            text_,
            GetBounds(),
            16.0F,
            FontWeight::SemiBold,
            Color::FromRgb(13, 13, 13),
            HorizontalAlignment::Center,
            VerticalAlignment::Center
        );
    }
}