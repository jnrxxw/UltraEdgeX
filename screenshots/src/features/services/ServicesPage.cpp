#include "features/services/ServicesPage.h"

#include "roui/controls/Button.h"
#include "roui/controls/Checkbox.h"
#include "roui/controls/ConfirmDialog.h"
#include "roui/controls/ScrollPanel.h"
#include "roui/controls/SurfacePanel.h"
#include "roui/controls/Text.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

using namespace real_optimizer::roui;

namespace
{
    // Espaço reservado à direita do cabeçalho (seletor de tema).
    constexpr float HeaderActionReserve = 196.0F;

    constexpr float RowHeight = 58.0F;
    constexpr float FooterHeight = 76.0F;

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

    std::wstring StartTypeText(real_optimizer::system::ServiceStartType type)
    {
        switch (type)
        {
        case real_optimizer::system::ServiceStartType::Automatic:
            return L"Automático";
        case real_optimizer::system::ServiceStartType::AutomaticDelayed:
            return L"Automático (atrasado)";
        case real_optimizer::system::ServiceStartType::Manual:
            return L"Manual";
        case real_optimizer::system::ServiceStartType::Disabled:
            return L"Desativado";
        case real_optimizer::system::ServiceStartType::Unknown:
        default:
            return L"";
        }
    }

    std::wstring Plural(
        std::size_t count,
        const wchar_t *singular,
        const wchar_t *plural)
    {
        return std::to_wstring(count) + L" " +
               (count == 1 ? singular : plural);
    }
}

namespace real_optimizer::features::services
{
    bool ServicesPage::Initialize(
        Panel &pageRoot,
        ServicesController &controller,
        ConfirmDialog &confirmDialog)
    {
        controller_ = &controller;
        confirmDialog_ = &confirmDialog;

        titleText_ = AddText(
            pageRoot,
            L"Serviços",
            28.0F,
            FontWeight::Bold,
            ColorRole::Primary);

        subtitleText_ = AddText(
            pageRoot,
            L"Desative serviços do Windows que você não usa",
            14.0F,
            FontWeight::Normal,
            ColorRole::Secondary);

        refreshButton_ = AddButton(
            pageRoot,
            L"Atualizar",
            Button::Style::Primary);

        refreshButton_->SetOnClick(
            [this]()
            {
                RequestRefresh();
            });

        statusText_ = AddText(
            pageRoot,
            L"Lendo o estado dos serviços…",
            13.0F,
            FontWeight::Normal,
            ColorRole::Secondary);

        recommendButton_ = AddButton(
            pageRoot,
            L"Marcar recomendados",
            Button::Style::Secondary);

        recommendButton_->SetOnClick(
            [this]()
            {
                MarkRecommended();
            });

        listSurface_ = AddSurface(
            pageRoot,
            SurfaceRole::Surface,
            16.0F);

        {
            auto scroll = std::make_unique<ScrollPanel>();
            scroll_ = scroll.get();

            scroll_->SetOnScroll(
                [this]()
                {
                    Layout(area_);
                });

            listSurface_->AddChild(std::move(scroll));
        }

        const auto &definitions = controller.Definitions();

        rows_.resize(definitions.size());

        for (std::size_t i = 0; i < definitions.size(); ++i)
        {
            const auto &definition = definitions[i];
            Row &row = rows_[i];

            if (i > 0)
            {
                row.divider = AddSurface(
                    *scroll_,
                    SurfaceRole::Divider,
                    0.0F);
            }

            {
                auto toggle = std::make_unique<Checkbox>();
                row.toggle = toggle.get();

                toggle->SetStyle(Checkbox::Style::Switch);
                toggle->SetEnabled(false);
                toggle->SetOnChanged(
                    [this]()
                    {
                        UpdateSummary();
                    });

                scroll_->AddChild(std::move(toggle));
            }

            row.name = AddText(
                *scroll_,
                definition.displayName,
                14.0F,
                FontWeight::SemiBold,
                ColorRole::Muted);

            row.description = AddText(
                *scroll_,
                definition.description,
                12.0F,
                FontWeight::Normal,
                ColorRole::Secondary);

            row.state = AddText(
                *scroll_,
                L"—",
                14.0F,
                FontWeight::SemiBold,
                ColorRole::Muted,
                HorizontalAlignment::Right);

            row.note = AddText(
                *scroll_,
                L"",
                11.0F,
                FontWeight::Normal,
                ColorRole::Muted,
                HorizontalAlignment::Right);

            const bool safe =
                definition.safety == system::SafetyLevel::Safe;

            row.badge = AddSurface(
                *scroll_,
                safe
                    ? SurfaceRole::SuccessSoft
                    : SurfaceRole::WarningSoft,
                11.0F);

            row.badgeText = AddText(
                *row.badge,
                safe ? L"Seguro" : L"Atenção",
                11.0F,
                FontWeight::SemiBold,
                safe
                    ? ColorRole::Success
                    : ColorRole::Warning,
                HorizontalAlignment::Center);
        }

        footerDivider_ = AddSurface(
            *listSurface_,
            SurfaceRole::Divider,
            0.0F);

        summaryText_ = AddText(
            *listSurface_,
            L"Nenhuma alteração pendente",
            14.0F,
            FontWeight::SemiBold,
            ColorRole::Primary);

        footerNote_ = AddText(
            *listSurface_,
            L"",
            12.0F,
            FontWeight::Normal,
            ColorRole::Muted,
            HorizontalAlignment::Left,
            VerticalAlignment::Top);

        restoreButton_ = AddButton(
            *listSurface_,
            L"Restaurar padrão",
            Button::Style::Secondary);

        restoreButton_->SetEnabled(false);
        restoreButton_->SetOnClick(
            [this]()
            {
                RequestRestore();
            });

        applyButton_ = AddButton(
            *listSurface_,
            L"Aplicar alterações",
            Button::Style::Primary);

        applyButton_->SetEnabled(false);
        applyButton_->SetOnClick(
            [this]()
            {
                RequestApply();
            });

        // Eventos do controlador.
        controller.onRefreshStarted = [this]()
        { HandleRefreshStarted(); };

        controller.onServiceLoaded =
            [this](std::size_t index, const system::ServiceState &state)
        { HandleServiceLoaded(index, state); };

        controller.onRefreshFinished = [this]()
        { HandleRefreshFinished(); };

        controller.onApplyStarted = [this]()
        { HandleApplyStarted(); };

        controller.onApplyFinished =
            [this](const ApplySummary &summary)
        { HandleApplyFinished(summary); };

        UpdateSummary();

        return true;
    }

