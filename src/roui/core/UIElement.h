#pragma once

#include "roui/core/Geometry.h"

namespace real_optimizer::roui::rendering
{
    class Renderer;
}

namespace real_optimizer::roui
{
    class UIElement
    {
    public:
        UIElement() = default;
        virtual ~UIElement() = default;

        UIElement(const UIElement&) = delete;
        UIElement& operator=(const UIElement&) = delete;

        UIElement(UIElement&&) = delete;
        UIElement& operator=(UIElement&&) = delete;

        void SetBounds(const Rect& bounds) noexcept;
        void SetPosition(float x, float y) noexcept;
        void SetSize(float width, float height) noexcept;

        void SetVisible(bool visible) noexcept;
        void SetEnabled(bool enabled) noexcept;

        [[nodiscard]] const Rect& GetBounds() const noexcept;

        [[nodiscard]] float GetX() const noexcept;
        [[nodiscard]] float GetY() const noexcept;
        [[nodiscard]] float GetWidth() const noexcept;
        [[nodiscard]] float GetHeight() const noexcept;

        [[nodiscard]] bool IsVisible() const noexcept;
        [[nodiscard]] bool IsEnabled() const noexcept;

        void Render(rendering::Renderer& renderer);

        [[nodiscard]] bool ContainsPoint(
            const Point& point
        ) const noexcept;

        virtual void OnMouseMove(const Point& point);
        virtual void OnMouseLeave();
        virtual void OnMouseDown(const Point& point);
        virtual void OnMouseUp(const Point& point);

        // notches > 0: roda para cima (afasta do usuário);
        // 1.0 = um "clique" da roda.
        virtual void OnMouseWheel(const Point& point, float notches)
        {
            static_cast<void>(point);
            static_cast<void>(notches);
        }

    protected:
        virtual void OnRender(
            rendering::Renderer& renderer
        ) = 0;

    private:
        Rect bounds_{};
        bool visible_ = true;
        bool enabled_ = true;
    };
}