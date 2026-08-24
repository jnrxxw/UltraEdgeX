#pragma once

#include "roui/controls/Panel.h"

namespace real_optimizer::roui
{
    class Panel;
    class Card;
    class StackPanel;
}

namespace real_optimizer::features::dashboard
{
    class DashboardPage final
    {
    public:
        DashboardPage() = default;
        ~DashboardPage() = default;

        DashboardPage(const DashboardPage&) = delete;
        DashboardPage& operator=(const DashboardPage&) = delete;

        [[nodiscard]]
        bool Initialize(
            real_optimizer::roui::Panel& rootPanel
        );

        void Resize(
            float width,
            float height
        ) noexcept;

    private:

        bool CreateHeader();

        bool CreateActions();

        bool CreateMetrics();

        bool CreateHistory();

    private:

        real_optimizer::roui::Panel* pagePanel_ = nullptr;

        real_optimizer::roui::StackPanel* headerPanel_ = nullptr;
        real_optimizer::roui::StackPanel* actionPanel_ = nullptr;
        real_optimizer::roui::StackPanel* metricsPanel_ = nullptr;
        real_optimizer::roui::StackPanel* historyPanel_ = nullptr;

        real_optimizer::roui::Card* cpuCard_ = nullptr;
    };
}