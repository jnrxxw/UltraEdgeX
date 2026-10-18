#include "features/shell/MainShell.h"

#include <windows.h>

#include "resources/Resource.h"
#include "roui/controls/Button.h"
#include "roui/controls/ConfirmDialog.h"
#include "roui/controls/ImageBox.h"
#include "roui/controls/SurfacePanel.h"
#include "roui/controls/Text.h"
#include "roui/core/Theme.h"

#include "system/SystemInfo.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

using namespace real_optimizer::roui;

namespace
{
    constexpr float SidebarWidth = 232.0F;
    constexpr float PageMarginX = 40.0F;
    constexpr float PageMarginTop = 28.0F;
    constexpr float PageMarginBottom = 24.0F;

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
        HorizontalAlignment horizontal = HorizontalAlignment::Left)
    {
        auto text = std::make_unique<Text>();
        Text *raw = text.get();

        text->SetText(content);
        text->SetFontSize(fontSize);
        text->SetFontWeight(weight);
        text->SetColorRole(role);
        text->SetHorizontalAlignment(horizontal);
        text->SetVerticalAlignment(VerticalAlignment::Center);

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

    Panel *AddPageRoot(Panel &parent)
    {
        auto panel = std::make_unique<Panel>();
        Panel *raw = panel.get();

        parent.AddChild(std::move(panel));

        return raw;
    }

    ImageBox *AddImage(
        Panel &parent,
        HMODULE module,
        int resourceId,
        const std::wstring &filePath)
    {
        auto image = std::make_unique<ImageBox>();
        ImageBox *raw = image.get();

        image->SetResource(module, resourceId);
        image->SetFilePath(filePath);

        parent.AddChild(std::move(image));

        return raw;
    }
}

namespace real_optimizer::features::shell
{
    bool MainShell::Initialize(
        Panel &rootPanel,
        UiDispatcher &dispatcher)
    {
        if (
            !CreateSidebar(rootPanel) ||
            !CreateThemeToggle(rootPanel))
        {
            return false;
        }

        dashboardRoot_ = AddPageRoot(rootPanel);
        cleanerRoot_ = AddPageRoot(rootPanel);
        startupRoot_ = AddPageRoot(rootPanel);
        servicesRoot_ = AddPageRoot(rootPanel);

        // A caixa de confirmação vem por último para ficar por cima
        // de tudo e bloquear o resto da tela enquanto estiver aberta.
        {
            auto dialog = std::make_unique<ConfirmDialog>();
            confirmDialog_ = dialog.get();

            rootPanel.AddChild(std::move(dialog));

            if (!confirmDialog_->Initialize())
            {
                return false;
            }

            rootPanel.SetModalChild(confirmDialog_);
        }

        cleanerController_ =
            std::make_unique<cleaner::CleanerController>(dispatcher);

        startupController_ =
            std::make_unique<startup::StartupController>(dispatcher);

        servicesController_ =
            std::make_unique<services::ServicesController>(dispatcher);

        if (!dashboardPage_.Initialize(
                *dashboardRoot_,
                dispatcher,
                [this]()
                {
                    ShowPage(Page::Cleaner);
                },
                [this]()
                {
                    ShowPage(Page::Services);
                },
                [this]()
                {
                    ShowPage(Page::Startup);
                }))
        {
            return false;
        }

        if (!cleanerPage_.Initialize(
                *cleanerRoot_,
                *cleanerController_,
                *confirmDialog_))
        {
            return false;
        }

        if (!startupPage_.Initialize(
                *startupRoot_,
                *startupController_,
                *confirmDialog_))
        {
            return false;
        }

        if (!servicesPage_.Initialize(
                *servicesRoot_,
                *servicesController_,
                *confirmDialog_))
        {
            return false;
        }

        UpdateThemeButtons();
        ShowPage(Page::Dashboard);
        Layout();

        // Quando reaberto como administrador, já abre na Limpeza.
        const std::wstring commandLine = GetCommandLineW();

        if (commandLine.find(L"--open-cleaner") != std::wstring::npos)
        {
            ShowPage(Page::Cleaner);
        }

        return true;
    }

