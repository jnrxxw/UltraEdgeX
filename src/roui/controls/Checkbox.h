#pragma once

#include "roui/core/UIElement.h"

#include <functional>

namespace real_optimizer::roui
{
    // Caixa de seleção. A área clicável é todo o retângulo do
    // elemento (útil para linhas de lista); a caixa é desenhada
    // à esquerda, centralizada na vertical.
    class Checkbox final : public UIElement
    {
    public:
        using ChangedHandler = std::function<void()>;

        enum class Style
        {
            Box,   // caixa com marca de seleção
            Switch // interruptor liga/desliga
        };

        Checkbox() = default;
        ~Checkbox() override = default;

        void SetChecked(bool checked) noexcept;
        [[nodiscard]] bool IsChecked() const noexcept;

        void SetOnChanged(ChangedHandler handler);
        void SetStyle(Style style) noexcept;

        void OnMouseMove(const Point& point) override;
        void OnMouseLeave() override;
        void OnMouseDown(const Point& point) override;
        void OnMouseUp(const Point& point) override;

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

    private:
        Style style_ = Style::Box;
        bool checked_ = false;
        bool hovered_ = false;
        bool pressedInside_ = false;

        ChangedHandler onChanged_;
    };
}
