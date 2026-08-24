#include "features/dashboard/DashboardPage.h"
#include <Windows.h>
#include <shellapi.h>
#include "roui/controls/Button.h"
#include "roui/controls/Card.h"
#include "roui/controls/Text.h"
#include "roui/layout/StackPanel.h"

#include "system/SystemAnalyzer.h"
#include "system/SystemInfo.h"
#include "system/TemporaryFilesCleaner.h"
#include "system/StartupManager.h"
#include <vector>
#include <memory>
#include <string>
#include <utility>

using namespace real_optimizer::roui;

namespace
{
    constexpr std::uint64_t Gigabyte =
        1024ULL * 1024ULL * 1024ULL;

    std::wstring FormatGigabytes(
        std::uint64_t bytes)
    {
        const double gigabytes =
            static_cast<double>(bytes) /
            static_cast<double>(Gigabyte);

        const auto rounded =
            static_cast<std::uint64_t>(
                gigabytes + 0.5);

        return std::to_wstring(rounded) +
               L" GB";
    }
}

namespace real_optimizer::features::dashboard
{
    bool DashboardPage::Initialize(
        Panel &rootPanel)
    {
        auto page =
            std::make_unique<StackPanel>();

        page->SetBounds({40.0F,
                         40.0F,
                         1100.0F,
                         650.0F});

        page->SetOrientation(
            Orientation::Vertical);

        page->SetSpacing(24.0F);

        pagePanel_ = page.get();

        rootPanel.AddChild(
            std::move(page));

        if (!CreateHeader())
        {
            return false;
        }

        if (!CreateMetrics())
        {
            return false;
        }

        //
        // CONTEÚDO PRINCIPAL
        //

        auto contentRow =
            std::make_unique<StackPanel>();

        contentRow->SetSize(
            940.0F,
            360.0F);

        contentRow->SetOrientation(
            Orientation::Horizontal);

        contentRow->SetAutoSize(
            false,
            true);

        contentRow->SetSpacing(
            40.0F);

        contentRow_ = contentRow.get();

        pagePanel_->AddChild(
            std::move(contentRow));

        //
        // SAÚDE DO SISTEMA
        //

        if (!CreateAnalysisSection())
        {
            return false;
        }

        //
        // INFORMAÇÕES DO SISTEMA
        //

        if (!CreateSystemInformation())
        {
            return false;
        }

        //
        // APLICATIVOS DE INICIALIZAÇÃO
        //

        if (!CreateStartupSection())
        {
            return false;
        }

        return true;
    }

    bool DashboardPage::CreateHeader()
    {
        if (pagePanel_ == nullptr)
        {
            return false;
        }

        auto header =
            std::make_unique<StackPanel>();

        header->SetSize(
            900.0F,
            76.0F);

        header->SetOrientation(
            Orientation::Vertical);

        header->SetSpacing(6.0F);

        auto title =
            std::make_unique<Text>();

        title->SetText(
            L"UltraEdgeX");

        title->SetSize(
            900.0F,
            42.0F);

        title->SetFontSize(32.0F);

        title->SetFontWeight(
            FontWeight::Bold);

        title->SetColor(
            Color::FromRgb(
                7,
                242,
                242));

        header->AddChild(
            std::move(title));

        auto subtitle =
            std::make_unique<Text>();

        subtitle->SetText(
            L"Performance & Optimization");

        subtitle->SetSize(
            900.0F,
            26.0F);

        subtitle->SetFontSize(15.0F);

        subtitle->SetColor(
            Color::FromRgb(
                190,
                190,
                190));

        header->AddChild(
            std::move(subtitle));

        headerPanel_ = header.get();

        pagePanel_->AddChild(
            std::move(header));

        return true;
    }

