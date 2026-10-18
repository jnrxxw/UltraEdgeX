#pragma once

#include "roui/controls/Panel.h"

#include <functional>

namespace real_optimizer::roui
{
    // Painel com rolagem vertical pela roda do mouse. Recorta o
    // desenho à própria área. Os filhos são posicionados pela
    // página usando GetScrollOffset(): y = topo - offset.
    class ScrollPanel final : public Panel
    {
    public:
        using ScrollHandler = std::function<void()>;

        ScrollPanel() = default;
        ~ScrollPanel() override = default;

        // Altura total do conteúdo (todas as linhas).
        void SetContentHeight(float height) noexcept;

        [[nodiscard]] float GetScrollOffset() const noexcept;
        void SetScrollOffset(float offset) noexcept;

        // Chamado quando o offset muda por causa da roda.
        void SetOnScroll(ScrollHandler handler);

        void OnMouseMove(const Point& point) override;
        void OnMouseDown(const Point& point) override;
        void OnMouseWheel(const Point& point, float notches) override;

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

    private:
        [[nodiscard]] float MaxOffset() const noexcept;

        float contentHeight_ = 0.0F;
        float offset_ = 0.0F;

        ScrollHandler onScroll_;
    };
}
