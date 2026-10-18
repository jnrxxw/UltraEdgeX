#include "roui/controls/Panel.h"

#include "roui/rendering/Renderer.h"

#include <utility>

namespace real_optimizer::roui
{
    UIElement* Panel::AddChild(
        std::unique_ptr<UIElement> child
    )
    {
        if (child == nullptr)
        {
            return nullptr;
        }

        UIElement* childPointer =
            child.get();

        children_.push_back(
            std::move(child)
        );

        return childPointer;
    }

    void Panel::ClearChildren() noexcept
    {
        modalChild_ = nullptr;
        children_.clear();
    }

    void Panel::SetModalChild(UIElement* child) noexcept
    {
        modalChild_ = child;
    }

    bool Panel::IsModalActive() const noexcept
    {
        return modalChild_ != nullptr && modalChild_->IsVisible();
    }

    // Ao abrir um diálogo, os botões por baixo podem estar com o
    // estado "hover"; limpa uma vez para não ficarem acesos.
    void Panel::ResetHoverOfOtherChildren()
    {
        if (hoverResetForModal_)
        {
            return;
        }

        hoverResetForModal_ = true;

        for (auto& child : children_)
        {
            if (child != nullptr && child.get() != modalChild_)
            {
                child->OnMouseLeave();
            }
        }
    }

    std::size_t Panel::GetChildCount()
        const noexcept
    {
        return children_.size();
    }

    void Panel::OnRender(
        rendering::Renderer& renderer
    )
    {
        for (const auto& child : children_)
        {
            if (
                child != nullptr &&
                child->IsVisible()
            )
            {
                child->Render(renderer);
            }
        }
    }

    void Panel::OnMouseMove(
        const Point& point
    )
    {
        if (IsModalActive())
        {
            ResetHoverOfOtherChildren();
            modalChild_->OnMouseMove(point);
            return;
        }

        hoverResetForModal_ = false;

        for (auto& child : children_)
        {
            if (
                child != nullptr &&
                child->IsVisible() &&
                child->IsEnabled()
            )
            {
                child->OnMouseMove(point);
            }
        }
    }

    void Panel::OnMouseLeave()
    {
        if (IsModalActive())
        {
            ResetHoverOfOtherChildren();
            modalChild_->OnMouseLeave();
            return;
        }

        for (auto& child : children_)
        {
            if (child != nullptr)
            {
                child->OnMouseLeave();
            }
        }
    }

    void Panel::OnMouseDown(
        const Point& point
    )
    {
        if (IsModalActive())
        {
            ResetHoverOfOtherChildren();
            modalChild_->OnMouseDown(point);
            return;
        }

        //
        // Propaga o evento para os filhos.
        //
        // Cada elemento decide internamente
        // se o ponto pertence a ele.
        //
        // Isso também permite que Panels
        // aninhados encaminhem corretamente
        // o clique para seus próprios filhos.
        //

        for (auto& child : children_)
        {
            if (
                child != nullptr &&
                child->IsVisible() &&
                child->IsEnabled()
            )
            {
                child->OnMouseDown(point);
            }
        }
    }

    void Panel::OnMouseUp(
        const Point& point
    )
    {
        if (IsModalActive())
        {
            modalChild_->OnMouseUp(point);
            return;
        }

        //
        // Mantemos MouseDown e MouseUp
        // com a mesma estratégia de
        // propagação.
        //

        for (auto& child : children_)
        {
            if (
                child != nullptr &&
                child->IsVisible() &&
                child->IsEnabled()
            )
            {
                child->OnMouseUp(point);
            }
        }
    }

    void Panel::OnMouseWheel(
        const Point& point,
        float notches
    )
    {
        if (IsModalActive())
        {
            modalChild_->OnMouseWheel(point, notches);
            return;
        }

        for (auto& child : children_)
        {
            if (
                child != nullptr &&
                child->IsVisible() &&
                child->IsEnabled()
            )
            {
                child->OnMouseWheel(point, notches);
            }
        }
    }

    std::vector<
        std::unique_ptr<UIElement>
    >& Panel::GetChildren() noexcept
    {
        return children_;
    }

    const std::vector<
        std::unique_ptr<UIElement>
    >& Panel::GetChildren() const noexcept
    {
        return children_;
    }
}