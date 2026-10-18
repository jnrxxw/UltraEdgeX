#include "features/startup/StartupPage.h"

#include "roui/controls/Button.h"
#include "roui/controls/Checkbox.h"
#include "roui/controls/ConfirmDialog.h"
#include "roui/controls/ScrollPanel.h"
#include "roui/controls/SurfacePanel.h"
#include "roui/controls/Text.h"

#include <algorithm>
#include <cwctype>
#include <memory>
#include <string>
#include <utility>

using namespace real_optimizer::roui;

namespace
{
    // Espaço reservado à direita do cabeçalho (seletor de tema).
    constexpr float HeaderActionReserve = 196.0F;

    constexpr float RowHeight = 68.0F;
    constexpr float FooterHeight = 92.0F;

    // A caixa de confirmação mostra no máximo 16 linhas.
    constexpr std::size_t MaxDialogRows = ConfirmDialog::MaxItems;

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

    std::wstring Plural(
        std::size_t count,
        const wchar_t *singular,
        const wchar_t *plural)
    {
        return std::to_wstring(count) + L" " +
               (count == 1 ? singular : plural);
    }

    std::wstring Shorten(const std::wstring &text, std::size_t maxLength)
    {
        if (text.size() <= maxLength)
        {
            return text;
        }

        return text.substr(0, maxLength - 1) + L"…";
    }

    std::wstring Lower(std::wstring text)
    {
        for (wchar_t &c : text)
        {
            c = static_cast<wchar_t>(std::towlower(c));
        }

        return text;
    }

    std::wstring SourceText(real_optimizer::system::StartupSource source)
    {
        switch (source)
        {
        case real_optimizer::system::StartupSource::CurrentUser:
            return L"Usuário atual";
        case real_optimizer::system::StartupSource::LocalMachine:
            return L"Todos os usuários";
        case real_optimizer::system::StartupSource::PackagedApp:
            return L"App da Microsoft Store";
        case real_optimizer::system::StartupSource::LocalMachine32:
        default:
            return L"Todos os usuários (32 bits)";
        }
    }

    // Apps da Loja: mostra o nome do pacote (parte antes do "_"), para
    // dar para saber de qual pacote é cada linha.
    std::wstring SourceLabel(const real_optimizer::system::StartupApp &app)
    {
        if (app.source != real_optimizer::system::StartupSource::PackagedApp)
        {
            return SourceText(app.source);
        }

        std::size_t end = app.name.find(L'_');

        if (end == std::wstring::npos)
        {
            end = app.name.find(L'\\');
        }

        const std::wstring package =
            end == std::wstring::npos ? app.name : app.name.substr(0, end);

        return L"Loja · " + Shorten(package, 26);
    }

    bool NeedsAdministrator(const real_optimizer::system::StartupApp &app)
    {
        return real_optimizer::system::StartupManager::
            RequiresAdministrator(app.source);
    }
}

namespace real_optimizer::features::startup
{
    bool StartupPage::Initialize(
        Panel &pageRoot,
        StartupController &controller,
        ConfirmDialog &confirmDialog)
    {
        controller_ = &controller;
        confirmDialog_ = &confirmDialog;

        titleText_ = AddText(
            pageRoot,
            L"Inicialização",
            28.0F,
            FontWeight::Bold,
            ColorRole::Primary);

        subtitleText_ = AddText(
            pageRoot,
            L"Escolha quais aplicativos abrem junto com o Windows",
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
            L"Lendo os aplicativos de inicialização…",
            13.0F,
            FontWeight::Normal,
            ColorRole::Secondary,
            HorizontalAlignment::Left,
            VerticalAlignment::Top);

        recommendButton_ = AddButton(
            pageRoot,
            L"Marcar recomendados",
            Button::Style::Secondary);

        recommendButton_->SetEnabled(false);
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

        emptyText_ = AddText(
            *listSurface_,
            L"",
            14.0F,
            FontWeight::Normal,
            ColorRole::Muted,
            HorizontalAlignment::Center);

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

        controller.onRefreshFinished =
            [this](std::vector<system::StartupApp> apps)
        { HandleRefreshFinished(std::move(apps)); };

        controller.onApplyStarted = [this]()
        { HandleApplyStarted(); };

        controller.onApplyFinished =
            [this](const ApplySummary &summary)
        { HandleApplyFinished(summary); };

        UpdateSummary();

        return true;
    }

