#pragma once

#include "roui/controls/Panel.h"

#include <array>
#include <functional>
#include <string>
#include <vector>

namespace real_optimizer::roui
{
    class Button;
    class SurfacePanel;
    class Text;

    struct ConfirmDialogItem
    {
        std::wstring label;
        std::wstring value;

        // Valor em cor discreta (ex.: "tamanho desconhecido").
        bool valueMuted = false;
    };

    struct ConfirmDialogNotice
    {
        std::wstring text;

        // true = aviso (amarelo); false = informação.
        bool warning = false;
    };

    struct ConfirmDialogContent
    {
        std::wstring title;
        std::wstring message;

        std::vector<ConfirmDialogItem> items;

        // Linha de total (opcional: deixe totalLabel vazio).
        std::wstring totalLabel;
        std::wstring totalValue;

        std::vector<ConfirmDialogNotice> notices;
        std::wstring footnote;

        std::wstring confirmText = L"Confirmar";
        std::wstring cancelText = L"Cancelar";
    };

    // Caixa de confirmação no estilo do app: escurece a janela,
    // mostra um cartão no centro e bloqueia o resto da tela
    // enquanto estiver aberta. Esc cancela.
    class ConfirmDialog final : public Panel
    {
    public:
        static constexpr std::size_t MaxItems = 16;
        static constexpr std::size_t MaxNotices = 3;

        ConfirmDialog() = default;
        ~ConfirmDialog() override = default;

        // Cria os elementos internos. Chamar uma vez, depois de
        // adicionar ao painel pai.
        [[nodiscard]] bool Initialize();

        void Show(
            ConfirmDialogContent content,
            std::function<void()> onConfirm);

        void Hide();

        [[nodiscard]] bool IsOpen() const noexcept;

        // Tamanho da janela (a caixa cobre a janela inteira).
        void Layout(float windowWidth, float windowHeight);

        // Retorna true se a tecla foi consumida.
        [[nodiscard]] bool OnKeyDown(unsigned int virtualKey);

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

    private:
        struct ItemRow
        {
            Text* label = nullptr;
            Text* value = nullptr;
        };

        struct NoticeRow
        {
            SurfacePanel* box = nullptr;
            Text* text = nullptr;
        };

        SurfacePanel* card_ = nullptr;
        Text* titleText_ = nullptr;
        Text* messageText_ = nullptr;

        SurfacePanel* listSurface_ = nullptr;
        std::array<ItemRow, MaxItems> itemRows_{};
        SurfacePanel* totalDivider_ = nullptr;
        Text* totalLabel_ = nullptr;
        Text* totalValue_ = nullptr;

        std::array<NoticeRow, MaxNotices> noticeRows_{};
        Text* footnoteText_ = nullptr;

        Button* cancelButton_ = nullptr;
        Button* confirmButton_ = nullptr;

        ConfirmDialogContent content_;
        std::function<void()> onConfirm_;

        float windowWidth_ = 1280.0F;
        float windowHeight_ = 950.0F;
    };
}
