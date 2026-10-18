#include "features/dashboard/DashboardPage.h"

#include <windows.h>
#include <shellapi.h>

#include "roui/controls/Button.h"
#include "roui/controls/Card.h"
#include "roui/controls/DrivePicker.h"
#include "roui/controls/ProgressRing.h"
#include "roui/controls/ScrollPanel.h"
#include "roui/controls/SurfacePanel.h"
#include "roui/controls/Text.h"
#include "roui/core/Theme.h"
#include "roui/window/UiDispatcher.h"

#include "system/SystemAnalyzer.h"
#include "system/StartupCatalog.h"
#include "system/SystemInfo.h"
#include "system/StartupManager.h"

#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace real_optimizer::roui;

namespace
{
    constexpr std::uint64_t Gigabyte =
        1024ULL * 1024ULL * 1024ULL;

    // Medidas do layout (px).
    // Espaço reservado à direita do cabeçalho (seletor de tema).
    constexpr float HeaderActionReserve = 196.0F;
    constexpr float SectionGap = 20.0F;
    constexpr float HeaderHeight = 52.0F;
    constexpr float MetricCardHeight = 128.0F;
    constexpr float MiddleRowHeight = 352.0F;

    // Área de alertas do cartão de saúde: cada alerta ocupa 52 px
    // (44 do bloco + 8 de espaço) e a área mostra 3 por vez.
    constexpr float IssueRowHeight = 52.0F;
    constexpr float IssueViewportTop = 184.0F;
    constexpr float IssueViewportHeight = 148.0F;
    constexpr float IssueScrollbarGutter = 12.0F;
    constexpr float StartupRowHeight = 38.0F;
    constexpr std::size_t MaxVisibleStartupApps = 5;

    // Informações do sistema: duas colunas.
    constexpr float InfoColumnGap = 24.0F;
    constexpr float InfoRowHeight = 56.0F;

    // Largura média de um caractere (fonte 13 SemiBold), para decidir
    // onde encurtar um texto longo.
    constexpr float InfoValueCharWidth = 7.0F;

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

    void RemoveAll(
        std::wstring &text,
        const std::wstring &token)
    {
        std::size_t position = 0;

        while ((position = text.find(token)) !=
               std::wstring::npos)
        {
            text.erase(position, token.size());
        }
    }

    // "Intel(R) Core(TM) i5-9600KF CPU @ 3.70GHz"
    // vira "Intel Core i5-9600KF @ 3.70GHz".
    std::wstring SimplifyProcessorName(
        std::wstring name)
    {
        RemoveAll(name, L"(R)");
        RemoveAll(name, L"(TM)");
        RemoveAll(name, L" CPU");

        RemoveAll(name, L" Processor");
        RemoveAll(name, L" with Radeon Graphics");

        // "AMD Ryzen 7 5700X 8-Core" -> "AMD Ryzen 7 5700X"
        std::size_t core = name.find(L"-Core");

        if (core != std::wstring::npos)
        {
            std::size_t start = core;

            while (start > 0 && std::iswdigit(name[start - 1]))
            {
                --start;
            }

            if (start > 0 && name[start - 1] == L' ')
            {
                --start;
            }

            name.erase(start, core + 5 - start);
        }

        // "Intel Core i5-11400 @ 2.60GHz" -> "Intel Core i5-11400"
        const std::size_t at = name.find(L" @ ");

        if (at != std::wstring::npos)
        {
            name.erase(at);
        }

        // "13th Gen Intel Core i7" -> "Intel Core i7"
        const std::size_t gen = name.find(L" Gen ");

        if (
            gen != std::wstring::npos &&
            gen <= 6 &&
            !name.empty() &&
            std::iswdigit(name.front()))
        {
            name.erase(0, gen + 5);
        }

        std::size_t position = 0;

        while ((position = name.find(L"  ")) !=
               std::wstring::npos)
        {
            name.erase(position, 1);
        }

        while (!name.empty() && name.back() == L' ')
        {
            name.pop_back();
        }

        return name;
    }

    constexpr const wchar_t *Unknown = L"Não identificado";

    std::wstring JoinParts(
        const std::wstring &first,
        const std::wstring &second)
    {
        if (first.empty())
        {
            return second;
        }

        return second.empty() ? first : first + L" · " + second;
    }

    std::wstring FormatUptime(std::uint64_t seconds)
    {
        const std::uint64_t minutes = seconds / 60;
        const std::uint64_t hours = minutes / 60;
        const std::uint64_t days = hours / 24;

        if (days > 0)
        {
            return std::to_wstring(days) +
                   (days == 1 ? L" dia " : L" dias ") +
                   std::to_wstring(hours % 24) + L" h";
        }

        if (hours > 0)
        {
            return std::to_wstring(hours) + L" h " +
                   std::to_wstring(minutes % 60) + L" min";
        }

        return std::to_wstring(minutes == 0 ? 1 : minutes) + L" min";
    }