    bool DashboardPage::CreateMetrics()
    {
        if (pagePanel_ == nullptr)
        {
            return false;
        }

        const auto memory =
            system::SystemInfo::GetMemoryInfo();

        const auto disk =
            system::SystemInfo::GetSystemDiskInfo();

        // Primeira amostra necessária
        // para calcular CPU depois.
        static_cast<void>(
            system::SystemInfo::GetCpuInfo());

        auto metrics =
            std::make_unique<StackPanel>();

        metrics->SetSize(
            1000.0F,
            170.0F);

        metrics->SetOrientation(
            Orientation::Horizontal);

        metrics->SetSpacing(20.0F);

        //
        // CPU
        //

        auto cpuCard =
            std::make_unique<Card>();

        cpuCard->SetTitle(
            L"PROCESSADOR");

        cpuCard->SetValue(
            L"--%");

        cpuCard->SetDescription(
            L"Uso da CPU");

        cpuCard->SetSize(
            300.0F,
            150.0F);

        cpuCard_ = cpuCard.get();

        metrics->AddChild(
            std::move(cpuCard));

        //
        // MEMORY
        //

        auto memoryCard =
            std::make_unique<Card>();

        memoryCard->SetTitle(
            L"MEMORIA");

        memoryCard->SetValue(
            FormatGigabytes(
                memory.totalBytes));

        memoryCard->SetDescription(
            std::to_wstring(
                memory.usagePercent) +
            L"% em uso");

        memoryCard->SetSize(
            300.0F,
            150.0F);

        memoryCard_ =
            memoryCard.get();

        metrics->AddChild(
            std::move(memoryCard));

        //
        // DISK
        //

        auto diskCard =
            std::make_unique<Card>();

        diskCard->SetTitle(
            L"DISCO DO SISTEMA");

        diskCard->SetValue(
            FormatGigabytes(
                disk.totalBytes));

        diskCard->SetDescription(
            FormatGigabytes(
                disk.freeBytes) +
            L" disponíveis");

        diskCard->SetSize(
            300.0F,
            150.0F);

        diskCard_ = diskCard.get();

        metrics->AddChild(
            std::move(diskCard));

        metricsPanel_ = metrics.get();

        pagePanel_->AddChild(
            std::move(metrics));

        return true;
    }

