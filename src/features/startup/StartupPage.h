#pragma once

#include "features/startup/StartupController.h"
#include "roui/controls/Panel.h"
#include "system/StartupCatalog.h"

#include <string>
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

namespace real_optimizer::features::startup
{
    // Tela "Inicialização": aplicativos que abrem junto com o Windows,
    // com interruptor, explicação, nível de segurança (Seguro/Atenção)
    // e estado atual. Nada é alterado até o usuário confirmar.
    class StartupPage final
    {
    public:
        StartupPage() = default;
        ~StartupPage() = default;

        StartupPage(const StartupPage&) = delete;
        StartupPage& operator=(const StartupPage&) = delete;

        [[nodiscard]] bool Initialize(
            roui::Panel& pageRoot,
            StartupController& controller,
            roui::ConfirmDialog& confirmDialog);

        void Layout(const roui::Rect& area);

        // Chamado quando a página passa a ser exibida.
        void OnShown();

    private:
        // A lista é dinâmica (cada PC tem apps diferentes), então as
        // linhas são recriadas a cada leitura.
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

            system::StartupApp app{};
            system::StartupInfo info{};
        };

        void RebuildRows(std::vector<system::StartupApp> apps);

        void RequestRefresh();
        void RequestApply();
        void MarkRecommended();

        void HandleRefreshStarted();
        void HandleRefreshFinished(std::vector<system::StartupApp> apps);
        void HandleApplyStarted();
        void HandleApplyFinished(const ApplySummary& summary);

        void CollectPending(
            std::vector<std::size_t>& toDisable,
            std::vector<std::size_t>& toEnable) const;

        void UpdateSummary();
        void UpdateButtons();
        void SetStatus(const std::wstring& text);

        StartupController* controller_ = nullptr;
        roui::ConfirmDialog* confirmDialog_ = nullptr;

        roui::Text* titleText_ = nullptr;
        roui::Text* subtitleText_ = nullptr;
        roui::Text* statusText_ = nullptr;
        roui::Button* refreshButton_ = nullptr;
        roui::Button* recommendButton_ = nullptr;

        roui::SurfacePanel* listSurface_ = nullptr;
        roui::ScrollPanel* scroll_ = nullptr;
        roui::Text* emptyText_ = nullptr;
        std::vector<Row> rows_;

        roui::SurfacePanel* footerDivider_ = nullptr;
        roui::Text* summaryText_ = nullptr;
        roui::Text* footerNote_ = nullptr;
        roui::Button* applyButton_ = nullptr;

        roui::Rect area_{};

        bool busy_ = false;
        bool loaded_ = false;
        bool keepResultMessage_ = false;
        std::size_t pendingCount_ = 0;
    };
}
