#include "roui/controls/ScrollPanel.h"

#include "roui/core/Theme.h"
#include "roui/rendering/Renderer.h"

#include <algorithm>
#include <utility>

namespace real_optimizer::roui
{
    namespace
    {
        constexpr float WheelStep = 64.0F;
    }

    float ScrollPanel::MaxOffset() const noexcept
    {
        return std::max(0.0F, contentHeight_ - GetHeight());
    }

    void ScrollPanel::SetContentHeight(float height) noexcept
    {
        contentHeight_ = std::max(0.0F, height);
        offset_ = std::clamp(offset_, 0.0F, MaxOffset());
    }

    float ScrollPanel::GetScrollOffset() const noexcept
    {
        return offset_;
    }

    void ScrollPanel::SetScrollOffset(float offset) noexcept
    {
        offset_ = std::clamp(offset, 0.0F, MaxOffset());
    }

    void ScrollPanel::SetOnScroll(ScrollHandler handler)
    {
        onScroll_ = std::move(handler);
    }

    void ScrollPanel::OnMouseMove(const Point& point)
    {
        if (ContainsPoint(point))
        {
            Panel::OnMouseMove(point);
        }
        else
        {
            // Fora da área visível: nada fica "aceso".
            Panel::OnMouseLeave();
        }
    }

    void ScrollPanel::OnMouseDown(const Point& point)
    {
        // Linhas parcialmente escondidas não recebem clique
        // fora da área visível.
        if (ContainsPoint(point))
        {
            Panel::OnMouseDown(point);
        }
    }

    void ScrollPanel::OnMouseWheel(
        const Point& point,
        float notches)
    {
        if (!ContainsPoint(point) || MaxOffset() <= 0.0F)
        {
            return;
        }

        const float previous = offset_;

        SetScrollOffset(offset_ - notches * WheelStep);

        if (offset_ != previous && onScroll_)
        {
            onScroll_();
        }
    }

    void ScrollPanel::OnRender(
        rendering::Renderer& renderer)
    {
        const Rect& bounds = GetBounds();

        renderer.PushClip(bounds);
        Panel::OnRender(renderer);
        renderer.PopClip();

        // Barra de rolagem fina, só quando há o que rolar.
        const float maxOffset = MaxOffset();

        if (maxOffset > 0.0F && contentHeight_ > 0.0F)
        {
            const Palette& palette = Theme::Current();

            const float trackHeight = bounds.height - 8.0F;

            const float thumbHeight = std::max(
                32.0F,
                trackHeight * (bounds.height / contentHeight_));

            const float thumbY =
                bounds.y + 4.0F +
                (trackHeight - thumbHeight) * (offset_ / maxOffset);

            renderer.FillRoundedRectangle(
                Rect{bounds.x + bounds.width - 7.0F, thumbY, 4.0F, thumbHeight},
                2.0F,
                palette.outline);
        }
    }
}
