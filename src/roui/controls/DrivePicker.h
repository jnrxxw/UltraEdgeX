#pragma once

#include "roui/core/UIElement.h"
#include "roui/core/VisualTypes.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace real_optimizer::roui
{
    struct DrivePickerItem
    {
        // Ex.: "C: Windows (Sistema)".
        std::wstring title;

        // Ex.: "SSD · 238 GB livres de 476 GB".
        std::wstring detail;

        // 0.0 a 1.0 (barra de uso).
        float usedFraction = 0.0F;
    };

    // Menu pequeno para escolher um disco. Aparece ancorado abaixo de
    // um ponto (a seta do card) e fecha ao escolher, ao clicar fora ou
    // com Esc. Deve ser o último filho do painel e ser definido como
    // filho modal dele, para receber todo o mouse enquanto aberto.
    class DrivePicker final : public UIElement
    {
    public:
        using SelectHandler = std::function<void(std::size_t)>;

        static constexpr std::size_t MaxItems = 8;

        DrivePicker() = default;
        ~DrivePicker() override = default;

        // anchor: retângulo da seta; o menu abre logo abaixo dele,
        // alinhado à direita.
        void Show(
            std::vector<DrivePickerItem> items,
            std::size_t selectedIndex,
            const Rect& anchor,
            SelectHandler onSelect,
            std::function<void()> onClose);

        void Hide();

        [[nodiscard]] bool IsOpen() const noexcept;

        // Retorna true se a tecla foi consumida (sempre, quando aberto).
        [[nodiscard]] bool OnKeyDown(unsigned int virtualKey);

        void OnMouseMove(const Point& point) override;
        void OnMouseLeave() override;
        void OnMouseDown(const Point& point) override;
        void OnMouseUp(const Point& point) override;

    protected:
        void OnRender(rendering::Renderer& renderer) override;

    private:
        static constexpr std::size_t None = static_cast<std::size_t>(-1);

        [[nodiscard]] Rect MenuRect() const noexcept;
        [[nodiscard]] Rect RowRect(std::size_t index) const noexcept;
        [[nodiscard]] std::size_t IndexAt(const Point& point) const noexcept;
        [[nodiscard]] bool InsideMenu(const Point& point) const noexcept;

        void Choose(std::size_t index);

        std::vector<DrivePickerItem> items_;
        std::size_t selected_ = None;
        std::size_t hover_ = None;
        std::size_t pressed_ = None;

        Rect anchor_{};

        SelectHandler onSelect_;
        std::function<void()> onClose_;
    };
}
