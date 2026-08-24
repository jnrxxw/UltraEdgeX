#include "roui/core/UIElement.h"

namespace real_optimizer::roui
{
    void UIElement::SetBounds(const Rect& bounds) noexcept
    {
        bounds_ = bounds;
    }

    void UIElement::SetPosition(float x, float y) noexcept
    {
        bounds_.x = x;
        bounds_.y = y;
    }

    void UIElement::SetSize(float width, float height) noexcept
    {
        bounds_.width = width;
        bounds_.height = height;
    }

    void UIElement::SetVisible(bool visible) noexcept
    {
        visible_ = visible;
    }

    void UIElement::SetEnabled(bool enabled) noexcept
    {
        enabled_ = enabled;
    }

    const Rect& UIElement::GetBounds() const noexcept
    {
        return bounds_;
    }

    float UIElement::GetX() const noexcept
    {
        return bounds_.x;
    }

    float UIElement::GetY() const noexcept
    {
        return bounds_.y;
    }

    float UIElement::GetWidth() const noexcept
    {
        return bounds_.width;
    }

    float UIElement::GetHeight() const noexcept
    {
        return bounds_.height;
    }

    bool UIElement::IsVisible() const noexcept
    {
        return visible_;
    }

    bool UIElement::IsEnabled() const noexcept
    {
        return enabled_;
    }

    void UIElement::Render(rendering::Renderer& renderer)
    {
        if (!visible_ || bounds_.IsEmpty())
        {
            return;
        }

        OnRender(renderer);
    }
         bool UIElement::ContainsPoint(
        const Point& point
    ) const noexcept
    {
        return visible_ &&
               enabled_ &&
               bounds_.Contains(point);
    }    

    void UIElement::OnMouseMove(const Point& point)
    {
        static_cast<void>(point);
    }

    void UIElement::OnMouseLeave()
    {
    }
    
    void UIElement::OnMouseDown(const Point& point)
    {
        static_cast<void>(point);
    }

    void UIElement::OnMouseUp(const Point& point)
    {
        static_cast<void>(point);
    }
}