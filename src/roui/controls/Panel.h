#pragma once

#include "roui/core/UIElement.h"

#include <memory>
#include <vector>

namespace real_optimizer::roui
{
    class Panel : public UIElement
    {
    public:
        Panel() = default;
        ~Panel() override = default;

        UIElement* AddChild(
            std::unique_ptr<UIElement> child
        );

        void ClearChildren() noexcept;

        // Quando o filho modal está visível, só ele recebe o mouse
        // (os outros elementos ficam "congelados" por baixo).
        // Usado por caixas de diálogo.
        void SetModalChild(UIElement* child) noexcept;

        [[nodiscard]] std::size_t GetChildCount() const noexcept;

        void OnMouseMove(const Point& point) override;
        void OnMouseLeave() override;
        void OnMouseDown(const Point& point) override;
        void OnMouseUp(const Point& point) override;
        void OnMouseWheel(const Point& point, float notches) override;

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

        [[nodiscard]] std::vector<
        std::unique_ptr<UIElement>
        >& GetChildren() noexcept;

        [[nodiscard]] const std::vector<
        std::unique_ptr<UIElement>
        >& GetChildren() const noexcept;

    private:
        [[nodiscard]] bool IsModalActive() const noexcept;
        void ResetHoverOfOtherChildren();

        std::vector<std::unique_ptr<UIElement>> children_;

        UIElement* modalChild_ = nullptr;
        bool hoverResetForModal_ = false;
    };
}