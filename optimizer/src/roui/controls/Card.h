#pragma once

#include "roui/core/UIElement.h"
#include "roui/core/VisualTypes.h"

#include <string>

namespace real_optimizer::roui
{
    class Card final : public UIElement
    {
    public:
        Card() = default;
        ~Card() override = default;

        void SetTitle(std::wstring title);
        void SetValue(std::wstring value);
        void SetDescription(std::wstring description);

        void SetBackgroundColor(const Color& color) noexcept;
        void SetBorderColor(const Color& color) noexcept;
        void SetAccentColor(const Color& color) noexcept;

        [[nodiscard]] const std::wstring&
        GetTitle() const noexcept;

        [[nodiscard]] const std::wstring&
        GetValue() const noexcept;

        [[nodiscard]] const std::wstring&
        GetDescription() const noexcept;

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

    private:
        std::wstring title_;
        std::wstring value_;
        std::wstring description_;

        Color backgroundColor_ =
            Color::FromRgb(23, 23, 23);

        Color borderColor_ =
            Color::FromRgb(45, 45, 45);

        Color accentColor_ =
            Color::FromRgb(7, 242, 242);
    };
}