    std::wstring FormatGpu(const real_optimizer::system::SystemDetails &d)
    {
        if (d.gpuName.empty())
        {
            return Unknown;
        }

        std::wstring name = d.gpuName;

        RemoveAll(name, L"(R)");
        RemoveAll(name, L"(TM)");

        if (name.rfind(L"NVIDIA GeForce", 0) == 0)
        {
            name.erase(0, 7);
        }

        // Placas integradas usam a memória do sistema: sem VRAM própria.
        const std::wstring memory =
            d.gpuMemoryBytes >= Gigabyte
                ? FormatGigabytes(d.gpuMemoryBytes)
                : std::wstring();

        return JoinParts(name, memory);
    }

    std::wstring FormatDisplay(const real_optimizer::system::SystemDetails &d)
    {
        if (d.displayWidth == 0 || d.displayHeight == 0)
        {
            return Unknown;
        }

        const std::wstring resolution =
            std::to_wstring(d.displayWidth) + L"×" +
            std::to_wstring(d.displayHeight);

        return JoinParts(
            resolution,
            d.displayRefreshHz > 0
                ? std::to_wstring(d.displayRefreshHz) + L" Hz"
                : std::wstring());
    }

    // Limpa redundâncias comuns em nomes de placa-mãe.
    std::wstring SimplifyMotherboardName(std::wstring name)
    {
        RemoveAll(name, L"(R)");
        RemoveAll(name, L"(TM)");

        // Se o fabricante for duplicado no início do modelo (ex: "ASUS ROG STRIX...")
        // ou se contiver sufixos longos desnecessários mantemos limpo.
        std::size_t position = 0;
        while ((position = name.find(L"  ")) != std::wstring::npos)
        {
            name.erase(position, 1);
        }

        while (!name.empty() && name.back() == L' ')
        {
            name.pop_back();
        }

        return name;
    }

    constexpr std::size_t InfoItemCount = 10;

    // Mesma ordem dos rótulos em CreateSystemInformation.
    std::array<std::wstring, InfoItemCount> BuildInfoValues(
        const real_optimizer::system::SystemDetails &d)
    {
        const std::uint64_t memoryGb =
            (d.totalMemoryBytes + Gigabyte / 2) / Gigabyte;

        const std::wstring windows =
            JoinParts(
                d.operatingSystem.empty() ? std::wstring(L"Windows")
                                          : d.operatingSystem,
                d.windowsVersion);

        const std::wstring bios =
            JoinParts(d.biosVersion, d.biosDate);

        const std::wstring motherboard =
            d.motherboard.empty()
                ? std::wstring(Unknown)
                : SimplifyMotherboardName(d.motherboard);

        return {
            windows,
            d.computerName,
            SimplifyProcessorName(d.processorName),
            FormatGpu(d),
            std::to_wstring(memoryGb) + L" GB",
            motherboard,
            bios.empty() ? std::wstring(Unknown) : bios,
            FormatDisplay(d),
            FormatUptime(d.uptimeSeconds),
            d.powerPlan.empty() ? std::wstring(Unknown) : d.powerPlan};
    }

    std::wstring FitText(
        const std::wstring &text,
        std::size_t maxChars)
    {
        if (text.size() <= maxChars || maxChars < 2)
        {
            return text;
        }

        return text.substr(0, maxChars - 1) + L"…";
    }

    void Place(
        UIElement *element,
        float x,
        float y,
        float width,
        float height) noexcept
    {
        if (element != nullptr)
        {
            element->SetBounds({x, y, width, height});
        }
    }

    Text *AddText(
        Panel &parent,
        const std::wstring &content,
        float fontSize,
        FontWeight weight,
        ColorRole role,
        HorizontalAlignment horizontal = HorizontalAlignment::Left,
        VerticalAlignment vertical = VerticalAlignment::Center)
    {
        auto text = std::make_unique<Text>();
        Text *raw = text.get();

        text->SetText(content);
        text->SetFontSize(fontSize);
        text->SetFontWeight(weight);
        text->SetColorRole(role);
        text->SetHorizontalAlignment(horizontal);
        text->SetVerticalAlignment(vertical);

        parent.AddChild(std::move(text));

        return raw;
    }

    SurfacePanel *AddSurface(
        Panel &parent,
        SurfaceRole role,
        float cornerRadius)
    {
        auto surface = std::make_unique<SurfacePanel>();
        SurfacePanel *raw = surface.get();

        surface->SetRole(role);
        surface->SetCornerRadius(cornerRadius);

        parent.AddChild(std::move(surface));

        return raw;
    }

    Button *AddButton(
        Panel &parent,
        const std::wstring &label,
        Button::Style style)
    {
        auto button = std::make_unique<Button>();
        Button *raw = button.get();

        button->SetText(label);
        button->SetStyle(style);

        parent.AddChild(std::move(button));

        return raw;
    }

    Card *AddCard(
        Panel &parent,
        const std::wstring &title)
    {
        auto card = std::make_unique<Card>();
        Card *raw = card.get();

        card->SetTitle(title);

        parent.AddChild(std::move(card));

        return raw;
    }
}

