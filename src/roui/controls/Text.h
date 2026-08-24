#pragma once

#include "roui/core/UIElement.h"
#include "roui/core/VisualTypes.h"

#include <string>

namespace real_optimizer::roui
{
    class Text final : public UIElement
    {
    public:
        Text() = default;
        ~Text() override = default;

        void SetText(std::wstring text);
        void SetFontSize(float fontSize) noexcept;
        void SetFontWeight(FontWeight fontWeight) noexcept;
        void SetColor(const Color& color) noexcept;

        void SetHorizontalAlignment(
            HorizontalAlignment alignment
        ) noexcept;

        void SetVerticalAlignment(
            VerticalAlignment alignment
        ) noexcept;

        [[nodiscard]] const std::wstring& GetText() const noexcept;
        [[nodiscard]] float GetFontSize() const noexcept;
        [[nodiscard]] FontWeight GetFontWeight() const noexcept;
        [[nodiscard]] const Color& GetColor() const noexcept;

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

    private:
        std::wstring text_;
        float fontSize_ = 16.0F;
        FontWeight fontWeight_ = FontWeight::Normal;
        Color color_ = Color::FromRgb(255, 255, 255);

        HorizontalAlignment horizontalAlignment_ =
            HorizontalAlignment::Left;

        VerticalAlignment verticalAlignment_ =
            VerticalAlignment::Top;
    };
}