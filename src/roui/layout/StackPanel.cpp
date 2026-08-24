#include "roui/layout/StackPanel.h"

#include "roui/rendering/Renderer.h"

namespace real_optimizer::roui
{
    void StackPanel::SetOrientation(
        Orientation orientation) noexcept
    {
        orientation_ = orientation;
    }

    void StackPanel::SetSpacing(
        float spacing) noexcept
    {
        spacing_ = spacing >= 0.0F
                       ? spacing
                       : 0.0F;
    }

    void StackPanel::SetPadding(
        const Thickness &padding) noexcept
    {
        padding_ = padding;
    }

    Orientation StackPanel::GetOrientation()
        const noexcept
    {
        return orientation_;
    }

    float StackPanel::GetSpacing()
        const noexcept
    {
        return spacing_;
    }

    const Thickness &StackPanel::GetPadding()
        const noexcept
    {
        return padding_;
    }

    void StackPanel::SetAutoSize(
        bool autoWidth,
        bool autoHeight) noexcept
    {
        autoWidth_ = autoWidth;
        autoHeight_ = autoHeight;
    }

    void StackPanel::ArrangeChildren()
    {
        auto &children =
            GetChildren();

        float currentX =
            GetX() + padding_.left;

        float currentY =
            GetY() + padding_.top;

        float contentWidth = 0.0F;
        float contentHeight = 0.0F;

        std::size_t visibleCount = 0;

        for (auto &child : children)
        {
            if (
                !child ||
                !child->IsVisible())
            {
                continue;
            }

            child->SetPosition(
                currentX,
                currentY);

            if (
                orientation_ ==
                Orientation::Vertical)
            {
                currentY +=
                    child->GetHeight() +
                    spacing_;

                contentHeight +=
                    child->GetHeight();

                if (child->GetWidth() > contentWidth)
                {
                    contentWidth =
                        child->GetWidth();
                }
            }
            else
            {
                currentX +=
                    child->GetWidth() +
                    spacing_;

                contentWidth +=
                    child->GetWidth();

                if (child->GetHeight() > contentHeight)
                {
                    contentHeight =
                        child->GetHeight();
                }
            }

            ++visibleCount;
        }

        if (visibleCount > 1)
        {
            const float totalSpacing =
                spacing_ *
                static_cast<float>(
                    visibleCount - 1);

            if (
                orientation_ ==
                Orientation::Vertical)
            {
                contentHeight += totalSpacing;
            }
            else
            {
                contentWidth += totalSpacing;
            }
        }

        contentWidth +=
            padding_.left +
            padding_.right;

        contentHeight +=
            padding_.top +
            padding_.bottom;

        float newWidth =
            GetWidth();

        float newHeight =
            GetHeight();

        if (autoWidth_)
        {
            newWidth = contentWidth;
        }

        if (autoHeight_)
        {
            newHeight = contentHeight;
        }

        SetSize(
            newWidth,
            newHeight);
    }

    void StackPanel::OnRender(
        rendering::Renderer &renderer)
    {
        ArrangeChildren();

        Panel::OnRender(renderer);
    }
}