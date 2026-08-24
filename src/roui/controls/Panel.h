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

        [[nodiscard]] std::size_t GetChildCount() const noexcept;

        void OnMouseMove(const Point& point) override;
        void OnMouseLeave() override;
        void OnMouseDown(const Point& point) override;
        void OnMouseUp(const Point& point) override;

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
        std::vector<std::unique_ptr<UIElement>> children_;
    };
}