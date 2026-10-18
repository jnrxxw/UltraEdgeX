#pragma once

#include "roui/controls/Panel.h"
#include "system/SystemAnalyzer.h"
#include "system/SystemInfo.h"

#include <array>
#include <functional>
#include <string>
#include <thread>
#include <vector>

namespace real_optimizer::roui
{
    class Button;
    class Card;
    class DrivePicker;
    class ProgressRing;
    class ScrollPanel;
    class SurfacePanel;
    class Text;
    class UiDispatcher;
}

namespace real_optimizer::features::dashboard
{
    class DashboardPage final
    {
    public:
        DashboardPage() = default;
        ~DashboardPage();

        DashboardPage(
            const DashboardPage&
        ) = delete;

        DashboardPage& operator=(
            const DashboardPage&
        ) = delete;

        // pageRoot: painel onde os elementos da página são criados.
        // openCleaner / openServices / openStartup: chamados quando o
        // usuário pede para abrir a página de Limpeza, Serviços ou
        // Inicialização.
        [[nodiscard]]
        bool Initialize(
            real_optimizer::roui::Panel& pageRoot,
            real_optimizer::roui::UiDispatcher& dispatcher,
            std::function<void()> openCleaner,
            std::function<void()> openServices,
            std::function<void()> openStartup
        );

        // area: região da tela destinada ao conteúdo da página.
        void Layout(
            const real_optimizer::roui::Rect& area
        );

        void RefreshRealtimeStats();

        // Chamado quando a página volta a ser exibida: se já houve
        // análise, refaz para os números não ficarem velhos.
        void OnShown();

        // Fecha menus abertos (ex.: escolha de disco). Chamado ao
        // sair da página.
        void CloseMenus();

        // Retorna true se a tecla foi consumida por um menu aberto.
        [[nodiscard]] bool OnKeyDown(unsigned int virtualKey);

        // Espera a análise em andamento terminar.
        void Shutdown();

    private:
        using Panel = real_optimizer::roui::Panel;

        struct IssueSlot
        {
            real_optimizer::roui::SurfacePanel* box = nullptr;
            real_optimizer::roui::Text* title = nullptr;
            real_optimizer::roui::Text* description = nullptr;

            // Botão ao lado do alerta (só aparece se houver ação).
            real_optimizer::roui::Button* button = nullptr;
            real_optimizer::system::IssueAction action =
                real_optimizer::system::IssueAction::None;
        };

        struct InfoRow
        {
            real_optimizer::roui::SurfacePanel* divider = nullptr;
            real_optimizer::roui::Text* label = nullptr;
            real_optimizer::roui::Text* value = nullptr;

            // Texto completo; o exibido pode ser encurtado com "…"
            // conforme a largura da coluna.
            std::wstring fullValue;
            std::size_t shownChars = 0;
        };

        struct StartupRow
        {
            real_optimizer::roui::SurfacePanel* divider = nullptr;
            real_optimizer::roui::SurfacePanel* icon = nullptr;
            real_optimizer::roui::Text* iconLetter = nullptr;
            real_optimizer::roui::Text* name = nullptr;
            real_optimizer::roui::SurfacePanel* badge = nullptr;
            real_optimizer::roui::Text* badgeText = nullptr;
        };

        bool CreateHeader(Panel& rootPanel);
        bool CreateMetrics(Panel& rootPanel);
        bool CreateHealthSection(Panel& rootPanel);
        bool CreateSystemInformation(Panel& rootPanel);

        // Relê o que muda com o tempo (tempo ligado, plano de energia,
        // monitor) e força o reajuste dos textos no próximo Layout.
        void RefreshSystemInformation();
        bool CreateStartupSection(Panel& rootPanel);

        // (Re)cria o conteúdo do bloco de inicialização a partir da
        // lista atual do Windows. Chamado ao criar e a cada vez que a
        // página volta a ser exibida (o usuário pode ter mudado algo
        // na página Inicialização).
        void BuildStartupContents();
        void RefreshStartupSection();

        void AnalyzeSystem();
        void ApplyAnalysis(
            const real_optimizer::system::AnalysisResult& result);

        void RunIssueAction(
            real_optimizer::system::IssueAction action);

        std::function<void()> openCleaner_;
        std::function<void()> openServices_;
        std::function<void()> openStartup_;
        real_optimizer::roui::UiDispatcher* dispatcher_ = nullptr;
        std::thread worker_;
        bool analyzing_ = false;
        bool hasAnalysis_ = false;
        bool shuttingDown_ = false;
        real_optimizer::roui::Rect area_{};

        // Cabeçalho
        real_optimizer::roui::Text* titleText_ = nullptr;
        real_optimizer::roui::Text* subtitleText_ = nullptr;
        real_optimizer::roui::Button* analyzeButton_ = nullptr;

        // Métricas
        real_optimizer::roui::Card* cpuCard_ = nullptr;
        real_optimizer::roui::Card* memoryCard_ = nullptr;
        real_optimizer::roui::Card* diskCard_ = nullptr;

        // Escolha do disco mostrado no card de disco.
        real_optimizer::roui::DrivePicker* drivePicker_ = nullptr;
        std::wstring selectedDriveRoot_; // vazio = disco do sistema
        std::vector<real_optimizer::system::DriveInfo> drives_;

        void RefreshDriveList();
        void UpdateDiskCard();
        void OpenDriveMenu();

        // Saúde do sistema
        real_optimizer::roui::SurfacePanel* healthSurface_ = nullptr;
        real_optimizer::roui::ProgressRing* healthRing_ = nullptr;
        real_optimizer::roui::Text* healthTitle_ = nullptr;
        real_optimizer::roui::Text* healthText_ = nullptr;
        real_optimizer::roui::Text* analysisStatusText_ = nullptr;
        // Os alertas ficam numa área com rolagem: aparecem 3 por vez
        // e o usuário rola pela roda do mouse para ver os outros.
        // 5 é o máximo que o SystemAnalyzer gera hoje.
        real_optimizer::roui::ScrollPanel* issueScroll_ = nullptr;
        std::array<IssueSlot, 5> issueSlots_{};
        std::size_t visibleIssueCount_ = 0;

        // Informações do sistema
        real_optimizer::roui::SurfacePanel* infoSurface_ = nullptr;
        real_optimizer::roui::Text* infoTitle_ = nullptr;
        std::array<InfoRow, 10> infoRows_{};

        // Inicialização
        real_optimizer::roui::SurfacePanel* startupSurface_ = nullptr;
        real_optimizer::roui::Text* startupTitle_ = nullptr;
        real_optimizer::roui::Text* startupSummary_ = nullptr;
        real_optimizer::roui::Text* startupMore_ = nullptr;
        real_optimizer::roui::Button* manageStartupButton_ = nullptr;
        std::vector<StartupRow> startupRows_;
    };
}