    bool DashboardPage::CreateAnalysisSection()
    {
        if (contentRow_ == nullptr)
        {
            return false;
        }
        auto analysis =
            std::make_unique<StackPanel>();

        analysis->SetSize(
            450.0F,
            330.0F);

        analysis->SetOrientation(
            Orientation::Vertical);

        analysis->SetAutoSize(
            false,
            true);

        analysis->SetSpacing(10.0F);

        auto sectionTitle =
            std::make_unique<Text>();

        sectionTitle->SetText(
            L"SAUDE DO SISTEMA");

        sectionTitle->SetSize(
            900.0F,
            30.0F);

        sectionTitle->SetFontSize(
            18.0F);

        sectionTitle->SetFontWeight(
            FontWeight::SemiBold);

        sectionTitle->SetColor(
            Color::FromRgb(
                220,
                220,
                220));

        analysis->AddChild(
            std::move(sectionTitle));

        //
        // HEALTH
        //

        auto healthText =
            std::make_unique<Text>();

        healthText->SetText(
            L"Ainda não analisado");

        healthText->SetSize(
            900.0F,
            30.0F);

        healthText->SetFontSize(
            20.0F);

        healthText->SetColor(
            Color::FromRgb(
                7,
                242,
                242));

        healthText_ =
            healthText.get();

        analysis->AddChild(
            std::move(healthText));

        //
        // STATUS
        //

        auto statusText =
            std::make_unique<Text>();

        statusText->SetText(
            L"Pronto para analisar seu sistema.");

        statusText->SetSize(
            900.0F,
            26.0F);

        statusText->SetFontSize(
            14.0F);

        statusText->SetColor(
            Color::FromRgb(
                170,
                170,
                170));

        analysisStatusText_ =
            statusText.get();

        analysis->AddChild(
            std::move(statusText));

        auto issueTitle1 =
            std::make_unique<Text>();

        issueTitle1->SetText(L"");

        issueTitle1->SetSize(
            900.0F,
            24.0F);

        issueTitle1->SetFontSize(15.0F);

        issueTitle1->SetFontWeight(
            FontWeight::SemiBold);

        issueTitle1->SetColor(
            Color::FromRgb(
                7,
                242,
                242));

        issueTitle1_ = issueTitle1.get();

        issueTitle1_->SetVisible(false);

        analysis->AddChild(
            std::move(issueTitle1));

        auto issueDescription1 =
            std::make_unique<Text>();

        issueDescription1->SetText(L"");

        issueDescription1->SetSize(
            900.0F,
            22.0F);

        issueDescription1->SetFontSize(13.0F);

        issueDescription1->SetColor(
            Color::FromRgb(
                170,
                170,
                170));

        issueDescription1_ =
            issueDescription1.get();

        issueDescription1_->SetVisible(false);

        analysis->AddChild(
            std::move(issueDescription1));

        //
        // ISSUE 2
        //

        auto issueTitle2 =
            std::make_unique<Text>();

        issueTitle2->SetText(L"");

        issueTitle2->SetSize(
            900.0F,
            24.0F);

        issueTitle2->SetFontSize(15.0F);

        issueTitle2->SetFontWeight(
            FontWeight::SemiBold);

        issueTitle2->SetColor(
            Color::FromRgb(
                7,
                242,
                242));

        issueTitle2_ = issueTitle2.get();
        issueTitle2_->SetVisible(false);

        analysis->AddChild(
            std::move(issueTitle2));

        auto issueDescription2 =
            std::make_unique<Text>();

        issueDescription2->SetText(L"");

        issueDescription2->SetSize(
            900.0F,
            22.0F);

        issueDescription2->SetFontSize(13.0F);

        issueDescription2->SetColor(
            Color::FromRgb(
                170,
                170,
                170));

        issueDescription2_ = issueDescription2.get();
        issueDescription2_->SetVisible(false);

        analysis->AddChild(
            std::move(issueDescription2));

        //
        // ISSUE 3
        //

        auto issueTitle3 =
            std::make_unique<Text>();

        issueTitle3->SetText(L"");

        issueTitle3->SetSize(
            900.0F,
            24.0F);

        issueTitle3->SetFontSize(15.0F);

        issueTitle3->SetFontWeight(
            FontWeight::SemiBold);

        issueTitle3->SetColor(
            Color::FromRgb(
                7,
                242,
                242));

        issueTitle3_ = issueTitle3.get();
        issueTitle3_->SetVisible(false);

        analysis->AddChild(
            std::move(issueTitle3));

        auto issueDescription3 =
            std::make_unique<Text>();

        issueDescription3->SetText(L"");

        issueDescription3->SetSize(
            900.0F,
            22.0F);

        issueDescription3->SetFontSize(13.0F);

        issueDescription3->SetColor(
            Color::FromRgb(
                170,
                170,
                170));

        issueDescription3_ = issueDescription3.get();
        issueDescription3_->SetVisible(false);

        analysis->AddChild(
            std::move(issueDescription3));

        //
        // CLEAN TEMPORARY FILES
        //

        auto cleanButton =
            std::make_unique<Button>();

        cleanButton->SetText(
            L"Limpar arquivos temporários");

        cleanButton->SetSize(
            220.0F,
            48.0F);

        cleanButton->SetOnClick(
            [this]()
            {
                CleanTemporaryFiles();
            });

        cleanTemporaryFilesButton_ =
            cleanButton.get();

        cleanTemporaryFilesButton_->SetVisible(false);
        cleanTemporaryFilesButton_->SetEnabled(false);

        analysis->AddChild(
            std::move(cleanButton));

        //
        // BUTTON
        //

        auto analyzeButton =
            std::make_unique<Button>();

        analyzeButton->SetText(
            L"Analisar sistema");

        analyzeButton->SetSize(
            220.0F,
            48.0F);

        analyzeButton->SetOnClick(
            [this]()
            {
                AnalyzeSystem();
            });

        analyzeButton_ =
            analyzeButton.get();

        analysis->AddChild(
            std::move(analyzeButton));

        analysisPanel_ =
            analysis.get();

        contentRow_->AddChild(
            std::move(analysis));

        return true;
    }