namespace real_optimizer::features::dashboard
{
    DashboardPage::~DashboardPage()
    {
        Shutdown();
    }

    void DashboardPage::Shutdown()
    {
        shuttingDown_ = true;

        if (worker_.joinable())
        {
            worker_.join();
        }
    }

    void DashboardPage::OnShown()
    {
        // Discos podem ter sido conectados/removidos.
        CloseMenus();
        RefreshDriveList();

        // Tempo ligado, plano de energia e monitor mudam com o tempo.
        // (O Layout feito abaixo já aplica os textos novos.)
        RefreshSystemInformation();

        // A lista de inicialização pode ter mudado.
        RefreshStartupSection();

        // Depois de uma limpeza, por exemplo, os números mudam.
        if (hasAnalysis_)
        {
            AnalyzeSystem();
        }
    }

    bool DashboardPage::Initialize(
        Panel &pageRoot,
        UiDispatcher &dispatcher,
        std::function<void()> openCleaner,
        std::function<void()> openServices,
        std::function<void()> openStartup)
    {
        dispatcher_ = &dispatcher;
        openCleaner_ = std::move(openCleaner);
        openServices_ = std::move(openServices);
        openStartup_ = std::move(openStartup);

        if (
            !CreateHeader(pageRoot) ||
            !CreateMetrics(pageRoot) ||
            !CreateHealthSection(pageRoot) ||
            !CreateSystemInformation(pageRoot) ||
            !CreateStartupSection(pageRoot))
        {
            return false;
        }

        // Menu de escolha de disco: último filho (desenha por cima de
        // tudo) e modal enquanto aberto.
        {
            auto picker = std::make_unique<DrivePicker>();
            drivePicker_ = picker.get();

            pageRoot.AddChild(std::move(picker));
            pageRoot.SetModalChild(drivePicker_);

            drivePicker_->Hide();
        }

        diskCard_->SetOnSelectorClick(
            [this]()
            {
                OpenDriveMenu();
            });

        RefreshDriveList();

        return true;
    }

    //
    // CABEÇALHO
    //

    bool DashboardPage::CreateHeader(
        Panel &rootPanel)
    {
        titleText_ = AddText(
            rootPanel,
            L"Painel",
            28.0F,
            FontWeight::Bold,
            ColorRole::Primary);

        subtitleText_ = AddText(
            rootPanel,
            L"Performance & Optimization",
            14.0F,
            FontWeight::Normal,
            ColorRole::Secondary);

        analyzeButton_ = AddButton(
            rootPanel,
            L"Analisar sistema",
            Button::Style::Primary);

        analyzeButton_->SetOnClick(
            [this]()
            {
                AnalyzeSystem();
            });

        return true;
    }

    //
    // MÉTRICAS
    //

    bool DashboardPage::CreateMetrics(
        Panel &rootPanel)
    {
        const auto memory =
            system::SystemInfo::GetMemoryInfo();

        const auto disk =
            system::SystemInfo::GetSystemDiskInfo();

        // Primeira amostra necessária
        // para calcular CPU depois.
        static_cast<void>(
            system::SystemInfo::GetCpuInfo());

        cpuCard_ = AddCard(rootPanel, L"PROCESSADOR");
        cpuCard_->SetValue(L"--%");
        cpuCard_->SetDescription(L"Uso da CPU");

        memoryCard_ = AddCard(rootPanel, L"MEMÓRIA");
        memoryCard_->SetValue(
            FormatGigabytes(memory.totalBytes));
        memoryCard_->SetDescription(
            std::to_wstring(memory.usagePercent) +
            L"% em uso");
        memoryCard_->SetProgress(
            static_cast<float>(memory.usagePercent) /
            100.0F);

        diskCard_ = AddCard(rootPanel, L"DISCO DO SISTEMA");
        diskCard_->SetValue(
            FormatGigabytes(disk.totalBytes));
        diskCard_->SetDescription(
            FormatGigabytes(disk.freeBytes) +
            L" disponíveis");

        if (disk.totalBytes > 0)
        {
            const double used =
                static_cast<double>(
                    disk.totalBytes - disk.freeBytes) /
                static_cast<double>(disk.totalBytes);

            diskCard_->SetProgress(
                static_cast<float>(used));
        }

        return true;
    }

    //
    // SAÚDE DO SISTEMA
    //

