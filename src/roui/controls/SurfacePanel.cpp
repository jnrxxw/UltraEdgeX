#include "roui/controls/SurfacePanel.h"

#include "roui/core/Theme.h"
#include "roui/rendering/Renderer.h"

namespace real_optimizer::roui
{
    void SurfacePanel::SetRole(SurfaceRole role) noexcept
    {
        role_ = role;
    }

    void SurfacePanel::SetCornerRadius(float radius) noexcept
    {
        cornerRadius_ = radius >= 0.0F ? radius : 0.0F;
    }

    void SurfacePanel::OnRender(
        rendering::Renderer& renderer
    )
    {
        const Palette& palette = Theme::Current();
        const Rect& bounds = GetBounds();

        switch (role_)
        {
        case SurfaceRole::Sidebar:
            renderer.FillRectangle(bounds, palette.sidebar);

            renderer.FillRectangle(
                Rect{bounds.Right() - 1.0F, bounds.y, 1.0F, bounds.height},
                palette.border);
            break;

        case SurfaceRole::Divider:
            renderer.FillRectangle(bounds, palette.divider);
            break;

        case SurfaceRole::Surface:
        case SurfaceRole::Inset:
        {
            renderer.FillRoundedRectangle(
                bounds,
                cornerRadius_,
                role_ == SurfaceRole::Inset
                    ? palette.surfaceInset
                    : palette.surface);

            renderer.DrawRoundedRectangle(
                bounds,
                cornerRadius_,
                1.0F,
                palette.border);
            break;
        }

        case SurfaceRole::AccentSoft:
            renderer.FillRoundedRectangle(
                bounds,
                cornerRadius_,
                palette.accentSoft);
            break;

        case SurfaceRole::Accent:
            renderer.FillRoundedRectangle(
                bounds,
                cornerRadius_,
                palette.accent);
            break;

        case SurfaceRole::SuccessSoft:
            renderer.FillRoundedRectangle(
                bounds,
                cornerRadius_,
                palette.successSoft);
            break;

        case SurfaceRole::WarningSoft:
            renderer.FillRoundedRectangle(
                bounds,
                cornerRadius_,
                palette.warningSoft);
            break;
        }

        Panel::OnRender(renderer);
    }
}