    void DashboardPage::AnalyzeSystem()
    {
        if (
            healthText_ == nullptr ||
            analysisStatusText_ == nullptr ||
            analyzeButton_ == nullptr)
        {
            return;
        }

        const auto result =
            system::SystemAnalyzer::Analyze();

        //
        // VERIFICA SE EXISTEM ARQUIVOS TEMPORÁRIOS
        //

        bool hasTemporaryFilesIssue = false;

        for (const auto &issue : result.issues)
        {
            if (issue.title == L"Arquivos temporários")
            {
                hasTemporaryFilesIssue = true;
                break;
            }
        }

        //
        // BOTÃO DE LIMPEZA
        //

        if (cleanTemporaryFilesButton_ != nullptr)
        {
            cleanTemporaryFilesButton_->SetVisible(
                hasTemporaryFilesIssue);

            cleanTemporaryFilesButton_->SetEnabled(
                hasTemporaryFilesIssue);
        }

        //
        // HEALTH SCORE
        //

        std::wstring healthStatus;

        if (result.healthScore >= 90)
        {
            healthStatus = L"Excelente";
        }
        else if (result.healthScore >= 75)
        {
            healthStatus = L"Bom";
        }
        else if (result.healthScore >= 50)
        {
            healthStatus = L"Atenção necessária";
        }
        else
        {
            healthStatus = L"Estado crítico";
        }

        healthText_->SetText(
            std::to_wstring(result.healthScore) +
            L" / 100  •  " +
            healthStatus);

        const std::size_t issueCount =
            result.GetIssueCount();

        //
        // RESUMO DA ANÁLISE
        //

        if (issueCount == 0)
        {
            analysisStatusText_->SetText(
                L"Nenhum problema imediato de desempenho detectado.");
        }
        else if (issueCount == 1)
        {
            analysisStatusText_->SetText(
                L"1 oportunidade de otimização encontrada.");
        }
        else
        {
            analysisStatusText_->SetText(
                std::to_wstring(issueCount) +
                L" oportunidades de otimização encontradas.");
        }

        //
        // LIMPA E ESCONDE RESULTADOS ANTIGOS
        //

        if (issueTitle1_ != nullptr)
        {
            issueTitle1_->SetText(L"");
            issueTitle1_->SetVisible(false);
        }

        if (issueDescription1_ != nullptr)
        {
            issueDescription1_->SetText(L"");
            issueDescription1_->SetVisible(false);
        }

        if (issueTitle2_ != nullptr)
        {
            issueTitle2_->SetText(L"");
            issueTitle2_->SetVisible(false);
        }

        if (issueDescription2_ != nullptr)
        {
            issueDescription2_->SetText(L"");
            issueDescription2_->SetVisible(false);
        }

        if (issueTitle3_ != nullptr)
        {
            issueTitle3_->SetText(L"");
            issueTitle3_->SetVisible(false);
        }

        if (issueDescription3_ != nullptr)
        {
            issueDescription3_->SetText(L"");
            issueDescription3_->SetVisible(false);
        }

        //
        // ISSUE 1
        //

        if (
            issueCount >= 1 &&
            issueTitle1_ != nullptr &&
            issueDescription1_ != nullptr)
        {
            issueTitle1_->SetText(
                result.issues[0].title);

            issueDescription1_->SetText(
                result.issues[0].description);

            issueTitle1_->SetVisible(true);
            issueDescription1_->SetVisible(true);
        }

        //
        // ISSUE 2
        //

        if (
            issueCount >= 2 &&
            issueTitle2_ != nullptr &&
            issueDescription2_ != nullptr)
        {
            issueTitle2_->SetText(
                result.issues[1].title);

            issueDescription2_->SetText(
                result.issues[1].description);

            issueTitle2_->SetVisible(true);
            issueDescription2_->SetVisible(true);
        }

        //
        // ISSUE 3
        //

        if (
            issueCount >= 3 &&
            issueTitle3_ != nullptr &&
            issueDescription3_ != nullptr)
        {
            issueTitle3_->SetText(
                result.issues[2].title);

            issueDescription3_->SetText(
                result.issues[2].description);

            issueTitle3_->SetVisible(true);
            issueDescription3_->SetVisible(true);
        }

        //
        // ATUALIZA BOTÃO
        //

        analyzeButton_->SetText(
            L"Analisar Novamente");
    }