    void StartupPage::OnShown()
    {
        if (controller_ != nullptr && !controller_->IsBusy())
        {
            keepResultMessage_ = false;
            controller_->StartRefresh();
        }
    }

    //
    // LINHAS DA LISTA
    //

    void StartupPage::RebuildRows(std::vector<system::StartupApp> apps)
    {
        scroll_->ClearChildren();
        rows_.clear();

        rows_.reserve(apps.size());

        for (system::StartupApp &app : apps)
        {
            Row row;
            row.info = system::StartupCatalog::Describe(app);
            row.app = std::move(app);
            rows_.push_back(std::move(row));
        }

        // Ativos primeiro; dentro de cada grupo, por nome.
        std::stable_sort(
            rows_.begin(),
            rows_.end(),
            [](const Row &a, const Row &b)
            {
                if (a.app.enabled != b.app.enabled)
                {
                    return a.app.enabled;
                }

                return Lower(a.info.displayName) <
                       Lower(b.info.displayName);
            });

        for (std::size_t i = 0; i < rows_.size(); ++i)
        {
            Row &row = rows_[i];

            const bool safe =
                row.info.safety == system::SafetyLevel::Safe;

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
                toggle->SetChecked(row.app.enabled);

                // Controlado pelo sistema (política ou pelo próprio app).
                toggle->SetEnabled(!row.app.locked);
                toggle->SetOnChanged(
                    [this]()
                    {
                        UpdateSummary();
                    });

                scroll_->AddChild(std::move(toggle));
            }

            row.name = AddText(
                *scroll_,
                Shorten(row.info.displayName, 48),
                14.0F,
                FontWeight::SemiBold,
                row.app.enabled ? ColorRole::Primary : ColorRole::Muted);

            row.description = AddText(
                *scroll_,
                row.info.description,
                12.0F,
                FontWeight::Normal,
                ColorRole::Secondary,
                HorizontalAlignment::Left,
                VerticalAlignment::Top);

            row.state = AddText(
                *scroll_,
                row.app.enabled ? L"Ativo" : L"Desativado",
                14.0F,
                FontWeight::SemiBold,
                row.app.enabled ? ColorRole::Accent : ColorRole::Muted,
                HorizontalAlignment::Right);

            row.note = AddText(
                *scroll_,
                row.app.locked
                    ? (row.app.lockReason.empty()
                           ? std::wstring(L"Controlado pelo sistema")
                           : row.app.lockReason)
                    : SourceLabel(row.app),
                11.0F,
                FontWeight::Normal,
                ColorRole::Muted,
                HorizontalAlignment::Right);

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
    }

    //
    // AÇÕES DO USUÁRIO
    //

    void StartupPage::RequestRefresh()
    {
        if (controller_ == nullptr || controller_->IsBusy())
        {
            return;
        }

        keepResultMessage_ = false;
        controller_->StartRefresh();
    }

    void StartupPage::MarkRecommended()
    {
        if (controller_ == nullptr || controller_->IsBusy())
        {
            return;
        }

        std::size_t marked = 0;

        for (Row &row : rows_)
        {
            if (system::StartupCatalog::IsRecommendedToDisable(row.app))
            {
                row.toggle->SetChecked(false);
                ++marked;
            }
        }

        SetStatus(
            marked == 0
                ? L"Nenhum aplicativo de nível Seguro está ativo agora."
                : L"Marquei para desativar os aplicativos de nível Seguro. Revise a lista e clique em Aplicar alterações.");

        UpdateSummary();
    }

    void StartupPage::CollectPending(
        std::vector<std::size_t> &toDisable,
        std::vector<std::size_t> &toEnable) const
    {
        for (std::size_t i = 0; i < rows_.size(); ++i)
        {
            const Row &row = rows_[i];

            const bool wantOn = row.toggle->IsChecked();

            if (row.app.enabled && !wantOn)
            {
                toDisable.push_back(i);
            }
            else if (!row.app.enabled && wantOn)
            {
                toEnable.push_back(i);
            }
        }
    }

    void StartupPage::RequestApply()
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

        ConfirmDialogContent content;

        bool hasAttention = false;
        bool needsAdmin = false;

