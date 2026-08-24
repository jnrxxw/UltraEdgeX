#pragma once

#include "roui/core/UIElement.h"
#include "roui/core/VisualTypes.h"

#include <functional>
#include <string>

namespace real_optimizer::roui
{
    class Button final : public UIElement
    {
    public:
        using ClickHandler = std::function<void()>;

        Button() = default;
        ~Button() override = default;

        void SetText(std::wstring text);
        void SetOnClick(ClickHandler handler);

        void OnMouseMove(const Point& point) override;
        void OnMouseLeave() override;
        void OnMouseDown(const Point& point) override;
        void OnMouseUp(const Point& point) override;

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

    private:
        enum class State
        {
            Normal,
            Hovered,
            Pressed
        };

        std::wstring text_;
        ClickHandler onClick_;

        State state_ = State::Normal;
        bool pressedInside_ = false;
    };
}