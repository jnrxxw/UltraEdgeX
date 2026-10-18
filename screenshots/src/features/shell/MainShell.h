#pragma once

#include "features/cleaner/CleanerController.h"
#include "features/cleaner/CleanerPage.h"
#include "features/dashboard/DashboardPage.h"
#include "features/services/ServicesController.h"
#include "features/services/ServicesPage.h"
#include "features/startup/StartupController.h"
#include "features/startup/StartupPage.h"
#include "roui/controls/Panel.h"
#include "roui/window/UiDispatcher.h"

#include <array>
#include <memory>

namespace real_optimizer::roui
{
    class Button;
    class ConfirmDialog;
    class ImageBox;
    class SurfacePanel;
    class Text;
    enum class ThemeMode;
}

namespace real_optimizer::features::shell
{
    // Estrutura fixa da janela: barra lateral, seletor de tema e
    // a página atual (Painel, Limpeza, Inicialização ou Serviços).
    class MainShell final
    {
    public:
        MainShell() = default;
        ~MainShell() = default;

        MainShell(const MainShell&) = delete;
        MainShell& operator=(const MainShell&) = delete;

        [[nodiscard]] bool Initialize(
            roui::Panel& rootPanel,
            roui::UiDispatcher& dispatcher);

        void Resize(float width, float height) noexcept;
        void RefreshRealtimeStats();

        // Teclado: Esc fecha a caixa de confirmação. Retorna true
        // se a tecla foi consumida.
        [[nodiscard]] bool OnKeyDown(unsigned int virtualKey);

        // Espera tarefas em segundo plano terminarem.
        void Shutdown();

    private:
        enum class Page
        {
            Dashboard,
            Cleaner,
            Startup,
            Services
        };

        bool CreateSidebar(roui::Panel& rootPanel);
        bool CreateThemeToggle(roui::Panel& rootPanel);

        void ShowPage(Page page);
        void SetTheme(roui::ThemeMode mode);
        void UpdateThemeButtons();
        void Layout();

        float width_ = 1280.0F;
        float height_ = 950.0F;

        // Barra lateral
        roui::SurfacePanel* sidebar_ = nullptr;
        roui::ImageBox* brandLogo_ = nullptr;
        roui::Text* brandText_ = nullptr;
        std::array<roui::Button*, 5> navButtons_{};
        roui::SurfacePanel* machineChip_ = nullptr;
        roui::Text* machineLabel_ = nullptr;
        roui::Text* machineName_ = nullptr;
        roui::Text* machineSystem_ = nullptr;

        // Seletor de tema
        roui::SurfacePanel* toggleSurface_ = nullptr;
        roui::Button* darkButton_ = nullptr;
        roui::Button* lightButton_ = nullptr;

        // Páginas
        roui::Panel* dashboardRoot_ = nullptr;
        roui::Panel* cleanerRoot_ = nullptr;
        roui::Panel* startupRoot_ = nullptr;
        roui::Panel* servicesRoot_ = nullptr;
        roui::ConfirmDialog* confirmDialog_ = nullptr;

        // O controlador precisa existir enquanto a página existir.
        std::unique_ptr<cleaner::CleanerController> cleanerController_;

        std::unique_ptr<startup::StartupController> startupController_;

        std::unique_ptr<services::ServicesController> servicesController_;

        dashboard::DashboardPage dashboardPage_;
        cleaner::CleanerPage cleanerPage_;
        startup::StartupPage startupPage_;
        services::ServicesPage servicesPage_;
    };
}
