#include "roui/layout/StackPanel.h"

#include "roui/rendering/Renderer.h"

namespace real_optimizer::roui
{
    void StackPanel::SetOrientation(
        Orientation orientation
    ) noexcept
    {
        orientation_ = orientation;
    }

    void StackPanel::SetSpacing(
        float spacing
    ) noexcept
    {
        spacing_ = spacing >= 0.0F
            ? spacing
            : 0.0F;
    }

    void StackPanel::SetPadding(
        const Thickness& padding
    ) noexcept
    {
        padding_ = padding;
    }

    Orientation StackPanel::GetOrientation() const noexcept
    {
        return orientation_;
    }

    float StackPanel::GetSpacing() const noexcept
    {
        return spacing_;
    }

    const Thickness& StackPanel::GetPadding() const noexcept
    {
        return padding_;
    }

    void StackPanel::ArrangeChildren()
    {
        auto& children = GetChildren();

        float currentX =
            GetX() + padding_.left;

        float currentY =
            GetY() + padding_.top;

        for (auto& child : children)
        {
            if (!child)
            {
                continue;
            }

            child->SetPosition(
                currentX,
                currentY
            );

            if (orientation_ ==
                Orientation::Vertical)
            {
                currentY +=
                    child->GetHeight() +
                    spacing_;
            }
            else
            {
                currentX +=
                    child->GetWidth() +
                    spacing_;
            }
        }
    }

    void StackPanel::OnRender(
        rendering::Renderer& renderer
    )
    {
        ArrangeChildren();
        Panel::OnRender(renderer);
    }
}