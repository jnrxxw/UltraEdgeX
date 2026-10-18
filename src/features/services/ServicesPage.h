#pragma once

#include "features/services/ServicesController.h"
#include "roui/controls/Panel.h"

#include <vector>

namespace real_optimizer::roui
{
    class Button;
    class Checkbox;
    class ConfirmDialog;
    class ScrollPanel;
    class SurfacePanel;
    class Text;
}

namespace real_optimizer::features::services
{
    // Tela "Serviços": lista fixa de serviços do Windows com
    // interruptor, descrição, nível de segurança e estado atual.
    // Nada é alterado até o usuário confirmar.
    class ServicesPage final
    {
    public:
        ServicesPage() = default;
        ~ServicesPage() = default;

        ServicesPage(const ServicesPage&) = delete;
        ServicesPage& operator=(const ServicesPage&) = delete;

        [[nodiscard]] bool Initialize(
            roui::Panel& pageRoot,
            ServicesController& controller,
            roui::ConfirmDialog& confirmDialog);

        void Layout(const roui::Rect& area);

        // Chamado quando a página passa a ser exibida.
        void OnShown();

    private:
        struct Row
        {
            roui::SurfacePanel* divider = nullptr;
            roui::Checkbox* toggle = nullptr;
            roui::Text* name = nullptr;
            roui::Text* description = nullptr;
            roui::SurfacePanel* badge = nullptr;
            roui::Text* badgeText = nullptr;
            roui::Text* state = nullptr;
            roui::Text* note = nullptr;

            system::ServiceState service{};
            bool present = false;
        };

        void RequestRefresh();
        void RequestApply();
        void RequestRestore();
        void MarkRecommended();

        void HandleRefreshStarted();
        void HandleServiceLoaded(
            std::size_t index,
            const system::ServiceState& state);
        void HandleRefreshFinished();
        void HandleApplyStarted();
        void HandleApplyFinished(const ApplySummary& summary);

        void CollectPending(
            std::vector<std::size_t>& toDisable,
            std::vector<std::size_t>& toEnable) const;

        void UpdateSummary();
        void UpdateButtons();
        void SetStatus(const std::wstring& text);

        ServicesController* controller_ = nullptr;
        roui::ConfirmDialog* confirmDialog_ = nullptr;

        roui::Text* titleText_ = nullptr;
        roui::Text* subtitleText_ = nullptr;
        roui::Text* statusText_ = nullptr;
        roui::Button* refreshButton_ = nullptr;
        roui::Button* recommendButton_ = nullptr;

        roui::SurfacePanel* listSurface_ = nullptr;
        roui::ScrollPanel* scroll_ = nullptr;
        std::vector<Row> rows_;

        roui::SurfacePanel* footerDivider_ = nullptr;
        roui::Text* summaryText_ = nullptr;
        roui::Text* footerNote_ = nullptr;
        roui::Button* restoreButton_ = nullptr;
        roui::Button* applyButton_ = nullptr;

        roui::Rect area_{};

        bool busy_ = false;
        bool keepResultMessage_ = false;
        bool hasBackup_ = false;
        std::size_t pendingCount_ = 0;
    };
}
