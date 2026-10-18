#include "roui/controls/Card.h"

#include "roui/core/Theme.h"
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

    void Card::SetProgress(float fraction) noexcept
    {
        if (fraction < 0.0F)
        {
            fraction = 0.0F;
        }
        else if (fraction > 1.0F)
        {
            fraction = 1.0F;
        }

        progress_ = fraction;
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

    //
    // SETA DE SELEÇÃO
    //

    namespace
    {
        constexpr float SelectorWidth = 34.0F;
        constexpr float SelectorHeight = 30.0F;
        constexpr float SelectorMargin = 12.0F;
    }

    void Card::SetSelectorVisible(bool visible) noexcept
    {
        selectorVisible_ = visible;

        if (!visible)
        {
            selectorHover_ = false;
            selectorPressed_ = false;
            selectorOpen_ = false;
        }
    }

    void Card::SetSelectorOpen(bool open) noexcept
    {
        selectorOpen_ = open && selectorVisible_;
    }

    void Card::SetOnSelectorClick(std::function<void()> handler)
    {
        onSelectorClick_ = std::move(handler);
    }

    Rect Card::GetSelectorBounds() const noexcept
    {
        const Rect& bounds = GetBounds();

        return Rect{
            bounds.x + bounds.width - SelectorMargin - SelectorWidth,
            bounds.y + 10.0F,
            SelectorWidth,
            SelectorHeight};
    }

    bool Card::InsideSelector(const Point& point) const noexcept
    {
        if (!selectorVisible_)
        {
            return false;
        }

        const Rect area = GetSelectorBounds();

        return point.x >= area.x &&
               point.x <= area.x + area.width &&
               point.y >= area.y &&
               point.y <= area.y + area.height;
    }

    void Card::OnMouseMove(const Point& point)
    {
        selectorHover_ = InsideSelector(point);
    }

    void Card::OnMouseLeave()
    {
        selectorHover_ = false;
    }

    void Card::OnMouseDown(const Point& point)
    {
        selectorPressed_ = InsideSelector(point);
    }

    void Card::OnMouseUp(const Point& point)
    {
        const bool shouldClick =
            selectorPressed_ && InsideSelector(point);

        selectorPressed_ = false;

        if (shouldClick && onSelectorClick_)
        {
            onSelectorClick_();
        }
    }

    void Card::OnRender(
        rendering::Renderer& renderer
    )
    {
        const Palette& palette = Theme::Current();
        const Rect& bounds = GetBounds();

        constexpr float Padding = 22.0F;

        renderer.FillRoundedRectangle(
            bounds,
            16.0F,
            palette.surface
        );

        renderer.DrawRoundedRectangle(
            bounds,
            16.0F,
            1.0F,
            palette.border
        );

        const float contentWidth =
            bounds.width - Padding * 2.0F;

        const float titleWidth =
            selectorVisible_
                ? contentWidth - (SelectorWidth + 8.0F)
                : contentWidth;

        renderer.DrawText(
            title_,
            Rect{bounds.x + Padding, bounds.y + 16.0F, titleWidth, 18.0F},
            12.0F,
            FontWeight::SemiBold,
            palette.textSecondary,
            HorizontalAlignment::Left,
            VerticalAlignment::Center
        );

        renderer.DrawText(
            value_,
            Rect{bounds.x + Padding, bounds.y + 36.0F, contentWidth, 44.0F},
            32.0F,
            FontWeight::Bold,
            palette.accentText,
            HorizontalAlignment::Left,
            VerticalAlignment::Center
        );

        renderer.DrawText(
            description_,
            Rect{bounds.x + Padding, bounds.y + 80.0F, contentWidth, 18.0F},
            13.0F,
            FontWeight::Normal,
            palette.textSecondary,
            HorizontalAlignment::Left,
            VerticalAlignment::Center
        );

        const Rect track{
            bounds.x + Padding,
            bounds.y + bounds.height - 22.0F,
            contentWidth,
            6.0F
        };

        renderer.FillRoundedRectangle(
            track,
            3.0F,
            palette.track
        );

        const float filledWidth =
            contentWidth * progress_;

        if (filledWidth > 0.5F)
        {
            renderer.FillRoundedRectangle(
                Rect{track.x, track.y, filledWidth, track.height},
                3.0F,
                palette.accent
            );
        }

        if (selectorVisible_)
        {
            const Rect area = GetSelectorBounds();

            if (selectorHover_ || selectorOpen_)
            {
                renderer.FillRoundedRectangle(
                    area,
                    8.0F,
                    palette.hover
                );
            }

            const float centerX = area.x + area.width / 2.0F;
            const float centerY = area.y + area.height / 2.0F;

            // Seta para baixo; para cima com o menu aberto.
            const float tip = selectorOpen_ ? -2.5F : 2.5F;

            const Color arrowColor =
                (selectorHover_ || selectorOpen_)
                    ? palette.textPrimary
                    : palette.textSecondary;

            renderer.DrawLine(
                Point{centerX - 5.0F, centerY - tip},
                Point{centerX, centerY + tip},
                1.8F,
                arrowColor
            );

            renderer.DrawLine(
                Point{centerX, centerY + tip},
                Point{centerX + 5.0F, centerY - tip},
                1.8F,
                arrowColor
            );
        }
    }
}