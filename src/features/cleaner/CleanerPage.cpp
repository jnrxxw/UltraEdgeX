#include "features/cleaner/CleanerPage.h"

#include <windows.h>

#include "core/ByteFormat.h"
#include "roui/controls/Button.h"
#include "roui/controls/Checkbox.h"
#include "roui/controls/ConfirmDialog.h"
#include "roui/controls/SurfacePanel.h"
#include "roui/controls/Text.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>

using namespace real_optimizer::roui;
using real_optimizer::core::FormatBytes;

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
}

namespace real_optimizer::features::cleaner
{
    bool CleanerPage::Initialize(
        Panel &pageRoot,
        CleanerController &controller,
        ConfirmDialog &confirmDialog)
    {
        controller_ = &controller;
        confirmDialog_ = &confirmDialog;

        titleText_ = AddText(
            pageRoot,
            L"Limpeza",
            28.0F,
            FontWeight::Bold,
            ColorRole::Primary);

        subtitleText_ = AddText(
            pageRoot,
            L"Encontre e remova arquivos temporários com segurança",
            14.0F,
            FontWeight::Normal,
            ColorRole::Secondary);

        scanButton_ = AddButton(
            pageRoot,
            L"Analisar",
            Button::Style::Primary);

        scanButton_->SetOnClick(
            [this]()
            {
                RequestScan();
            });

        if (!controller.IsElevated())
        {
            adminButton_ = AddButton(
                pageRoot,
                L"Reabrir como admin",
                Button::Style::Secondary);

            adminButton_->SetOnClick(
                [this]()
                {
                    RequestAdministratorRestart();
                });
        }

        statusText_ = AddText(
            pageRoot,
            L"Analise o computador para ver quanto espaço pode ser liberado. Nada é apagado nesta etapa.",
            13.0F,
            FontWeight::Normal,
            ColorRole::Secondary);

        listSurface_ = AddSurface(
            pageRoot,
            SurfaceRole::Surface,
            16.0F);

        const auto &categories = controller.Categories();

        rows_.resize(categories.size());

        for (std::size_t i = 0; i < categories.size(); ++i)
        {
            const auto &category = categories[i];
            Row &row = rows_[i];

            if (i > 0)
            {
                row.divider = AddSurface(
                    *listSurface_,
                    SurfaceRole::Divider,
                    0.0F);
            }

            {
                auto checkbox = std::make_unique<Checkbox>();
                row.checkbox = checkbox.get();

                checkbox->SetEnabled(false);
                checkbox->SetOnChanged(
                    [this]()
                    {
                        UpdateSummary();
                    });

                listSurface_->AddChild(std::move(checkbox));
            }

            row.name = AddText(
                *listSurface_,
                category.name,
                14.0F,
                FontWeight::SemiBold,
                ColorRole::Muted);

            row.description = AddText(
                *listSurface_,
                category.description,
                12.0F,
                FontWeight::Normal,
                ColorRole::Secondary);

            row.size = AddText(
                *listSurface_,
                L"—",
                14.0F,
                FontWeight::SemiBold,
                ColorRole::Muted,
                HorizontalAlignment::Right);

            row.note = AddText(
                *listSurface_,
                L"",
                11.0F,
                FontWeight::Normal,
                ColorRole::Muted,
                HorizontalAlignment::Right);

            const bool safe =
                category.safety == system::SafetyLevel::Safe;

            row.badge = AddSurface(
                *listSurface_,
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
            L"Nenhuma análise feita ainda",
            14.0F,
            FontWeight::SemiBold,
            ColorRole::Primary);

        footerNote_ = AddText(
            *listSurface_,
            L"Arquivos em uso são ignorados. A limpeza não envia nada para a Lixeira.",
            12.0F,
            FontWeight::Normal,
            ColorRole::Muted);

        cleanButton_ = AddButton(
            *listSurface_,
            L"Limpar selecionados",
            Button::Style::Primary);

        cleanButton_->SetEnabled(false);
        cleanButton_->SetOnClick(
            [this]()
            {
                RequestClean();
            });

        // Eventos do controlador.
        controller.onScanStarted = [this]()
        { HandleScanStarted(); };

        controller.onCategoryScanned =
            [this](std::size_t index, const system::ScanResult &result)
        { HandleCategoryScanned(index, result); };

        controller.onScanFinished = [this]()
        { HandleScanFinished(); };

        controller.onCleanStarted = [this]()
        { HandleCleanStarted(); };

        controller.onCleanFinished =
            [this](const CleanSummary &summary)
        { HandleCleanFinished(summary); };

        return true;
    }

    void CleanerPage::OnShown()
    {
        // Na primeira vez que a página abre, já faz a análise
        // (só mede, não apaga nada).
        if (!everScanned_ && controller_ != nullptr &&
            !controller_->IsBusy())
        {
            RequestScan();
        }
    }

    //
    // AÇÕES DO USUÁRIO
    //

    void CleanerPage::RequestScan()
    {
        if (controller_ == nullptr || controller_->IsBusy())
        {
            return;
        }

        keepResultMessage_ = false;
        controller_->StartScan();
    }

    void CleanerPage::RequestAdministratorRestart()
    {
        if (controller_ == nullptr || controller_->IsBusy())
        {
            return;
        }

        // O Windows mostra o pedido de permissão. Se o usuário
        // aceitar, a nova cópia abre na Limpeza e esta é fechada.
        if (controller_->RestartAsAdministrator())
        {
            PostMessageW(GetActiveWindow(), WM_CLOSE, 0, 0);
        }
    }

    void CleanerPage::RequestClean()
    {
        if (
            controller_ == nullptr ||
            confirmDialog_ == nullptr ||
            controller_->IsBusy() ||
            confirmDialog_->IsOpen())
        {
            return;
        }

        const auto &categories = controller_->Categories();

        ConfirmDialogContent content;
        std::vector<std::size_t> indices;

        std::uint64_t totalBytes = 0;
        bool hasAttention = false;
        bool needsAdmin = false;
        bool hasUnknown = false;

        for (std::size_t i = 0; i < rows_.size(); ++i)
        {
            if (!rows_[i].checkbox->IsChecked())
            {
                continue;
            }

            indices.push_back(i);

            const auto &category = categories[i];

            ConfirmDialogItem item;
            item.label = category.name;

            if (IsUnknownSize(i))
            {
                item.value = L"tamanho desconhecido";
                item.valueMuted = true;
                hasUnknown = true;
            }
            else
            {
                item.value = FormatBytes(rows_[i].scan.bytes);
                totalBytes += rows_[i].scan.bytes;
            }

            content.items.push_back(std::move(item));

            if (category.safety == system::SafetyLevel::Attention)
            {
                hasAttention = true;
            }

            if (category.requiresAdmin &&
                !controller_->IsElevated())
            {
                needsAdmin = true;
            }
        }

        if (indices.empty())
        {
            return;
        }

        content.title = L"Confirmar limpeza";
        content.message = L"Remover os arquivos destas categorias?";

        content.totalLabel = L"Total";
        content.totalValue =
            FormatBytes(totalBytes) +
            (hasUnknown ? L" + desconhecido" : L"");

        if (hasAttention)
        {
            content.notices.push_back(
                {L"Inclui itens de nível Atenção. Leia a descrição de cada um antes de continuar.",
                 true});
        }

        if (hasUnknown)
        {
            content.notices.push_back(
                {L"Alguns tamanhos não puderam ser medidos sem administrador. O total real pode ser maior.",
                 false});
        }

        if (needsAdmin)
        {
            content.notices.push_back(
                {L"O Windows vai pedir permissão de administrador.",
                 false});
        }

        content.footnote =
            L"Arquivos em uso serão ignorados. Esta ação não pode ser desfeita.";

        content.confirmText = L"Limpar agora";
        content.cancelText = L"Cancelar";

        confirmDialog_->Show(
            std::move(content),
            [this, indices = std::move(indices)]()
            {
                if (controller_ != nullptr)
                {
                    controller_->StartClean(indices);
                }
            });
    }

    //
    // EVENTOS DO CONTROLADOR
    //

    bool CleanerPage::IsUnknownSize(std::size_t index) const
    {
        const Row &row = rows_[index];
        const auto &category = controller_->Categories()[index];

        return row.scanned &&
               row.scan.bytes == 0 &&
               row.scan.partial &&
               category.requiresAdmin &&
               !controller_->IsElevated();
    }

    void CleanerPage::HandleScanStarted()
    {
        busy_ = true;

        for (auto &row : rows_)
        {
            row.scanned = false;
            row.selectable = false;
            row.checkbox->SetEnabled(false);
            row.checkbox->SetChecked(false);
            row.size->SetText(L"…");
            row.size->SetColorRole(ColorRole::Muted);
            row.name->SetColorRole(ColorRole::Muted);
            row.note->SetText(L"");
        }

        if (!keepResultMessage_)
        {
            SetStatus(L"Analisando… nada está sendo apagado.");
        }

        UpdateSummary();
        UpdateButtons();
    }

    void CleanerPage::HandleCategoryScanned(
        std::size_t index,
        const system::ScanResult &result)
    {
        if (index >= rows_.size())
        {
            return;
        }

        Row &row = rows_[index];
        const auto &category = controller_->Categories()[index];

        row.scan = result;
        row.scanned = true;

        const bool needsAdmin =
            category.requiresAdmin && !controller_->IsElevated();

        const bool unknown = IsUnknownSize(index);

        row.selectable = result.bytes > 0 || unknown;

        row.checkbox->SetEnabled(row.selectable);

        row.checkbox->SetChecked(
            row.selectable &&
            category.selectedByDefault &&
            result.bytes > 0);

        row.name->SetColorRole(
            row.selectable
                ? ColorRole::Primary
                : ColorRole::Muted);

        if (result.bytes > 0)
        {
            row.size->SetText(FormatBytes(result.bytes));
            row.size->SetColorRole(ColorRole::Accent);
        }
        else if (unknown)
        {
            row.size->SetText(L"—");
            row.size->SetColorRole(ColorRole::Muted);
        }
        else
        {
            row.size->SetText(L"Vazio");
            row.size->SetColorRole(ColorRole::Muted);
        }

        std::wstring note;

        // O aviso só faz sentido quando há algo (ou algo desconhecido)
        // que depende de administrador; uma pasta realmente vazia não
        // precisa dele.
        if (needsAdmin && (result.bytes > 0 || unknown))
        {
            note = L"Requer administrador";
        }

        if (result.partial && result.bytes > 0)
        {
            note += note.empty() ? L"" : L" • ";
            note += L"medição parcial";
        }

        row.note->SetText(note);

        UpdateSummary();
    }

    void CleanerPage::HandleScanFinished()
    {
        busy_ = false;
        everScanned_ = true;

        std::uint64_t total = 0;
        std::size_t categoriesWithFiles = 0;
        bool anyPartial = false;

        for (const auto &row : rows_)
        {
            total += row.scan.bytes;
            anyPartial = anyPartial || row.scan.partial;

            if (row.scan.bytes > 0)
            {
                ++categoriesWithFiles;
            }
        }

        // Sem administrador, o total é só o que o app conseguiu ler.
        const bool needsAdminHint =
            anyPartial && !controller_->IsElevated();

        if (!keepResultMessage_)
        {
            if (total == 0)
            {
                SetStatus(
                    needsAdminHint
                        ? L"Análise concluída: nada encontrado nas pastas que o app consegue ler. Algumas exigem administrador para serem medidas."
                        : L"Análise concluída: não há arquivos para remover.");
            }
            else
            {
                std::wstring text =
                    L"Análise concluída: " +
                    std::wstring(anyPartial ? L"pelo menos " : L"") +
                    FormatBytes(total) +
                    L" podem ser liberados em " +
                    std::to_wstring(categoriesWithFiles) +
                    (categoriesWithFiles == 1
                         ? L" categoria"
                         : L" categorias");

                text += needsAdminHint
                            ? L". Algumas pastas exigem administrador para medir tudo."
                            : L".";

                SetStatus(text);
            }
        }

        UpdateSummary();
        UpdateButtons();
    }

    void CleanerPage::HandleCleanStarted()
    {
        busy_ = true;

        SetStatus(L"Limpando… isso pode levar alguns instantes.");

        cleanButton_->SetText(L"Limpando…");

        UpdateButtons();
    }

    void CleanerPage::HandleCleanFinished(
        const CleanSummary &summary)
    {
        busy_ = false;

        std::wstring message;

        if (summary.removedBytes > 0)
        {
            message =
                L"Limpeza concluída: " +
                FormatBytes(summary.removedBytes) + L" liberados.";
        }
        else
        {
            message = L"Limpeza concluída: nenhum arquivo foi removido.";
        }

        if (summary.failedFiles > 0)
        {
            message +=
                L" " + FormatBytes(summary.failedBytes) +
                L" estavam em uso ou sem permissão e foram ignorados.";
        }

        if (summary.elevationCancelled)
        {
            message +=
                L" A permissão de administrador foi recusada, então essas categorias não foram limpas.";
        }
        else if (summary.elevationFailed)
        {
            message +=
                L" Não foi possível concluir a limpeza das categorias que precisam de administrador.";
        }

        SetStatus(message);

        // Atualiza os tamanhos mantendo a mensagem do resultado.
        keepResultMessage_ = true;
        cleanButton_->SetText(L"Limpar selecionados");

        if (controller_ != nullptr)
        {
            controller_->StartScan();
        }

        UpdateButtons();
    }

    //
    // ATUALIZAÇÃO DA TELA
    //

    void CleanerPage::SetStatus(const std::wstring &text)
    {
        if (statusText_ != nullptr)
        {
            statusText_->SetText(text);
        }
    }

    void CleanerPage::UpdateSummary()
    {
        selectedCount_ = 0;

        std::uint64_t selectedBytes = 0;
        bool anyUnknown = false;
        bool needsAdmin = false;
        bool anySelectable = false;

        for (std::size_t i = 0; i < rows_.size(); ++i)
        {
            if (rows_[i].selectable)
            {
                anySelectable = true;
            }

            if (!rows_[i].checkbox->IsChecked())
            {
                continue;
            }

            ++selectedCount_;
            selectedBytes += rows_[i].scan.bytes;

            if (IsUnknownSize(i))
            {
                anyUnknown = true;
            }

            if (controller_->Categories()[i].requiresAdmin &&
                !controller_->IsElevated())
            {
                needsAdmin = true;
            }
        }

        if (!everScanned_ && !busy_)
        {
            summaryText_->SetText(L"Nenhuma análise feita ainda");
        }
        else if (selectedCount_ == 0)
        {
            summaryText_->SetText(
                anySelectable
                    ? L"Nenhuma categoria selecionada"
                    : L"Nada para limpar");
        }
        else
        {
            std::wstring text =
                std::to_wstring(selectedCount_) +
                (selectedCount_ == 1 ? L" categoria" : L" categorias") +
                L" • " + FormatBytes(selectedBytes) +
                L" para liberar";

            if (anyUnknown)
            {
                text += L" (+ tamanho desconhecido)";
            }

            summaryText_->SetText(text);
        }

        std::wstring note =
            L"Arquivos em uso são ignorados. A limpeza não envia nada para a Lixeira.";

        if (needsAdmin)
        {
            note += L" O Windows vai pedir permissão de administrador.";
        }

        footerNote_->SetText(note);

        UpdateButtons();
    }

    void CleanerPage::UpdateButtons()
    {
        scanButton_->SetText(
            busy_ ? L"Aguarde…" : L"Analisar");

        scanButton_->SetEnabled(!busy_);

        if (adminButton_ != nullptr)
        {
            adminButton_->SetEnabled(!busy_);
        }
        cleanButton_->SetEnabled(!busy_ && selectedCount_ > 0);
    }

    //
    // LAYOUT
    //

    void CleanerPage::Layout(const Rect &area)
    {
        if (titleText_ == nullptr)
        {
            return;
        }

        float y = area.y;

        Place(titleText_, area.x, y, 500.0F, 36.0F);
        Place(subtitleText_, area.x, y + 38.0F, 600.0F, 20.0F);

        Place(
            scanButton_,
            area.x + area.width - HeaderActionReserve - 180.0F,
            y,
            180.0F,
            48.0F);

        // Ao lado do seletor de tema: mesma altura e alinhamento.
        Place(
            adminButton_,
            area.x + area.width - HeaderActionReserve - 180.0F - 12.0F - 170.0F,
            y + 2.0F,
            170.0F,
            44.0F);

        y += 52.0F + 16.0F;

        Place(statusText_, area.x, y, area.width, 20.0F);

        y += 20.0F + 12.0F;

        const float listHeight =
            8.0F +
            static_cast<float>(rows_.size()) * RowHeight +
            FooterHeight;

        Place(listSurface_, area.x, y, area.width, listHeight);

        const float badgeWidth = 72.0F;
        const float sizeWidth = 130.0F;

        for (std::size_t i = 0; i < rows_.size(); ++i)
        {
            Row &row = rows_[i];

            const float rowY =
                y + 4.0F + static_cast<float>(i) * RowHeight;

            Place(row.divider, area.x + 24.0F, rowY, area.width - 48.0F, 1.0F);

            // Todo o retângulo da linha é clicável.
            Place(row.checkbox, area.x + 8.0F, rowY + 2.0F, area.width - 16.0F, RowHeight - 4.0F);

            const float sizeX = area.x + area.width - 24.0F - sizeWidth;
            const float badgeX = sizeX - 16.0F - badgeWidth;
            const float textX = area.x + 60.0F;
            const float textWidth = badgeX - 16.0F - textX;

            Place(row.name, textX, rowY + 8.0F, textWidth, 20.0F);
            Place(row.description, textX, rowY + 30.0F, textWidth, 18.0F);

            Place(row.badge, badgeX, rowY + (RowHeight - 22.0F) / 2.0F, badgeWidth, 22.0F);
            Place(row.badgeText, badgeX, rowY + (RowHeight - 22.0F) / 2.0F, badgeWidth, 22.0F);

            Place(row.size, sizeX, rowY + 10.0F, sizeWidth, 22.0F);
            Place(row.note, sizeX - 90.0F, rowY + 33.0F, sizeWidth + 90.0F, 16.0F);
        }

        const float footerY =
            y + 8.0F + static_cast<float>(rows_.size()) * RowHeight;

        Place(footerDivider_, area.x + 24.0F, footerY, area.width - 48.0F, 1.0F);
        Place(summaryText_, area.x + 24.0F, footerY + 14.0F, area.width - 300.0F, 22.0F);
        Place(footerNote_, area.x + 24.0F, footerY + 38.0F, area.width - 300.0F, 32.0F);

        Place(
            cleanButton_,
            area.x + area.width - 24.0F - 210.0F,
            footerY + 16.0F,
            210.0F,
            44.0F);
    }
}