    bool DashboardPage::CreateHealthSection(
        Panel &rootPanel)
    {
        healthSurface_ = AddSurface(
            rootPanel,
            SurfaceRole::Surface,
            16.0F);

        healthRing_ = nullptr;

        {
            auto ring = std::make_unique<ProgressRing>();
            healthRing_ = ring.get();

            ring->SetCaption(L"de 100");

            healthSurface_->AddChild(std::move(ring));
        }

        healthTitle_ = AddText(
            *healthSurface_,
            L"SAÚDE DO SISTEMA",
            12.0F,
            FontWeight::SemiBold,
            ColorRole::Secondary);

        healthText_ = AddText(
            *healthSurface_,
            L"Ainda não analisado",
            22.0F,
            FontWeight::SemiBold,
            ColorRole::Primary);

        analysisStatusText_ = AddText(
            *healthSurface_,
            L"Pronto para analisar seu sistema.",
            13.0F,
            FontWeight::Normal,
            ColorRole::Secondary,
            HorizontalAlignment::Left,
            VerticalAlignment::Top);

        {
            auto scroll = std::make_unique<ScrollPanel>();
            issueScroll_ = scroll.get();

            issueScroll_->SetOnScroll(
                [this]()
                {
                    Layout(area_);
                });

            healthSurface_->AddChild(std::move(scroll));
        }

        for (std::size_t i = 0; i < issueSlots_.size(); ++i)
        {
            IssueSlot &slot = issueSlots_[i];

            slot.box = AddSurface(
                *issueScroll_,
                SurfaceRole::Inset,
                10.0F);

            slot.title = AddText(
                *slot.box,
                L"",
                13.0F,
                FontWeight::SemiBold,
                ColorRole::Primary);

            slot.description = AddText(
                *slot.box,
                L"",
                12.0F,
                FontWeight::Normal,
                ColorRole::Secondary);

            // Cada alerta tem o próprio botão, ao lado do bloco.
            slot.button = AddButton(
                *issueScroll_,
                L"",
                Button::Style::Secondary);

            slot.button->SetOnClick(
                [this, i]()
                {
                    RunIssueAction(issueSlots_[i].action);
                });

            slot.button->SetVisible(false);
            slot.button->SetEnabled(false);

            slot.box->SetVisible(false);
        }

        return true;
    }

    void DashboardPage::RunIssueAction(
        system::IssueAction action)
    {
        if (
            action == system::IssueAction::OpenCleaner &&
            openCleaner_)
        {
            openCleaner_();
        }
        else if (
            action == system::IssueAction::OpenServices &&
            openServices_)
        {
            openServices_();
        }
    }

    void DashboardPage::AnalyzeSystem()
    {
        if (
            analyzing_ ||
            shuttingDown_ ||
            dispatcher_ == nullptr ||
            analyzeButton_ == nullptr)
        {
            return;
        }

        analyzing_ = true;

        analyzeButton_->SetText(L"Analisando…");
        analyzeButton_->SetEnabled(false);

        if (worker_.joinable())
        {
            worker_.join();
        }

        // A análise lê várias pastas; roda fora da thread da
        // interface para a janela não travar.
        worker_ = std::thread(
            [this]()
            {
                system::AnalysisResult result =
                    system::SystemAnalyzer::Analyze();

                dispatcher_->Post(
                    [this, result = std::move(result)]()
                    {
                        analyzing_ = false;

                        if (!shuttingDown_)
                        {
                            ApplyAnalysis(result);
                        }
                    });
            });
    }

    void DashboardPage::ApplyAnalysis(
        const system::AnalysisResult &result)
    {
        if (
            healthText_ == nullptr ||
            analysisStatusText_ == nullptr ||
            analyzeButton_ == nullptr ||
            healthRing_ == nullptr)
        {
            return;
        }

        //
        // HEALTH SCORE
        //

        const int score =
            std::clamp(result.healthScore, 0, 100);

        std::wstring healthStatus;

        if (score >= 90)
        {
            healthStatus = L"Excelente";
        }
        else if (score >= 75)
        {
            healthStatus = L"Bom";
        }
        else if (score >= 50)
        {
            healthStatus = L"Atenção necessária";
        }
        else
        {
            healthStatus = L"Estado crítico";
        }

        healthRing_->SetValue(
            static_cast<float>(score));

        healthText_->SetText(healthStatus);

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
        // PROBLEMAS (todos; a área rola quando passam de 3)
        //

        if (issueScroll_ != nullptr)
        {
            issueScroll_->SetScrollOffset(0.0F);
        }

        visibleIssueCount_ =
            std::min(issueCount, issueSlots_.size());

        for (std::size_t i = 0; i < issueSlots_.size(); ++i)
        {
            IssueSlot &slot = issueSlots_[i];

            if (i < visibleIssueCount_)
            {
                slot.title->SetText(
                    result.issues[i].title);

                slot.description->SetText(
                    result.issues[i].description);

                slot.action = result.issues[i].action;

                const bool hasAction =
                    slot.action != system::IssueAction::None;

                slot.button->SetText(
                    slot.action == system::IssueAction::OpenServices
                        ? L"Ver em Serviços"
                        : L"Ver em Limpeza");

                slot.button->SetVisible(hasAction);
                slot.button->SetEnabled(hasAction);

                slot.box->SetVisible(true);
            }
            else
            {
                slot.action = system::IssueAction::None;
                slot.title->SetText(L"");
                slot.description->SetText(L"");
                slot.button->SetVisible(false);
                slot.button->SetEnabled(false);
                slot.box->SetVisible(false);
            }
        }

        hasAnalysis_ = true;

        analyzeButton_->SetText(
            L"Analisar novamente");

        analyzeButton_->SetEnabled(true);

        Layout(area_);
    }