    void DashboardPage::CleanTemporaryFiles()
    {
        if (
            healthText_ == nullptr ||
            analysisStatusText_ == nullptr)
        {
            return;
        }

        const auto cleanup =
            system::TemporaryFilesCleaner::Clean();

        if (!cleanup.valid)
        {
            analysisStatusText_->SetText(
                L"Não foi possível realizar a limpeza.");

            return;
        }

        constexpr std::uint64_t Megabyte =
            1024ULL * 1024ULL;

        const std::uint64_t removedMb =
            cleanup.removedBytes / Megabyte;

        const std::uint64_t failedMb =
            cleanup.failedBytes / Megabyte;

        std::wstring message;

        if (cleanup.removedBytes > 0)
        {
            message =
                std::to_wstring(removedMb) +
                L" MB removidos com sucesso.";

            if (cleanup.failedFiles > 0)
            {
                message +=
                    L"\n" +
                    std::to_wstring(failedMb) +
                    L" MB não puderam ser removidos (arquivos em uso).";
            }
        }
        else if (cleanup.failedFiles > 0)
        {
            message =
                L"Alguns arquivos estão em uso.\n" +
                std::to_wstring(failedMb) +
                L" MB não puderam ser removidos.";
        }
        else
        {
            message =
                L"Não há arquivos temporários para remover.";
        }

        //
        // Atualiza a análise após a limpeza.
        //

        AnalyzeSystem();

        //
        // Exibe o resultado da limpeza.
        //

        analysisStatusText_->SetText(
            message);
    }

    bool DashboardPage::CreateStartupSection()
    {
        if (pagePanel_ == nullptr)
        {
            return false;
        }

        //
        // Obtém todos os aplicativos registrados
        // para inicialização.
        //
        const auto startupApps =
            system::StartupManager::GetStartupApps();

        //
        // Mantém somente os aplicativos
        // realmente habilitados no Windows.
        //
        std::vector<const system::StartupApp *> enabledApps;

        for (const auto &app : startupApps)
        {
            if (app.enabled)
            {
                enabledApps.push_back(&app);
            }
        }

        auto startup =
            std::make_unique<StackPanel>();

        startup->SetSize(
            940.0F,
            300.0F);

        startup->SetOrientation(
            Orientation::Vertical);

        startup->SetSpacing(8.0F);

        //
        // SECTION TITLE
        //

        auto title =
            std::make_unique<Text>();

        title->SetText(
            L"APLICATIVOS DE INICIALIZAÇÃO");

        title->SetSize(
            900.0F,
            30.0F);

        title->SetFontSize(
            18.0F);

        title->SetFontWeight(
            FontWeight::SemiBold);

        title->SetColor(
            Color::FromRgb(
                220,
                220,
                220));

        startup->AddChild(
            std::move(title));

        //
        // SUMMARY
        //

        auto summary =
            std::make_unique<Text>();

        const std::size_t enabledCount =
            enabledApps.size();

        if (enabledCount == 0)
        {
            summary->SetText(
                L"Nenhum aplicativo habilitado para iniciar com o Windows.");
        }
        else if (enabledCount == 1)
        {
            summary->SetText(
                L"1 aplicativo habilitado para iniciar com o Windows.");
        }
        else
        {
            summary->SetText(
                std::to_wstring(enabledCount) +
                L" aplicativos habilitados para iniciar com o Windows.");
        }

        summary->SetSize(
            900.0F,
            26.0F);

        summary->SetFontSize(
            14.0F);

        summary->SetColor(
            Color::FromRgb(
                170,
                170,
                170));

        startup->AddChild(
            std::move(summary));

        //
        // MOSTRA ATÉ 5 APLICATIVOS HABILITADOS
        //

        constexpr std::size_t MaxVisibleApps = 5;

        const std::size_t appCount =
            enabledCount < MaxVisibleApps
                ? enabledCount
                : MaxVisibleApps;

        for (
            std::size_t i = 0;
            i < appCount;
            ++i)
        {
            auto appText =
                std::make_unique<Text>();

            appText->SetText(
                enabledApps[i]->name);

            appText->SetSize(
                900.0F,
                24.0F);

            appText->SetFontSize(
                15.0F);

            appText->SetFontWeight(
                FontWeight::SemiBold);

            appText->SetColor(
                Color::FromRgb(
                    7,
                    242,
                    242));

            startup->AddChild(
                std::move(appText));
        }

        //
        // MOSTRA SE EXISTEM MAIS
        //

        if (enabledCount > MaxVisibleApps)
        {
            auto moreText =
                std::make_unique<Text>();

            moreText->SetText(
                L"+ " +
                std::to_wstring(
                    enabledCount -
                    MaxVisibleApps) +
                L" outros aplicativos habilitados");

            moreText->SetSize(
                900.0F,
                24.0F);

            moreText->SetFontSize(
                13.0F);

            moreText->SetColor(
                Color::FromRgb(
                    150,
                    150,
                    150));

            startup->AddChild(
                std::move(moreText));
        }

        //
        // BOTÃO GERENCIAR INICIALIZAÇÃO
        //

        auto manageButton =
            std::make_unique<Button>();

        manageButton->SetText(
            L"Gerenciar inicialização");

        manageButton->SetSize(
            220.0F,
            48.0F);

        manageButton->SetOnClick(
            [this]()
            {
                OpenStartupSettings();
            });

        manageStartupButton_ =
            manageButton.get();

        startup->AddChild(
            std::move(manageButton));

        startupPanel_ =
            startup.get();

        pagePanel_->AddChild(
            std::move(startup));

        return true;
    }

