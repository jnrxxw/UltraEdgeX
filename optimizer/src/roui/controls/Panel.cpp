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

        UIElement* childPointer = child.get();

        children_.push_back(
            std::move(child)
        );

        return childPointer;
    }

    void Panel::ClearChildren() noexcept
    {
        children_.clear();
    }

    std::size_t Panel::GetChildCount() const noexcept
    {
        return children_.size();
    }

    void Panel::OnRender(
        rendering::Renderer& renderer
    )
    {
        for (const auto& child : children_)
        {
            if (child != nullptr)
            {
                child->Render(renderer);
            }
        }
    }

    void Panel::OnMouseMove(const Point& point)
    {
        for (auto& child : children_)
        {
            if (child != nullptr)
            {
                child->OnMouseMove(point);
            }
        }
    }

    void Panel::OnMouseLeave()
    {
        for (auto& child : children_)
        {
            if (child != nullptr)
            {
                child->OnMouseLeave();
            }
        }
    }

    void Panel::OnMouseDown(const Point& point)
    {
        for (
            auto iterator = children_.rbegin();
            iterator != children_.rend();
            ++iterator
        )
        {
            if (
                *iterator != nullptr &&
                (*iterator)->ContainsPoint(point)
            )
            {
                (*iterator)->OnMouseDown(point);
                return;
            }
        }
    }

    void Panel::OnMouseUp(const Point& point)
    {
        for (auto& child : children_)
        {
            if (child != nullptr)
            {
                child->OnMouseUp(point);
            }
        }
    }

        std::vector<std::unique_ptr<UIElement>>&
    Panel::GetChildren() noexcept
    {
        return children_;
    }

    const std::vector<std::unique_ptr<UIElement>>&
    Panel::GetChildren() const noexcept    
   {
    return children_;
   }
   
}