    void ServicesPage::OnShown()
    {
        if (controller_ != nullptr && !controller_->IsBusy())
        {
            keepResultMessage_ = false;
            controller_->StartRefresh();
        }
    }

    //
    // AÇÕES DO USUÁRIO
    //

    void ServicesPage::RequestRefresh()
    {
        if (controller_ == nullptr || controller_->IsBusy())
        {
            return;
        }

        keepResultMessage_ = false;
        controller_->StartRefresh();
    }

    void ServicesPage::MarkRecommended()
    {
        if (controller_ == nullptr || controller_->IsBusy())
        {
            return;
        }

        const auto &definitions = controller_->Definitions();

        std::size_t marked = 0;

        for (std::size_t i = 0; i < rows_.size(); ++i)
        {
            Row &row = rows_[i];

            if (system::Services::IsRecommendedToDisable(
                    definitions[i],
                    row.service))
            {
                row.toggle->SetChecked(false);
                ++marked;
            }
        }

        SetStatus(
            marked == 0
                ? L"Os serviços de nível Seguro já estão desativados ou não existem neste Windows."
                : L"Marquei para desativar os serviços de nível Seguro. Revise a lista e clique em Aplicar alterações.");

        UpdateSummary();
    }

    void ServicesPage::CollectPending(
        std::vector<std::size_t> &toDisable,
        std::vector<std::size_t> &toEnable) const
    {
        for (std::size_t i = 0; i < rows_.size(); ++i)
        {
            const Row &row = rows_[i];

            if (!row.present)
            {
                continue;
            }

            const bool wantOn = row.toggle->IsChecked();

            const bool isOn =
                row.service.startType !=
                system::ServiceStartType::Disabled;

            if (isOn && !wantOn)
            {
                toDisable.push_back(i);
            }
            else if (!isOn && wantOn)
            {
                toEnable.push_back(i);
            }
        }
    }