    //
    // INFORMAÇÕES DO SISTEMA
    //

    bool DashboardPage::CreateSystemInformation(
        Panel &rootPanel)
    {
        const auto details =
            system::SystemInfo::GetSystemDetails();

        infoSurface_ = AddSurface(
            rootPanel,
            SurfaceRole::Surface,
            16.0F);

        infoTitle_ = AddText(
            *infoSurface_,
            L"INFORMAÇÕES DO SISTEMA",
            12.0F,
            FontWeight::SemiBold,
            ColorRole::Secondary);

        const std::wstring labels[InfoItemCount] = {
            L"Sistema operacional",
            L"Computador",
            L"Processador",
            L"Placa de vídeo",
            L"Memória",
            L"Placa-mãe",
            L"BIOS",
            L"Monitor",
            L"Tempo ligado",
            L"Plano de energia"};

        const auto values = BuildInfoValues(details);

        for (std::size_t i = 0; i < infoRows_.size(); ++i)
        {
            InfoRow &row = infoRows_[i];

            // O divisor atravessa as duas colunas: só a coluna da
            // esquerda cria, e não na primeira linha.
            if (i >= 2 && i % 2 == 0)
            {
                row.divider = AddSurface(
                    *infoSurface_,
                    SurfaceRole::Divider,
                    0.0F);
            }

            row.label = AddText(
                *infoSurface_,
                labels[i],
                12.0F,
                FontWeight::Normal,
                ColorRole::Secondary);

            row.fullValue = values[i];

            row.value = AddText(
                *infoSurface_,
                values[i],
                13.0F,
                FontWeight::SemiBold,
                ColorRole::Primary);
        }

        return true;
    }

    void DashboardPage::RefreshSystemInformation()
    {
        const auto values =
            BuildInfoValues(system::SystemInfo::GetSystemDetails());

        for (std::size_t i = 0; i < infoRows_.size(); ++i)
        {
            infoRows_[i].fullValue = values[i];

            // Força o texto a ser ajustado de novo no Layout.
            infoRows_[i].shownChars = 0;
        }
    }

    //
    // APLICATIVOS DE INICIALIZAÇÃO
    //

    bool DashboardPage::CreateStartupSection(
        Panel &rootPanel)
    {
        startupSurface_ = AddSurface(
            rootPanel,
            SurfaceRole::Surface,
            16.0F);

        BuildStartupContents();

        return true;
    }

    void DashboardPage::RefreshStartupSection()
    {
        if (startupSurface_ == nullptr)
        {
            return;
        }

        startupSurface_->ClearChildren();

        startupRows_.clear();
        startupTitle_ = nullptr;
        startupSummary_ = nullptr;
        startupMore_ = nullptr;
        manageStartupButton_ = nullptr;

        BuildStartupContents();

        // Só reposiciona se a página já foi posicionada alguma vez.
        if (area_.width > 0.0F)
        {
            Layout(area_);
        }
    }

    void DashboardPage::BuildStartupContents()
    {
        const auto startupApps =
            system::StartupManager::GetStartupApps();

        // Somente os aplicativos realmente
        // habilitados no Windows.
        std::vector<const system::StartupApp *> enabledApps;

        for (const auto &app : startupApps)
        {
            if (app.enabled)
            {
                enabledApps.push_back(&app);
            }
        }

        const std::size_t enabledCount =
            enabledApps.size();

        startupTitle_ = AddText(
            *startupSurface_,
            L"APLICATIVOS DE INICIALIZAÇÃO",
            12.0F,
            FontWeight::SemiBold,
            ColorRole::Secondary);

        std::wstring summary;

        if (enabledCount == 0)
        {
            summary =
                L"Nenhum aplicativo habilitado para iniciar com o Windows.";
        }
        else if (enabledCount == 1)
        {
            summary =
                L"1 aplicativo habilitado para iniciar com o Windows.";
        }
        else
        {
            summary =
                std::to_wstring(enabledCount) +
                L" aplicativos habilitados para iniciar com o Windows.";
        }

        startupSummary_ = AddText(
            *startupSurface_,
            summary,
            13.0F,
            FontWeight::Normal,
            ColorRole::Muted);

        manageStartupButton_ = AddButton(
            *startupSurface_,
            L"Gerenciar inicialização",
            Button::Style::Secondary);

        manageStartupButton_->SetOnClick(
            [this]()
            {
                if (openStartup_)
                {
                    openStartup_();
                }
            });

        const std::size_t visibleCount =
            std::min(enabledCount, MaxVisibleStartupApps);

        for (std::size_t i = 0; i < visibleCount; ++i)
        {
            StartupRow row;

            row.divider = AddSurface(
                *startupSurface_,
                SurfaceRole::Divider,
                0.0F);

            row.icon = AddSurface(
                *startupSurface_,
                SurfaceRole::AccentSoft,
                7.0F);

            // Nome amigável (o nome bruto de apps da Loja é um
            // identificador de pacote).
            const std::wstring name =
                system::StartupCatalog::Describe(*enabledApps[i])
                    .displayName;

            std::wstring letter =
                name.empty()
                    ? std::wstring(L"?")
                    : std::wstring(1, name.front());

            row.iconLetter = AddText(
                *row.icon,
                letter,
                13.0F,
                FontWeight::Bold,
                ColorRole::Accent,
                HorizontalAlignment::Center);

            row.name = AddText(
                *startupSurface_,
                name,
                14.0F,
                FontWeight::SemiBold,
                ColorRole::Primary);

            row.badge = AddSurface(
                *startupSurface_,
                SurfaceRole::AccentSoft,
                11.0F);

            row.badgeText = AddText(
                *row.badge,
                L"Habilitado",
                11.0F,
                FontWeight::SemiBold,
                ColorRole::Accent,
                HorizontalAlignment::Center);

            startupRows_.push_back(row);
        }

        if (enabledCount > MaxVisibleStartupApps)
        {
            startupMore_ = AddText(
                *startupSurface_,
                L"+ " +
                    std::to_wstring(
                        enabledCount - MaxVisibleStartupApps) +
                    L" outros aplicativos habilitados",
                13.0F,
                FontWeight::Normal,
                ColorRole::Muted);
        }
    }

