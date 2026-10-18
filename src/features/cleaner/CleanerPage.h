#pragma once

#include "features/cleaner/CleanerController.h"
#include "roui/controls/Panel.h"

#include <vector>

namespace real_optimizer::roui
{
    class Button;
    class Checkbox;
    class ConfirmDialog;
    class SurfacePanel;
    class Text;
}

namespace real_optimizer::features::cleaner
{
    // Tela "Limpeza": lista as categorias, mostra o tamanho de
    // cada uma e só remove o que o usuário marcar e confirmar.
    class CleanerPage final
    {
    public:
        CleanerPage() = default;
        ~CleanerPage() = default;

        CleanerPage(const CleanerPage&) = delete;
        CleanerPage& operator=(const CleanerPage&) = delete;

        [[nodiscard]] bool Initialize(
            roui::Panel& pageRoot,
            CleanerController& controller,
            roui::ConfirmDialog& confirmDialog);

        void Layout(const roui::Rect& area);

        // Chamado quando a página passa a ser exibida.
        void OnShown();

    private:
        struct Row
        {
            roui::SurfacePanel* divider = nullptr;
            roui::Checkbox* checkbox = nullptr;
            roui::Text* name = nullptr;
            roui::Text* description = nullptr;
            roui::Text* size = nullptr;
            roui::Text* note = nullptr;
            roui::SurfacePanel* badge = nullptr;
            roui::Text* badgeText = nullptr;

            system::ScanResult scan{};
            bool scanned = false;
            bool selectable = false;
        };

        void RequestScan();
        void RequestClean();
        void RequestAdministratorRestart();

        void HandleScanStarted();
        void HandleCategoryScanned(
            std::size_t index,
            const system::ScanResult& result);
        void HandleScanFinished();
        void HandleCleanStarted();
        void HandleCleanFinished(const CleanSummary& summary);

        void UpdateSummary();
        void UpdateButtons();
        void SetStatus(const std::wstring& text);

        [[nodiscard]] bool IsUnknownSize(
            std::size_t index) const;

        CleanerController* controller_ = nullptr;
        roui::ConfirmDialog* confirmDialog_ = nullptr;

        roui::Text* titleText_ = nullptr;
        roui::Text* subtitleText_ = nullptr;
        roui::Text* statusText_ = nullptr;
        roui::Button* scanButton_ = nullptr;
        roui::Button* adminButton_ = nullptr;

        roui::SurfacePanel* listSurface_ = nullptr;
        std::vector<Row> rows_;

        roui::SurfacePanel* footerDivider_ = nullptr;
        roui::Text* summaryText_ = nullptr;
        roui::Text* footerNote_ = nullptr;
        roui::Button* cleanButton_ = nullptr;

        bool busy_ = false;
        bool everScanned_ = false;
        bool keepResultMessage_ = false;
        std::size_t selectedCount_ = 0;
    };
}
