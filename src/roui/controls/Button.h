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

        enum class Style
        {
            Primary,   // preenchido na cor de destaque
            Secondary, // contorno
            Nav,       // item da barra lateral
            Segment    // opção de um seletor segmentado
        };

        Button() = default;
        ~Button() override = default;

        void SetText(std::wstring text);
        void SetOnClick(ClickHandler handler);
        void SetStyle(Style style) noexcept;

        // Estado "selecionado" (Nav e Segment).
        void SetActive(bool active) noexcept;

        // Selo à direita (Nav), ex.: "Em breve".
        void SetBadge(std::wstring badge);

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
        std::wstring badge_;
        ClickHandler onClick_;

        Style style_ = Style::Primary;
        bool active_ = false;

        State state_ = State::Normal;
        bool pressedInside_ = false;
    };
}