    //
    // DISCO MOSTRADO NO CARD
    //

    void DashboardPage::CloseMenus()
    {
        if (drivePicker_ != nullptr)
        {
            drivePicker_->Hide();
        }
    }

    bool DashboardPage::OnKeyDown(unsigned int virtualKey)
    {
        return drivePicker_ != nullptr &&
               drivePicker_->OnKeyDown(virtualKey);
    }

    void DashboardPage::RefreshDriveList()
    {
        drives_ = system::SystemInfo::GetFixedDrives();

        // Se o disco escolhido sumiu (ex.: HD externo desconectado),
        // volta para o disco do sistema.
        if (!selectedDriveRoot_.empty())
        {
            const bool found = std::any_of(
                drives_.begin(),
                drives_.end(),
                [this](const system::DriveInfo &drive)
                { return drive.root == selectedDriveRoot_; });

            if (!found)
            {
                selectedDriveRoot_.clear();
            }
        }

        if (diskCard_ != nullptr)
        {
            // Com um disco só, não há o que escolher: sem seta.
            diskCard_->SetSelectorVisible(drives_.size() > 1);
        }

        UpdateDiskCard();
    }

    void DashboardPage::UpdateDiskCard()
    {
        if (diskCard_ == nullptr)
        {
            return;
        }

        system::DiskInfo disk =
            selectedDriveRoot_.empty()
                ? system::SystemInfo::GetSystemDiskInfo()
                : system::SystemInfo::GetDiskInfo(selectedDriveRoot_);

        if (!selectedDriveRoot_.empty() && disk.totalBytes == 0)
        {
            selectedDriveRoot_.clear();
            disk = system::SystemInfo::GetSystemDiskInfo();
        }

        std::wstring title = L"DISCO DO SISTEMA";

        if (!selectedDriveRoot_.empty())
        {
            title = L"DISCO " + selectedDriveRoot_.substr(0, 2);

            for (const system::DriveInfo &drive : drives_)
            {
                if (drive.root == selectedDriveRoot_ &&
                    !drive.label.empty())
                {
                    std::wstring label = drive.label;

                    for (wchar_t &c : label)
                    {
                        c = static_cast<wchar_t>(std::towupper(c));
                    }

                    title += L" · " + label;
                    break;
                }
            }
        }

        diskCard_->SetTitle(title);
        diskCard_->SetValue(FormatGigabytes(disk.totalBytes));

        diskCard_->SetDescription(
            FormatGigabytes(disk.freeBytes) + L" disponíveis");

        if (disk.totalBytes > 0)
        {
            const double used =
                static_cast<double>(disk.totalBytes - disk.freeBytes) /
                static_cast<double>(disk.totalBytes);

            diskCard_->SetProgress(static_cast<float>(used));
        }
    }