        std::vector<system::StartupTarget> disableTargets;
        std::vector<system::StartupTarget> enableTargets;

        std::vector<std::pair<std::wstring, std::wstring>> lines;

        for (const std::size_t index : toDisable)
        {
            const Row &row = rows_[index];

            lines.push_back({row.info.displayName, L"Desativar"});
            disableTargets.push_back({row.app.source, row.app.name});

            if (row.info.safety == system::SafetyLevel::Attention)
            {
                hasAttention = true;
            }

            needsAdmin = needsAdmin || NeedsAdministrator(row.app);
        }

        for (const std::size_t index : toEnable)
        {
            const Row &row = rows_[index];

            lines.push_back({row.info.displayName, L"Reativar"});
            enableTargets.push_back({row.app.source, row.app.name});

            needsAdmin = needsAdmin || NeedsAdministrator(row.app);
        }

        // A caixa mostra no máximo 16 linhas.
        for (std::size_t i = 0; i < lines.size(); ++i)
        {
            if (lines.size() > MaxDialogRows &&
                i == MaxDialogRows - 1)
            {
                content.items.push_back(
                    {L"e mais " +
                         std::to_wstring(lines.size() - i) +
                         L" aplicativos",
                     L"",
                     true});
                break;
            }

            content.items.push_back({lines[i].first, lines[i].second, false});
        }

        content.title = L"Aplicar alterações na inicialização";
        content.message = L"Estas mudanças serão feitas no Windows:";

        if (hasAttention)
        {
            content.notices.push_back(
                {L"Inclui aplicativos de nível Atenção. Leia a descrição de cada um: desativar pode tirar recursos do app ou do PC.",
                 true});
        }

        if (needsAdmin && !controller_->IsElevated())
        {
            content.notices.push_back(
                {L"Aplicativos de todos os usuários exigem permissão de administrador. O Windows vai pedir.",
                 false});
        }

        content.notices.push_back(
            {L"Nenhum aplicativo é apagado. Para voltar atrás, é só reativar o interruptor e aplicar de novo.",
             false});

        content.confirmText = L"Aplicar";
        content.cancelText = L"Cancelar";

