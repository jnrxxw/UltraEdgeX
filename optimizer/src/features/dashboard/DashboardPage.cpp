#include "features/dashboard/DashboardPage.h"

#include "roui/controls/Panel.h"
#include "roui/layout/StackPanel.h"
#include "roui/controls/Text.h"

#include <memory>

using namespace real_optimizer::roui;

namespace real_optimizer::features::dashboard{

    bool DashboardPage::Initialize(
    Panel& rootPanel
    )
    {
    auto page =
        std::make_unique<StackPanel>();

    page->SetBounds({
        40.0F,
        40.0F,
        900.0F,
        650.0F
    });

    page->SetOrientation(
        Orientation::Vertical
    );

    page->SetSpacing(
        24.0F
    );

    pagePanel_ = page.get();

    rootPanel.AddChild(
        std::move(page)
    );

    if (!CreateHeader())
    {
    return false;
    }

    return true;
    }

    void DashboardPage::Resize(
    float width,
    float height
    ) noexcept
    {
    if (pagePanel_ == nullptr)
    {
        return;
    }

    pagePanel_->SetSize(
        width - 80.0F,
        height - 80.0F
    );
    }


    bool DashboardPage::CreateHeader()
    {
    auto header =
        std::make_unique<StackPanel>();

    header->SetOrientation(
        Orientation::Vertical
    );

    header->SetSpacing(6.0F);

    auto title =
        std::make_unique<Text>();

    title->SetText(
        L"UltraEdgeX"
    );

    title->SetSize(
        700.0F,
        42.0F
    );

    title->SetFontSize(32.0F);

    title->SetFontWeight(
        FontWeight::Bold
    );

    title->SetColor(
        Color::FromRgb(
            7,
            242,
            242
        )
    );

    header->AddChild(
        std::move(title)
    );

    auto subtitle =
        std::make_unique<Text>();

    subtitle->SetText(
        L"Windows Performance & Optimization"
    );

    subtitle->SetSize(
        700.0F,
        24.0F
    );

    subtitle->SetFontSize(15.0F);

    subtitle->SetColor(
        Color::FromRgb(
            180,
            180,
            180
        )
    );

    header->AddChild(
        std::move(subtitle)
    );

    headerPanel_ = header.get();

    pagePanel_->AddChild(
        std::move(header)
    );

    return true;
    }




}