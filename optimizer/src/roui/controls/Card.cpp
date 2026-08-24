#include "roui/controls/Card.h"

#include "roui/rendering/Renderer.h"

#include <utility>

namespace real_optimizer::roui
{
    void Card::SetTitle(std::wstring title)
    {
        title_ = std::move(title);
    }

    void Card::SetValue(std::wstring value)
    {
        value_ = std::move(value);
    }

    void Card::SetDescription(
        std::wstring description
    )
    {
        description_ = std::move(description);
    }

    void Card::SetBackgroundColor(
        const Color& color
    ) noexcept
    {
        backgroundColor_ = color;
    }

    void Card::SetBorderColor(
        const Color& color
    ) noexcept
    {
        borderColor_ = color;
    }

    void Card::SetAccentColor(
        const Color& color
    ) noexcept
    {
        accentColor_ = color;
    }

    const std::wstring&
    Card::GetTitle() const noexcept
    {
        return title_;
    }

    const std::wstring&
    Card::GetValue() const noexcept
    {
        return value_;
    }

    const std::wstring&
    Card::GetDescription() const noexcept
    {
        return description_;
    }

    void Card::OnRender(
        rendering::Renderer& renderer
    )
    {
        const Rect& bounds = GetBounds();

        renderer.FillRoundedRectangle(
            bounds,
            12.0F,
            backgroundColor_
        );

        renderer.DrawRoundedRectangle(
            bounds,
            12.0F,
            1.0F,
            borderColor_
        );

        const Rect titleBounds{
            bounds.x + 20.0F,
            bounds.y + 16.0F,
            bounds.width - 40.0F,
            24.0F
        };

        renderer.DrawText(
            title_,
            titleBounds,
            14.0F,
            FontWeight::SemiBold,
            Color::FromRgb(190, 190, 190),
            HorizontalAlignment::Left,
            VerticalAlignment::Center
        );

        const Rect valueBounds{
            bounds.x + 20.0F,
            bounds.y + 50.0F,
            bounds.width - 40.0F,
            44.0F
        };

        renderer.DrawText(
            value_,
            valueBounds,
            30.0F,
            FontWeight::Bold,
            accentColor_,
            HorizontalAlignment::Left,
            VerticalAlignment::Center
        );

        const Rect descriptionBounds{
            bounds.x + 20.0F,
            bounds.y + bounds.height - 42.0F,
            bounds.width - 40.0F,
            24.0F
        };

        renderer.DrawText(
            description_,
            descriptionBounds,
            13.0F,
            FontWeight::Normal,
            Color::FromRgb(150, 150, 150),
            HorizontalAlignment::Left,
            VerticalAlignment::Center
        );
    }
}