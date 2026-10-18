#pragma once

#include "roui/core/UIElement.h"
#include "roui/core/VisualTypes.h"

#include <string>

namespace real_optimizer::roui
{
    // Anel de progresso (0 a 100) com valor no centro.
    class ProgressRing final : public UIElement
    {
    public:
        ProgressRing() = default;
        ~ProgressRing() override = default;

        // percent < 0 significa "sem valor" (mostra "--").
        void SetValue(float percent) noexcept;
        void SetCaption(std::wstring caption);

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

    private:
        float percent_ = -1.0F;
        std::wstring caption_;
    };
}