    void ServicesPage::RequestApply()
    {
        if (
            controller_ == nullptr ||
            confirmDialog_ == nullptr ||
            controller_->IsBusy() ||
            confirmDialog_->IsOpen())
        {
            return;
        }

        std::vector<std::size_t> toDisable;
        std::vector<std::size_t> toEnable;

        CollectPending(toDisable, toEnable);

        if (toDisable.empty() && toEnable.empty())
        {
            return;
        }

        const auto &definitions = controller_->Definitions();

        ConfirmDialogContent content;
        bool hasAttention = false;

        for (const std::size_t index : toDisable)
        {
            content.items.push_back(
                {definitions[index].displayName, L"Desativar", false});

            if (definitions[index].safety ==
                system::SafetyLevel::Attention)
            {
                hasAttention = true;
            }
        }

        for (const std::size_t index : toEnable)
        {
            content.items.push_back(
                {definitions[index].displayName, L"Reativar", false});
        }

        content.title = L"Aplicar alterações nos serviços";
        content.message = L"Estas mudanças serão feitas no Windows:";

        if (hasAttention)
        {
            content.notices.push_back(
                {L"Inclui serviços de nível Atenção. Desativar pode tirar funções do Windows; leia a descrição de cada um.",
                 true});
        }

        if (!controller_->IsElevated())
        {
            content.notices.push_back(
                {L"O Windows vai pedir permissão de administrador.",
                 false});
        }

        content.notices.push_back(
            {L"Você pode voltar ao estado original a qualquer momento em Restaurar padrão.",
             false});

        content.footnote =
            L"Serviços que outros serviços ativos usam não serão alterados.";

        content.confirmText = L"Aplicar";
        content.cancelText = L"Cancelar";

        confirmDialog_->Show(
            std::move(content),
            [this,
             toDisable = std::move(toDisable),
             toEnable = std::move(toEnable)]()
            {
                if (controller_ != nullptr)
                {
                    controller_->StartApply(toDisable, toEnable);
                }
            });
    }

    void ServicesPage::RequestRestore()
    {
        if (
            controller_ == nullptr ||
            confirmDialog_ == nullptr ||
            controller_->IsBusy() ||
            confirmDialog_->IsOpen())
        {
            return;
        }

        const std::vector<std::wstring> names =
            controller_->BackedUpNames();

        if (names.empty())
        {
            return;
        }

        ConfirmDialogContent content;

        for (const std::wstring &name : names)
        {
            const system::ServiceDefinition *definition =
                system::Services::Find(name);

            content.items.push_back(
                {definition != nullptr ? definition->displayName : name,
                 L"Restaurar",
                 false});
        }

        content.title = L"Restaurar padrão";
        content.message =
            L"Voltar estes serviços à configuração que tinham antes do UeX?";

        if (!controller_->IsElevated())
        {
            content.notices.push_back(
                {L"O Windows vai pedir permissão de administrador.",
                 false});
        }

        content.confirmText = L"Restaurar";
        content.cancelText = L"Cancelar";

        confirmDialog_->Show(
            std::move(content),
            [this]()
            {
                if (controller_ != nullptr)
                {
                    controller_->StartRestore();
                }
            });
    }

    //
    // EVENTOS DO CONTROLADOR
    //

    void ServicesPage::HandleRefreshStarted()
    {
        busy_ = true;

        if (!keepResultMessage_)
        {
            SetStatus(L"Lendo o estado dos serviços…");
        }

        UpdateButtons();
    }