    void MainShell::Shutdown()
    {
        dashboardPage_.Shutdown();

        if (startupController_ != nullptr)
        {
            startupController_->Shutdown();
        }

        if (servicesController_ != nullptr)
        {
            servicesController_->Shutdown();
        }

        if (cleanerController_ != nullptr)
        {
            cleanerController_->Shutdown();
        }
    }

    //
    // BARRA LATERAL
    //

    bool MainShell::CreateSidebar(
        Panel &rootPanel)
    {
        sidebar_ = AddSurface(
            rootPanel,
            SurfaceRole::Sidebar,
            0.0F);

        brandLogo_ = AddImage(
            *sidebar_,
            GetModuleHandleW(nullptr),
            IDR_LOGO_PNG,
            L"assets/logo.png");

        brandText_ = AddText(
            *sidebar_,
            L"UeX",
            22.0F,
            FontWeight::Bold,
            ColorRole::Accent);

        struct NavItem
        {
            const wchar_t *label;
            const wchar_t *badge;
            bool enabled;
        };

        const NavItem items[] = {
            {L"Painel", L"", true},
            {L"Limpeza", L"", true},
            {L"Inicialização", L"", true},
            {L"Serviços", L"", true},
            {L"Configurações", L"Em breve", false}};

        for (std::size_t i = 0; i < navButtons_.size(); ++i)
        {
            Button *button = AddButton(
                *sidebar_,
                items[i].label,
                Button::Style::Nav);

            button->SetBadge(items[i].badge);
            button->SetEnabled(items[i].enabled);

            navButtons_[i] = button;
        }

        navButtons_[0]->SetOnClick(
            [this]()
            {
                ShowPage(Page::Dashboard);
            });

        navButtons_[1]->SetOnClick(
            [this]()
            {
                ShowPage(Page::Cleaner);
            });

        navButtons_[3]->SetOnClick(
            [this]()
            {
                ShowPage(Page::Services);
            });

        navButtons_[2]->SetOnClick(
            [this]()
            {
                ShowPage(Page::Startup);
            });

        const auto details =
            system::SystemInfo::GetSystemDetails();

        machineChip_ = AddSurface(
            *sidebar_,
            SurfaceRole::Inset,
            12.0F);

        machineLabel_ = AddText(
            *machineChip_,
            L"Este computador",
            12.0F,
            FontWeight::Normal,
            ColorRole::Secondary);

        machineName_ = AddText(
            *machineChip_,
            details.computerName,
            14.0F,
            FontWeight::SemiBold,
            ColorRole::Primary);

        machineSystem_ = AddText(
            *machineChip_,
            details.operatingSystem,
            12.0F,
            FontWeight::Normal,
            ColorRole::Secondary);

        return true;
    }

    //
    // SELETOR DE TEMA
    //

    bool MainShell::CreateThemeToggle(
        Panel &rootPanel)
    {
        toggleSurface_ = AddSurface(
            rootPanel,
            SurfaceRole::Surface,
            12.0F);

        darkButton_ = AddButton(
            *toggleSurface_,
            L"Escuro",
            Button::Style::Segment);

        darkButton_->SetOnClick(
            [this]()
            {
                SetTheme(ThemeMode::Dark);
            });

        lightButton_ = AddButton(
            *toggleSurface_,
            L"Claro",
            Button::Style::Segment);

        lightButton_->SetOnClick(
            [this]()
            {
                SetTheme(ThemeMode::Light);
            });

        return true;
    }

    void MainShell::SetTheme(
        ThemeMode mode)
    {
        Theme::SetMode(mode);
        Theme::SavePreference();

        UpdateThemeButtons();
    }

    void MainShell::UpdateThemeButtons()
    {
        const bool isDark =
            Theme::GetMode() == ThemeMode::Dark;

        if (darkButton_ != nullptr)
        {
            darkButton_->SetActive(isDark);
        }

        if (lightButton_ != nullptr)
        {
            lightButton_->SetActive(!isDark);
        }
    }

