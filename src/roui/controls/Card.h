#pragma once

#include "roui/core/UIElement.h"
#include "roui/core/VisualTypes.h"

#include <functional>
#include <string>

namespace real_optimizer::roui
{
    // Cartão de métrica: título, valor em destaque,
    // descrição e barra de progresso opcional.
    // Todas as cores vêm do tema atual.
    class Card final : public UIElement
    {
    public:
        Card() = default;
        ~Card() override = default;

        void SetTitle(std::wstring title);
        void SetValue(std::wstring value);
        void SetDescription(std::wstring description);

        // fraction entre 0.0 e 1.0.
        void SetProgress(float fraction) noexcept;

        [[nodiscard]] const std::wstring&
        GetTitle() const noexcept;

        [[nodiscard]] const std::wstring&
        GetValue() const noexcept;

        [[nodiscard]] const std::wstring&
        GetDescription() const noexcept;

        // Seta no canto superior direito (ex.: escolher o disco).
        // Só aparece e só responde ao mouse quando visível.
        void SetSelectorVisible(bool visible) noexcept;

        // Seta virada para cima enquanto o menu estiver aberto.
        void SetSelectorOpen(bool open) noexcept;

        void SetOnSelectorClick(std::function<void()> handler);

        [[nodiscard]] Rect GetSelectorBounds() const noexcept;

        void OnMouseMove(const Point& point) override;
        void OnMouseLeave() override;
        void OnMouseDown(const Point& point) override;
        void OnMouseUp(const Point& point) override;

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

    private:
        std::wstring title_;
        std::wstring value_;
        std::wstring description_;

        float progress_ = 0.0F;

        [[nodiscard]] bool InsideSelector(
            const Point& point) const noexcept;

        std::function<void()> onSelectorClick_;

        bool selectorVisible_ = false;
        bool selectorOpen_ = false;
        bool selectorHover_ = false;
        bool selectorPressed_ = false;
    };
}