    void ServicesPage::HandleServiceLoaded(
        std::size_t index,
        const system::ServiceState &state)
    {
        if (index >= rows_.size())
        {
            return;
        }

        Row &row = rows_[index];

        row.service = state;
        row.present = state.installed && state.readable;

        row.toggle->SetEnabled(row.present);

        row.toggle->SetChecked(
            row.present &&
            state.startType != system::ServiceStartType::Disabled);

        row.name->SetColorRole(
            row.present ? ColorRole::Primary : ColorRole::Muted);

        if (!state.installed)
        {
            row.state->SetText(L"Não instalado");
            row.state->SetColorRole(ColorRole::Muted);
            row.note->SetText(L"");
        }
        else if (!state.readable)
        {
            row.state->SetText(L"—");
            row.state->SetColorRole(ColorRole::Muted);
            row.note->SetText(L"Sem permissão para ler");
        }
        else
        {
            row.state->SetText(state.running ? L"Rodando" : L"Parado");
            row.state->SetColorRole(
                state.running ? ColorRole::Accent : ColorRole::Muted);
            row.note->SetText(StartTypeText(state.startType));
        }

        UpdateSummary();
    }

    void ServicesPage::HandleRefreshFinished()
    {
        busy_ = false;

        std::size_t present = 0;
        std::size_t running = 0;

        for (const Row &row : rows_)
        {
            if (row.present)
            {
                ++present;

                if (row.service.running)
                {
                    ++running;
                }
            }
        }

        if (!keepResultMessage_)
        {
            SetStatus(
                std::to_wstring(running) + L" de " +
                std::to_wstring(present) +
                L" serviços da lista estão rodando agora. Muitos já ficam parados em Manual, então desativar nem sempre muda o uso de memória.");
        }

        hasBackup_ =
            controller_ != nullptr &&
            !controller_->BackedUpNames().empty();

        UpdateSummary();
        UpdateButtons();
    }

    void ServicesPage::HandleApplyStarted()
    {
        busy_ = true;

        SetStatus(L"Aplicando… isso pode levar alguns segundos.");

        applyButton_->SetText(L"Aplicando…");

        UpdateButtons();
    }

    void ServicesPage::HandleApplyFinished(
        const ApplySummary &summary)
    {
        busy_ = false;

        std::wstring message;

        if (summary.changed > 0)
        {
            message =
                L"Pronto: " +
                Plural(summary.changed, L"serviço alterado.", L"serviços alterados.");
        }
        else
        {
            message = L"Nenhum serviço foi alterado.";
        }

        if (summary.inUse > 0)
        {
            message +=
                L" " +
                Plural(summary.inUse,
                       L"não foi alterado porque outros serviços dependem dele.",
                       L"não foram alterados porque outros serviços dependem deles.");
        }

        if (summary.failed > 0)
        {
            message +=
                L" " + Plural(summary.failed, L"falhou.", L"falharam.");
        }

        if (summary.elevationCancelled)
        {
            message +=
                L" A permissão de administrador foi recusada.";
        }
        else if (summary.elevationFailed)
        {
            message +=
                L" Não foi possível concluir como administrador.";
        }

        SetStatus(message);

        applyButton_->SetText(L"Aplicar alterações");

        // Atualiza o estado mantendo a mensagem do resultado.
        keepResultMessage_ = true;

        if (controller_ != nullptr)
        {
            controller_->StartRefresh();
        }

        UpdateButtons();
    }

    //
    // ATUALIZAÇÃO DA TELA
    //

    void ServicesPage::SetStatus(const std::wstring &text)
    {
        if (statusText_ != nullptr)
        {
            statusText_->SetText(text);
        }
    }

    void ServicesPage::UpdateSummary()
    {
        std::vector<std::size_t> toDisable;
        std::vector<std::size_t> toEnable;

        CollectPending(toDisable, toEnable);

        pendingCount_ = toDisable.size() + toEnable.size();

        if (pendingCount_ == 0)
        {
            summaryText_->SetText(L"Nenhuma alteração pendente");
        }
        else
        {
            std::wstring text = Plural(
                pendingCount_,
                L"alteração pendente",
                L"alterações pendentes");

            text += L" (";

            if (!toDisable.empty())
            {
                text += std::to_wstring(toDisable.size()) +
                        L" desativar";
            }

            if (!toEnable.empty())
            {
                text += (toDisable.empty() ? L"" : L", ") +
                        std::to_wstring(toEnable.size()) + L" reativar";
            }

            text += L")";

            summaryText_->SetText(text);
        }

        std::wstring note =
            L"Alguns serviços só mudam por completo depois de reiniciar o PC.";

        if (controller_ != nullptr && !controller_->IsElevated())
        {
            note += L" O Windows vai pedir permissão de administrador ao aplicar.";
        }

        footerNote_->SetText(note);

        UpdateButtons();
    }