    //
    // PÁGINAS
    //

    void MainShell::ShowPage(Page page)
    {
        // Menus abertos do Painel fecham ao trocar de página.
        dashboardPage_.CloseMenus();

        dashboardRoot_->SetVisible(page == Page::Dashboard);
        cleanerRoot_->SetVisible(page == Page::Cleaner);
        startupRoot_->SetVisible(page == Page::Startup);
        servicesRoot_->SetVisible(page == Page::Services);

        navButtons_[0]->SetActive(page == Page::Dashboard);
        navButtons_[1]->SetActive(page == Page::Cleaner);
        navButtons_[2]->SetActive(page == Page::Startup);
        navButtons_[3]->SetActive(page == Page::Services);

        if (page == Page::Cleaner)
        {
            cleanerPage_.OnShown();
        }
        else if (page == Page::Startup)
        {
            startupPage_.OnShown();
        }
        else if (page == Page::Services)
        {
            servicesPage_.OnShown();
        }
        else
        {
            dashboardPage_.OnShown();
        }
    }

    void MainShell::RefreshRealtimeStats()
    {
        dashboardPage_.RefreshRealtimeStats();
    }

    bool MainShell::OnKeyDown(unsigned int virtualKey)
    {
        if (confirmDialog_ != nullptr &&
            confirmDialog_->OnKeyDown(virtualKey))
        {
            return true;
        }

        return dashboardRoot_ != nullptr &&
               dashboardRoot_->IsVisible() &&
               dashboardPage_.OnKeyDown(virtualKey);
    }

    //
    // LAYOUT
    //

    void MainShell::Resize(
        float width,
        float height) noexcept
    {
        width_ = width;
        height_ = height;

        Layout();
    }

    void MainShell::Layout()
    {
        if (sidebar_ == nullptr)
        {
            return;
        }

        const float width = std::max(width_, 1000.0F);
        const float height = std::max(height_, 600.0F);

        //
        // Barra lateral
        //

        Place(sidebar_, 0.0F, 0.0F, SidebarWidth, height);
        Place(brandLogo_, 24.0F, 28.0F, 36.0F, 36.0F);
        Place(brandText_, 72.0F, 28.0F, 140.0F, 36.0F);

        for (std::size_t i = 0; i < navButtons_.size(); ++i)
        {
            Place(
                navButtons_[i],
                16.0F,
                96.0F + static_cast<float>(i) * 48.0F,
                200.0F,
                44.0F);
        }

        const float chipY = height - 16.0F - 84.0F;

        Place(machineChip_, 16.0F, chipY, 200.0F, 84.0F);
        Place(machineLabel_, 30.0F, chipY + 12.0F, 172.0F, 16.0F);
        Place(machineName_, 30.0F, chipY + 30.0F, 172.0F, 22.0F);
        Place(machineSystem_, 30.0F, chipY + 54.0F, 172.0F, 18.0F);

        //
        // Área do conteúdo
        //

        const Rect area{
            SidebarWidth + PageMarginX,
            PageMarginTop,
            width - SidebarWidth - PageMarginX * 2.0F,
            height - PageMarginTop - PageMarginBottom};

        dashboardRoot_->SetBounds(area);
        cleanerRoot_->SetBounds(area);
        startupRoot_->SetBounds(area);
        servicesRoot_->SetBounds(area);

        dashboardPage_.Layout(area);
        cleanerPage_.Layout(area);
        startupPage_.Layout(area);
        servicesPage_.Layout(area);

        if (confirmDialog_ != nullptr)
        {
            confirmDialog_->Layout(width, height);
        }

        // Seletor de tema: canto superior direito.
        const float toggleX = area.x + area.width - 180.0F;

        Place(toggleSurface_, toggleX, area.y + 2.0F, 180.0F, 44.0F);
        Place(darkButton_, toggleX + 4.0F, area.y + 6.0F, 84.0F, 36.0F);
        Place(lightButton_, toggleX + 92.0F, area.y + 6.0F, 84.0F, 36.0F);
    }
}