        confirmDialog_->Show(
            std::move(content),
            [this,
             disableTargets = std::move(disableTargets),
             enableTargets = std::move(enableTargets)]()
            {
                if (controller_ != nullptr)
                {
                    controller_->StartApply(disableTargets, enableTargets);
                }
            });
    }

    //
    // EVENTOS DO CONTROLADOR
    //

    void StartupPage::HandleRefreshStarted()
    {
        busy_ = true;

        if (!keepResultMessage_)
        {
            SetStatus(L"Lendo os aplicativos de inicialização…");
        }

        UpdateButtons();
    }

    void StartupPage::HandleRefreshFinished(
        std::vector<system::StartupApp> apps)
    {
        busy_ = false;
        loaded_ = true;

        RebuildRows(std::move(apps));

        std::size_t active = 0;

        for (const Row &row : rows_)
        {
            if (row.app.enabled)
            {
                ++active;
            }
        }

        if (!keepResultMessage_)
        {
            if (rows_.empty())
            {
                SetStatus(L"Nenhum aplicativo encontrado na lista de inicialização.");
            }
            else
            {
                SetStatus(
                    std::to_wstring(active) + L" de " +
                    std::to_wstring(rows_.size()) +
                    L" aplicativos abrem junto com o Windows. Desativar não apaga nada: o aplicativo só deixa de abrir sozinho.");
            }
        }

        emptyText_->SetText(
            rows_.empty()
                ? L"Nenhum aplicativo registrado para iniciar com o Windows."
                : L"");

        UpdateSummary();
        UpdateButtons();

        Layout(area_);
    }

    void StartupPage::HandleApplyStarted()
    {
        busy_ = true;

        SetStatus(L"Aplicando… isso pode levar alguns segundos.");

        applyButton_->SetText(L"Aplicando…");

        UpdateButtons();
    }

    void StartupPage::HandleApplyFinished(
        const ApplySummary &summary)
    {
        busy_ = false;

        std::wstring message;

        if (summary.changed > 0)
        {
            message =
                L"Pronto: " +
                Plural(summary.changed,
                       L"aplicativo alterado.",
                       L"aplicativos alterados.");
        }
        else
        {
            message = L"Nenhum aplicativo foi alterado.";
        }

        if (summary.notFound > 0)
        {
            message +=
                L" " +
                Plural(summary.notFound,
                       L"não existe mais na lista.",
                       L"não existem mais na lista.");
        }

        if (summary.failed > 0)
        {
            message +=
                L" " + Plural(summary.failed, L"falhou.", L"falharam.");
        }

        if (summary.elevationCancelled)
        {
            message += L" A permissão de administrador foi recusada.";
        }
        else if (summary.elevationFailed)
        {
            message += L" Não foi possível concluir como administrador.";
        }

        if (summary.changed > 0)
        {
            message += L" A mudança vale a partir da próxima vez que o PC iniciar.";
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

    void StartupPage::SetStatus(const std::wstring &text)
    {
        if (statusText_ != nullptr)
        {
            statusText_->SetText(text);
        }
    }

    void StartupPage::UpdateSummary()
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
            L"A lista mostra aplicativos do Registro (chaves Run) e da Microsoft Store. Itens da pasta Inicializar e tarefas agendadas não aparecem aqui.";

        if (controller_ != nullptr && !controller_->IsElevated())
        {
            note += L" Aplicativos de todos os usuários pedem permissão de administrador ao aplicar.";
        }

        footerNote_->SetText(note);

        UpdateButtons();
    }

    void StartupPage::UpdateButtons()
    {
        refreshButton_->SetEnabled(!busy_);
        recommendButton_->SetEnabled(!busy_ && loaded_ && !rows_.empty());

        applyButton_->SetEnabled(!busy_ && pendingCount_ > 0);
    }

    //
    // LAYOUT
    //

    void StartupPage::Layout(const Rect &area)
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

        // O texto de status pode ocupar duas linhas.
        Place(statusText_, area.x, y, area.width - 210.0F, 36.0F);
        Place(recommendButton_, area.x + area.width - 190.0F, y, 190.0F, 32.0F);

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

        Place(
            emptyText_,
            viewport.x,
            viewport.y,
            viewport.width,
            viewport.height);

        const float offset = scroll_->GetScrollOffset();

        const float badgeWidth = 72.0F;
        const float stateWidth = 150.0F;

        // Margem direita maior para o texto não encostar na barra
        // de rolagem (que fica na borda da lista).
        const float rightInset = 48.0F;

        for (std::size_t i = 0; i < rows_.size(); ++i)
        {
            Row &row = rows_[i];

            const float rowY =
                viewport.y + static_cast<float>(i) * RowHeight - offset;

            Place(row.divider, area.x + 24.0F, rowY, area.width - 48.0F, 1.0F);

            // Todo o retângulo da linha é clicável.
            Place(row.toggle, area.x + 8.0F, rowY + 2.0F, area.width - 16.0F, RowHeight - 4.0F);

            const float stateX = area.x + area.width - rightInset - stateWidth;
            const float badgeX = stateX - 16.0F - badgeWidth;
            const float textX = area.x + 80.0F;
            const float textWidth = badgeX - 16.0F - textX;

            Place(row.name, textX, rowY + 8.0F, textWidth, 20.0F);
            Place(row.description, textX, rowY + 30.0F, textWidth, 34.0F);

            Place(row.badge, badgeX, rowY + (RowHeight - 22.0F) / 2.0F, badgeWidth, 22.0F);
            Place(row.badgeText, badgeX, rowY + (RowHeight - 22.0F) / 2.0F, badgeWidth, 22.0F);

            Place(row.state, stateX, rowY + 14.0F, stateWidth, 22.0F);
            Place(row.note, stateX - 40.0F, rowY + 38.0F, stateWidth + 40.0F, 16.0F);
        }

        const float footerY = y + listHeight - FooterHeight;

        Place(footerDivider_, area.x + 24.0F, footerY, area.width - 48.0F, 1.0F);
        Place(summaryText_, area.x + 24.0F, footerY + 12.0F, area.width - 500.0F, 22.0F);
        Place(footerNote_, area.x + 24.0F, footerY + 36.0F, area.width - 300.0F, 52.0F);

        Place(
            applyButton_,
            area.x + area.width - 24.0F - 190.0F,
            footerY + 24.0F,
            190.0F,
            44.0F);
    }
}