    void ServicesPage::UpdateButtons()
    {
        refreshButton_->SetEnabled(!busy_);
        recommendButton_->SetEnabled(!busy_);

        applyButton_->SetEnabled(!busy_ && pendingCount_ > 0);
        restoreButton_->SetEnabled(!busy_ && hasBackup_);
    }

    //
    // LAYOUT
    //

    void ServicesPage::Layout(const Rect &area)
    {
        area_ = area;

        if (titleText_ == nullptr)
        {
            return;
        }

        float y = area.y;

        Place(titleText_, area.x, y, 500.0F, 36.0F);
        Place(subtitleText_, area.x, y + 38.0F, 600.0F, 20.0F);

        Place(
            refreshButton_,
            area.x + area.width - HeaderActionReserve - 180.0F,
            y,
            180.0F,
            48.0F);

        y += 52.0F + 12.0F;

        Place(statusText_, area.x, y, area.width - 210.0F, 36.0F);
        Place(recommendButton_, area.x + area.width - 190.0F, y, 190.0F, 32.0F);

        // O texto de status pode ocupar duas linhas.
        statusText_->SetVerticalAlignment(VerticalAlignment::Top);

        y += 36.0F + 8.0F;

        const float listHeight =
            std::max(260.0F, area.y + area.height - y);

        Place(listSurface_, area.x, y, area.width, listHeight);

        const Rect viewport{
            area.x + 8.0F,
            y + 6.0F,
            area.width - 16.0F,
            listHeight - FooterHeight - 6.0F};

        scroll_->SetBounds(viewport);

        scroll_->SetContentHeight(
            static_cast<float>(rows_.size()) * RowHeight);

        const float offset = scroll_->GetScrollOffset();

        const float badgeWidth = 72.0F;
        const float stateWidth = 130.0F;

        for (std::size_t i = 0; i < rows_.size(); ++i)
        {
            Row &row = rows_[i];

            const float rowY =
                viewport.y + static_cast<float>(i) * RowHeight - offset;

            Place(row.divider, area.x + 24.0F, rowY, area.width - 48.0F, 1.0F);

            // Todo o retângulo da linha é clicável.
            Place(row.toggle, area.x + 8.0F, rowY + 2.0F, area.width - 16.0F, RowHeight - 4.0F);

            const float stateX = area.x + area.width - 24.0F - stateWidth;
            const float badgeX = stateX - 16.0F - badgeWidth;
            const float textX = area.x + 80.0F;
            const float textWidth = badgeX - 16.0F - textX;

            Place(row.name, textX, rowY + 8.0F, textWidth, 20.0F);
            Place(row.description, textX, rowY + 30.0F, textWidth, 18.0F);

            Place(row.badge, badgeX, rowY + (RowHeight - 22.0F) / 2.0F, badgeWidth, 22.0F);
            Place(row.badgeText, badgeX, rowY + (RowHeight - 22.0F) / 2.0F, badgeWidth, 22.0F);

            Place(row.state, stateX, rowY + 10.0F, stateWidth, 22.0F);
            Place(row.note, stateX - 70.0F, rowY + 33.0F, stateWidth + 70.0F, 16.0F);
        }

        const float footerY = y + listHeight - FooterHeight;

        Place(footerDivider_, area.x + 24.0F, footerY, area.width - 48.0F, 1.0F);
        Place(summaryText_, area.x + 24.0F, footerY + 12.0F, area.width - 500.0F, 22.0F);
        Place(footerNote_, area.x + 24.0F, footerY + 36.0F, area.width - 500.0F, 34.0F);

        Place(
            applyButton_,
            area.x + area.width - 24.0F - 190.0F,
            footerY + 16.0F,
            190.0F,
            44.0F);

        Place(
            restoreButton_,
            area.x + area.width - 24.0F - 190.0F - 12.0F - 160.0F,
            footerY + 16.0F,
            160.0F,
            44.0F);
    }
}