    void DashboardPage::OpenDriveMenu()
    {
        if (drivePicker_ == nullptr || diskCard_ == nullptr)
        {
            return;
        }

        if (drivePicker_->IsOpen())
        {
            drivePicker_->Hide();
            return;
        }

        // Lista atualizada na hora de abrir.
        RefreshDriveList();

        if (drives_.size() < 2)
        {
            return;
        }

        const std::wstring currentRoot =
            selectedDriveRoot_.empty()
                ? [this]() -> std::wstring
                {
                    for (const system::DriveInfo &drive : drives_)
                    {
                        if (drive.isSystem)
                        {
                            return drive.root;
                        }
                    }

                    return {};
                }()
                : selectedDriveRoot_;

        std::vector<DrivePickerItem> items;
        std::size_t selectedIndex = DrivePicker::MaxItems;

        for (std::size_t i = 0;
             i < drives_.size() && i < DrivePicker::MaxItems;
             ++i)
        {
            const system::DriveInfo &drive = drives_[i];

            DrivePickerItem item;

            item.title =
                drive.root.substr(0, 2) + L" " +
                (drive.label.empty() ? std::wstring(L"Disco local")
                                     : drive.label) +
                (drive.isSystem ? L" (Sistema)" : L"");

            std::wstring kind;

            if (drive.kind == system::DriveKind::Ssd)
            {
                kind = L"SSD";
            }
            else if (drive.kind == system::DriveKind::Hdd)
            {
                kind = L"HD";
            }

            const std::wstring space =
                FormatGigabytes(drive.freeBytes) + L" livres de " +
                FormatGigabytes(drive.totalBytes);

            item.detail = kind.empty() ? space : kind + L" · " + space;

            item.usedFraction =
                drive.totalBytes > 0
                    ? static_cast<float>(
                          static_cast<double>(drive.totalBytes -
                                              drive.freeBytes) /
                          static_cast<double>(drive.totalBytes))
                    : 0.0F;

            if (drive.root == currentRoot)
            {
                selectedIndex = i;
            }

            items.push_back(std::move(item));
        }

        drivePicker_->Show(
            std::move(items),
            selectedIndex,
            diskCard_->GetSelectorBounds(),
            [this](std::size_t index)
            {
                if (index >= drives_.size())
                {
                    return;
                }

                // Escolher o disco do sistema volta ao modo padrão.
                selectedDriveRoot_ =
                    drives_[index].isSystem ? std::wstring()
                                            : drives_[index].root;

                UpdateDiskCard();
            },
            [this]()
            {
                if (diskCard_ != nullptr)
                {
                    diskCard_->SetSelectorOpen(false);
                }
            });

        diskCard_->SetSelectorOpen(true);
    }

    //
    // TEMPO REAL
    //

    void DashboardPage::RefreshRealtimeStats()
    {
        if (
            cpuCard_ == nullptr ||
            memoryCard_ == nullptr ||
            diskCard_ == nullptr)
        {
            return;
        }

        const auto cpu =
            system::SystemInfo::GetCpuInfo();

        if (cpu.valid)
        {
            cpuCard_->SetValue(
                std::to_wstring(cpu.usagePercent) +
                L"%");

            cpuCard_->SetProgress(
                static_cast<float>(cpu.usagePercent) /
                100.0F);
        }

        const auto memory =
            system::SystemInfo::GetMemoryInfo();

        memoryCard_->SetDescription(
            std::to_wstring(memory.usagePercent) +
            L"% em uso");

        memoryCard_->SetProgress(
            static_cast<float>(memory.usagePercent) /
            100.0F);

        UpdateDiskCard();
    }

    //
    // LAYOUT
    //