    void DashboardPage::OpenStartupSettings()
    {
        const HINSTANCE result =
            ShellExecuteW(
                nullptr,
                L"open",
                L"ms-settings:startupapps",
                nullptr,
                nullptr,
                SW_SHOWNORMAL);

        if (
            reinterpret_cast<INT_PTR>(result) <= 32)
        {
            MessageBoxW(
                nullptr,
                L"Não foi possível abrir as configurações de inicialização do Windows.",
                L"UltraEdgeX",
                MB_OK | MB_ICONWARNING);
        }
    }

    bool DashboardPage::CreateSystemInformation()
    {
        if (contentRow_ == nullptr)
        {
            return false;
        }

        const auto details =
            system::SystemInfo::GetSystemDetails();

        auto section =
            std::make_unique<StackPanel>();

        section->SetSize(
            450.0F,
            360.0F);

        section->SetOrientation(
            Orientation::Vertical);

        section->SetSpacing(
            7.0F);

        //
        // TITLE
        //

        auto title =
            std::make_unique<Text>();

        title->SetText(
            L"INFORMAÇÕES DO SISTEMA");

        title->SetSize(
            900.0F,
            30.0F);

        title->SetFontSize(
            18.0F);

        title->SetFontWeight(
            FontWeight::SemiBold);

        title->SetColor(
            Color::FromRgb(
                220,
                220,
                220));

        section->AddChild(
            std::move(title));

        //
        // Helper para criar as linhas.
        //

        const auto addInformationLine =
            [&section](
                const std::wstring &label,
                const std::wstring &value)
        {
            auto text =
                std::make_unique<Text>();

            text->SetText(
                label +
                L":  " +
                value);

            text->SetSize(
                900.0F,
                24.0F);

            text->SetFontSize(
                14.0F);

            text->SetColor(
                Color::FromRgb(
                    190,
                    190,
                    190));

            section->AddChild(
                std::move(text));
        };

        //
        // OPERATING SYSTEM
        //

        addInformationLine(
            L"Sistema Operacional",
            details.operatingSystem);

        //
        // PROCESSOR
        //

        addInformationLine(
            L"Processador",
            details.processorName);

        //
        // MEMORY
        //

        const std::uint64_t memoryGb =
            (details.totalMemoryBytes +
             Gigabyte / 2) /
            Gigabyte;

        addInformationLine(
            L"Memória",
            std::to_wstring(memoryGb) +
                L" GB");

        //
        // ARCHITECTURE
        //

        addInformationLine(
            L"Arquitetura",
            details.architecture);

        //
        // COMPUTER
        //

        addInformationLine(
            L"Computador",
            details.computerName);

        contentRow_->AddChild(
            std::move(section));

        return true;
    }

    void DashboardPage::RefreshRealtimeStats()
    {
        if (
            cpuCard_ == nullptr ||
            memoryCard_ == nullptr)
        {
            return;
        }

        const auto cpu =
            system::SystemInfo::GetCpuInfo();

        if (cpu.valid)
        {
            cpuCard_->SetValue(
                std::to_wstring(
                    cpu.usagePercent) +
                L"%");
        }

        const auto memory =
            system::SystemInfo::GetMemoryInfo();

        memoryCard_->SetDescription(
            std::to_wstring(
                memory.usagePercent) +
            L"% em uso");
    }

    void DashboardPage::Resize(
        float width,
        float height) noexcept
    {
        if (pagePanel_ == nullptr)
        {
            return;
        }

        pagePanel_->SetSize(
            width > 80.0F
                ? width - 80.0F
                : 0.0F,

            height > 80.0F
                ? height - 80.0F
                : 0.0F);
    }
}