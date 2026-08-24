#pragma once

#include "roui/controls/Panel.h"
#include "roui/core/Geometry.h"
#include "roui/layout/Orientation.h"

namespace real_optimizer::roui
{
    class StackPanel final : public Panel
    {
    public:
        StackPanel() = default;
        ~StackPanel() override = default;

        void SetOrientation(
            Orientation orientation) noexcept;

        [[nodiscard]]
        Orientation GetOrientation() const noexcept;

        void SetSpacing(
            float spacing) noexcept;

        [[nodiscard]]
        float GetSpacing() const noexcept;

        void SetPadding(
            const Thickness &padding) noexcept;

        [[nodiscard]]
        const Thickness &GetPadding() const noexcept;

        void SetAutoSize(
            bool autoWidth,
            bool autoHeight) noexcept;

    protected:
        void OnRender(
            rendering::Renderer &renderer) override;

    private:
        void ArrangeChildren();

        bool autoWidth_ = false;
        bool autoHeight_ = false;

        Orientation orientation_ =
            Orientation::Vertical;

        float spacing_ = 10.0F;

        Thickness padding_{};
    };
}