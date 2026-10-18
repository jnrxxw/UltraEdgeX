#pragma once

#include "roui/controls/Panel.h"

namespace real_optimizer::roui
{
    enum class SurfaceRole
    {
        Surface,    // cartão padrão (preenchimento + borda)
        Sidebar,    // barra lateral (borda só à direita)
        Inset,      // superfície rebaixada
        AccentSoft, // fundo suave da cor de destaque
        Accent,     // preenchido com a cor de destaque
        SuccessSoft, // fundo suave de sucesso
        WarningSoft, // fundo suave de aviso
        Divider     // linha divisória
    };

    // Painel que desenha o próprio fundo a partir do tema
    // e depois renderiza os filhos por cima.
    class SurfacePanel final : public Panel
    {
    public:
        SurfacePanel() = default;
        ~SurfacePanel() override = default;

        void SetRole(SurfaceRole role) noexcept;
        void SetCornerRadius(float radius) noexcept;

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

    private:
        SurfaceRole role_ = SurfaceRole::Surface;
        float cornerRadius_ = 16.0F;
    };
}
