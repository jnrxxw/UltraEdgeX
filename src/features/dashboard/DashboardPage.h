#pragma once

#include "roui/controls/Panel.h"

namespace real_optimizer::roui
{
    class Button;
    class Card;
    class StackPanel;
    class Text;
}

namespace real_optimizer::features::dashboard
{
    class DashboardPage final
    {
    public:
        DashboardPage() = default;
        ~DashboardPage() = default;

        DashboardPage(
            const DashboardPage&
        ) = delete;

        DashboardPage& operator=(
            const DashboardPage&
        ) = delete;

        [[nodiscard]]
        bool Initialize(
            real_optimizer::roui::Panel& rootPanel
        );

        void Resize(
            float width,
            float height
        ) noexcept;

        void RefreshRealtimeStats();

    private:
        bool CreateHeader();
        bool CreateMetrics();
        bool CreateAnalysisSection();
        bool CreateStartupSection();
        bool CreateSystemInformation();
        
        

        void AnalyzeSystem();
        void CleanTemporaryFiles();
        void OpenStartupSettings();

        real_optimizer::roui::Panel*
            pagePanel_ = nullptr;

        real_optimizer::roui::StackPanel*
            headerPanel_ = nullptr;

        real_optimizer::roui::StackPanel*
            metricsPanel_ = nullptr;

        real_optimizer::roui::StackPanel*
            analysisPanel_ = nullptr;

        // Startup
        real_optimizer::roui::StackPanel*
            startupPanel_ = nullptr;

        real_optimizer::roui::StackPanel*
        contentRow_ = nullptr;

        real_optimizer::roui::Card*
            cpuCard_ = nullptr;

        real_optimizer::roui::Card*
            memoryCard_ = nullptr;

        real_optimizer::roui::Card*
            diskCard_ = nullptr;

        real_optimizer::roui::Text*
            healthText_ = nullptr;

        real_optimizer::roui::Text*
            analysisStatusText_ = nullptr;

        real_optimizer::roui::Text*
            issueTitle1_ = nullptr;

        real_optimizer::roui::Text*
            issueDescription1_ = nullptr;

        real_optimizer::roui::Text*
            issueTitle2_ = nullptr;

        real_optimizer::roui::Text*
            issueDescription2_ = nullptr;

        real_optimizer::roui::Text*
            issueTitle3_ = nullptr;

        real_optimizer::roui::Text*
            issueDescription3_ = nullptr;

        real_optimizer::roui::Button*
            analyzeButton_ = nullptr;

        real_optimizer::roui::Button*
            cleanTemporaryFilesButton_ = nullptr;
        
        real_optimizer::roui::Button*
            manageStartupButton_ = nullptr;
    };
}