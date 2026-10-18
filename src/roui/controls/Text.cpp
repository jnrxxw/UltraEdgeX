#include "roui/controls/Text.h"

#include "roui/core/Theme.h"
#include "roui/rendering/Renderer.h"

#include <utility>

namespace real_optimizer::roui
{
    void Text::SetText(std::wstring text)
    {
        text_ = std::move(text);
    }

    void Text::SetFontSize(float fontSize) noexcept
    {
        if (fontSize > 0.0F)
        {
            fontSize_ = fontSize;
        }
    }

    void Text::SetFontWeight(
        FontWeight fontWeight
    ) noexcept
    {
        fontWeight_ = fontWeight;
    }

    void Text::SetColor(const Color& color) noexcept
    {
        color_ = color;
        colorRole_ = ColorRole::Custom;
    }

    void Text::SetColorRole(ColorRole role) noexcept
    {
        colorRole_ = role;
    }

    void Text::SetHorizontalAlignment(
        HorizontalAlignment alignment
    ) noexcept
    {
        horizontalAlignment_ = alignment;
    }

    void Text::SetVerticalAlignment(
        VerticalAlignment alignment
    ) noexcept
    {
        verticalAlignment_ = alignment;
    }

    const std::wstring& Text::GetText() const noexcept
    {
        return text_;
    }

    float Text::GetFontSize() const noexcept
    {
        return fontSize_;
    }

    FontWeight Text::GetFontWeight() const noexcept
    {
        return fontWeight_;
    }

    const Color& Text::GetColor() const noexcept
    {
        return color_;
    }

    void Text::OnRender(
        rendering::Renderer& renderer
    )
    {
        renderer.DrawText(
            text_,
            GetBounds(),
            fontSize_,
            fontWeight_,
            Theme::Resolve(colorRole_, color_),
            horizontalAlignment_,
            verticalAlignment_
        );
    }
}