    void DashboardPage::Layout(
        const Rect &area)
    {
        area_ = area;

        if (drivePicker_ != nullptr)
        {
            drivePicker_->SetBounds(area);
        }

        if (titleText_ == nullptr || issueScroll_ == nullptr)
        {
            return;
        }

        const float mainX = area.x;
        const float mainWidth = area.width;

        float y = area.y;

        // Cabeçalho (o seletor de tema fica por conta do shell)
        Place(titleText_, mainX, y, 400.0F, 36.0F);
        Place(subtitleText_, mainX, y + 38.0F, 400.0F, 20.0F);

        Place(
            analyzeButton_,
            mainX + mainWidth - HeaderActionReserve - 180.0F,
            y,
            180.0F,
            48.0F);

        y += HeaderHeight + SectionGap;

        // Métricas
        const float cardWidth =
            (mainWidth - SectionGap * 2.0F) / 3.0F;

        Card *cards[] = {cpuCard_, memoryCard_, diskCard_};

        for (std::size_t i = 0; i < 3; ++i)
        {
            Place(
                cards[i],
                mainX + static_cast<float>(i) * (cardWidth + SectionGap),
                y,
                cardWidth,
                MetricCardHeight);
        }

        y += MetricCardHeight + SectionGap;

        //
        // Saúde do sistema + Informações do sistema
        //

        const float healthWidth =
            (mainWidth - SectionGap) * 0.49F;

        const float infoWidth =
            mainWidth - SectionGap - healthWidth;

        Place(healthSurface_, mainX, y, healthWidth, MiddleRowHeight);

        const float ringSize = 140.0F;

        Place(healthRing_, mainX + 26.0F, y + 26.0F, ringSize, ringSize);

        const float textX = mainX + 26.0F + ringSize + 26.0F;
        const float textWidth = healthWidth - (textX - mainX) - 26.0F;

        Place(healthTitle_, textX, y + 28.0F, textWidth, 18.0F);
        Place(healthText_, textX, y + 48.0F, textWidth, 28.0F);
        Place(analysisStatusText_, textX, y + 80.0F, textWidth, 34.0F);

        // Alertas: ficam numa área com rolagem. O bloco ocupa a largura
        // toda; se tiver ação, o botão fica à direita e o bloco encolhe
        // para ele caber. Uma faixa à direita fica livre para a barra
        // de rolagem não ficar em cima dos botões.
        constexpr float IssueButtonWidth = 136.0F;
        constexpr float IssueButtonGap = 8.0F;

        const float fullIssueWidth = healthWidth - 52.0F;
        const float contentWidth = fullIssueWidth - IssueScrollbarGutter;

        const Rect issueViewport{
            mainX + 26.0F,
            y + IssueViewportTop,
            fullIssueWidth,
            IssueViewportHeight};

        issueScroll_->SetBounds(issueViewport);

        issueScroll_->SetContentHeight(
            static_cast<float>(visibleIssueCount_) * IssueRowHeight);

        const float issueOffset = issueScroll_->GetScrollOffset();

        for (std::size_t i = 0; i < issueSlots_.size(); ++i)
        {
            const IssueSlot &slot = issueSlots_[i];

            const bool hasAction =
                slot.action != system::IssueAction::None;

            const float boxY =
                issueViewport.y +
                static_cast<float>(i) * IssueRowHeight -
                issueOffset;

            const float boxWidth = hasAction
                                       ? contentWidth - IssueButtonWidth - IssueButtonGap
                                       : contentWidth;

            Place(slot.box, issueViewport.x, boxY, boxWidth, 44.0F);
            Place(slot.title, issueViewport.x + 14.0F, boxY + 5.0F, boxWidth - 28.0F, 18.0F);
            Place(slot.description, issueViewport.x + 14.0F, boxY + 23.0F, boxWidth - 28.0F, 16.0F);

            Place(
                slot.button,
                issueViewport.x + boxWidth + IssueButtonGap,
                boxY + 2.0F,
                IssueButtonWidth,
                40.0F);
        }

        const float infoX = mainX + healthWidth + SectionGap;

        Place(infoSurface_, infoX, y, infoWidth, MiddleRowHeight);
        Place(infoTitle_, infoX + 26.0F, y + 24.0F, infoWidth - 52.0F, 18.0F);

        const float infoInnerX = infoX + 26.0F;
        const float infoInnerWidth = infoWidth - 52.0F;

        const float columnWidth =
            (infoInnerWidth - InfoColumnGap) / 2.0F;

        const auto maxChars = static_cast<std::size_t>(
            std::max(8.0F, columnWidth / InfoValueCharWidth));

        for (std::size_t i = 0; i < infoRows_.size(); ++i)
        {
            InfoRow &row = infoRows_[i];

            const std::size_t line = i / 2;
            const std::size_t column = i % 2;

            const float rowY =
                y + 58.0F + static_cast<float>(line) * InfoRowHeight;

            const float columnX =
                infoInnerX +
                static_cast<float>(column) * (columnWidth + InfoColumnGap);

            Place(row.divider, infoInnerX, rowY, infoInnerWidth, 1.0F);
            Place(row.label, columnX, rowY + 8.0F, columnWidth, 16.0F);
            Place(row.value, columnX, rowY + 26.0F, columnWidth, 22.0F);

            // Texto longo é encurtado com "…" para caber na coluna.
            if (row.value != nullptr && row.shownChars != maxChars)
            {
                row.value->SetText(FitText(row.fullValue, maxChars));
                row.shownChars = maxChars;
            }
        }

        y += MiddleRowHeight + SectionGap;

        //
        // Aplicativos de inicialização
        //

        const float rowsHeight =
            static_cast<float>(startupRows_.size()) * StartupRowHeight;

        const float moreHeight =
            startupMore_ != nullptr ? 28.0F : 0.0F;

        Place(
            startupSurface_,
            mainX,
            y,
            mainWidth,
            66.0F + rowsHeight + moreHeight + 16.0F);

        Place(startupTitle_, mainX + 26.0F, y + 22.0F, mainWidth - 300.0F, 18.0F);
        Place(startupSummary_, mainX + 26.0F, y + 42.0F, mainWidth - 300.0F, 20.0F);
        Place(manageStartupButton_, mainX + mainWidth - 26.0F - 190.0F, y + 16.0F, 190.0F, 38.0F);

        for (std::size_t i = 0; i < startupRows_.size(); ++i)
        {
            const StartupRow &row = startupRows_[i];
            const float rowY = y + 66.0F + static_cast<float>(i) * StartupRowHeight;

            Place(row.divider, mainX + 26.0F, rowY, mainWidth - 52.0F, 1.0F);
            Place(row.icon, mainX + 26.0F, rowY + 7.0F, 24.0F, 24.0F);
            Place(row.iconLetter, mainX + 26.0F, rowY + 7.0F, 24.0F, 24.0F);
            Place(row.name, mainX + 62.0F, rowY + 1.0F, mainWidth - 220.0F, StartupRowHeight - 1.0F);

            const float badgeX = mainX + mainWidth - 26.0F - 84.0F;

            Place(row.badge, badgeX, rowY + 8.0F, 84.0F, 22.0F);
            Place(row.badgeText, badgeX, rowY + 8.0F, 84.0F, 22.0F);
        }

        Place(
            startupMore_,
            mainX + 26.0F,
            y + 66.0F + rowsHeight + 6.0F,
            mainWidth - 52.0F,
            20.0F);
    }
}
