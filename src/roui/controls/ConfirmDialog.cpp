#include "roui/controls/ConfirmDialog.h"

#include "roui/controls/Button.h"
#include "roui/controls/SurfacePanel.h"
#include "roui/controls/Text.h"
#include "roui/core/Theme.h"
#include "roui/rendering/Renderer.h"

#include <windows.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

namespace real_optimizer::roui
{
    namespace
    {
        constexpr float CardMaxWidth = 560.0F;
        constexpr float CardPadding = 28.0F;
        constexpr float RowHeight = 30.0F;
        constexpr float ValueWidth = 190.0F;

        // Estimativa de quantas linhas um texto ocupa. O Segoe UI
        // usa em média ~0,56 do tamanho da fonte por caractere; a
        // folga evita que o texto escape da caixa.
        int EstimateLines(
            const std::wstring &text,
            float fontSize,
            float width)
        {
            if (text.empty())
            {
                return 0;
            }

            const float perLine =
                std::max(1.0F, std::floor(width / (fontSize * 0.56F)));

            return static_cast<int>(
                std::ceil(static_cast<float>(text.size()) / perLine));
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
            float fontSize,
            FontWeight weight,
            ColorRole role,
            HorizontalAlignment horizontal = HorizontalAlignment::Left,
            VerticalAlignment vertical = VerticalAlignment::Center)
        {
            auto text = std::make_unique<Text>();
            Text *raw = text.get();

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
            Button::Style style)
        {
            auto button = std::make_unique<Button>();
            Button *raw = button.get();

            button->SetStyle(style);

            parent.AddChild(std::move(button));

            return raw;
        }
    }

    bool ConfirmDialog::Initialize()
    {
        SetVisible(false);

        card_ = AddSurface(*this, SurfaceRole::Surface, 16.0F);

        titleText_ = AddText(
            *card_, 18.0F, FontWeight::Bold, ColorRole::Primary);

        messageText_ = AddText(
            *card_, 13.0F, FontWeight::Normal, ColorRole::Secondary,
            HorizontalAlignment::Left, VerticalAlignment::Top);

        listSurface_ = AddSurface(*card_, SurfaceRole::Inset, 12.0F);

        for (ItemRow &row : itemRows_)
        {
            row.label = AddText(
                *listSurface_, 14.0F, FontWeight::Normal,
                ColorRole::Primary);

            row.value = AddText(
                *listSurface_, 14.0F, FontWeight::SemiBold,
                ColorRole::Accent, HorizontalAlignment::Right);
        }

        totalDivider_ = AddSurface(
            *listSurface_, SurfaceRole::Divider, 0.0F);

        totalLabel_ = AddText(
            *listSurface_, 14.0F, FontWeight::Bold, ColorRole::Primary);

        totalValue_ = AddText(
            *listSurface_, 14.0F, FontWeight::Bold, ColorRole::Accent,
            HorizontalAlignment::Right);

        for (NoticeRow &row : noticeRows_)
        {
            row.box = AddSurface(*card_, SurfaceRole::Inset, 10.0F);

            row.text = AddText(
                *row.box, 13.0F, FontWeight::Normal,
                ColorRole::Secondary, HorizontalAlignment::Left,
                VerticalAlignment::Center);
        }

        footnoteText_ = AddText(
            *card_, 12.0F, FontWeight::Normal, ColorRole::Muted,
            HorizontalAlignment::Left, VerticalAlignment::Top);

        cancelButton_ = AddButton(*card_, Button::Style::Secondary);

        cancelButton_->SetOnClick(
            [this]()
            {
                Hide();
            });

        confirmButton_ = AddButton(*card_, Button::Style::Primary);

        confirmButton_->SetOnClick(
            [this]()
            {
                std::function<void()> callback = std::move(onConfirm_);
                onConfirm_ = nullptr;

                Hide();

                if (callback)
                {
                    callback();
                }
            });

        return true;
    }

    void ConfirmDialog::Show(
        ConfirmDialogContent content,
        std::function<void()> onConfirm)
    {
        if (card_ == nullptr)
        {
            return;
        }

        content_ = std::move(content);
        onConfirm_ = std::move(onConfirm);

        if (content_.items.size() > MaxItems)
        {
            content_.items.resize(MaxItems);
        }

        if (content_.notices.size() > MaxNotices)
        {
            content_.notices.resize(MaxNotices);
        }

        titleText_->SetText(content_.title);
        messageText_->SetText(content_.message);

        for (std::size_t i = 0; i < itemRows_.size(); ++i)
        {
            const bool used = i < content_.items.size();

            itemRows_[i].label->SetVisible(used);
            itemRows_[i].value->SetVisible(used);

            if (used)
            {
                const ConfirmDialogItem &item = content_.items[i];

                itemRows_[i].label->SetText(item.label);
                itemRows_[i].value->SetText(item.value);
                itemRows_[i].value->SetColorRole(
                    item.valueMuted
                        ? ColorRole::Muted
                        : ColorRole::Accent);
            }
        }

        const bool hasTotal = !content_.totalLabel.empty();

        totalDivider_->SetVisible(hasTotal);
        totalLabel_->SetVisible(hasTotal);
        totalValue_->SetVisible(hasTotal);
        totalLabel_->SetText(content_.totalLabel);
        totalValue_->SetText(content_.totalValue);

        for (std::size_t i = 0; i < noticeRows_.size(); ++i)
        {
            const bool used = i < content_.notices.size();

            noticeRows_[i].box->SetVisible(used);

            if (used)
            {
                const ConfirmDialogNotice &notice = content_.notices[i];

                noticeRows_[i].box->SetRole(
                    notice.warning
                        ? SurfaceRole::WarningSoft
                        : SurfaceRole::Inset);

                noticeRows_[i].text->SetColorRole(
                    notice.warning
                        ? ColorRole::Warning
                        : ColorRole::Secondary);

                noticeRows_[i].text->SetText(notice.text);
            }
        }

        footnoteText_->SetText(content_.footnote);
        cancelButton_->SetText(content_.cancelText);
        confirmButton_->SetText(content_.confirmText);

        Layout(windowWidth_, windowHeight_);

        SetVisible(true);
    }

    void ConfirmDialog::Hide()
    {
        SetVisible(false);
        onConfirm_ = nullptr;
    }

    bool ConfirmDialog::IsOpen() const noexcept
    {
        return IsVisible();
    }

    bool ConfirmDialog::OnKeyDown(unsigned int virtualKey)
    {
        if (!IsVisible())
        {
            return false;
        }

        if (virtualKey == VK_ESCAPE)
        {
            Hide();
        }

        // Enquanto aberta, nenhuma tecla vai para o resto do app.
        // O Enter não confirma de propósito: a ação é destrutiva.
        return true;
    }

    void ConfirmDialog::OnRender(
        rendering::Renderer &renderer)
    {
        // Escurece a janela por trás da caixa.
        const bool dark = Theme::GetMode() == ThemeMode::Dark;

        renderer.FillRectangle(
            GetBounds(),
            dark
                ? Color{0.0F, 0.0F, 0.0F, 0.62F}
                : Color{0.06F, 0.08F, 0.10F, 0.42F});

        Panel::OnRender(renderer);
    }

    void ConfirmDialog::Layout(
        float windowWidth,
        float windowHeight)
    {
        windowWidth_ = windowWidth;
        windowHeight_ = windowHeight;

        SetBounds({0.0F, 0.0F, windowWidth, windowHeight});

        if (card_ == nullptr)
        {
            return;
        }

        const float cardWidth =
            std::min(CardMaxWidth, windowWidth - 80.0F);

        const float innerWidth = cardWidth - CardPadding * 2.0F;

        const bool hasTotal = !content_.totalLabel.empty();
        const std::size_t itemCount =
            std::min(content_.items.size(), itemRows_.size());

        // Passo único que mede (apply = false) ou posiciona
        // (apply = true). Retorna a altura total do cartão.
        const auto run =
            [&](bool apply, float ox, float oy) -> float
        {
            const float x0 = ox + CardPadding;

            float y = 24.0F;

            if (apply)
            {
                Place(titleText_, x0, oy + y, innerWidth, 26.0F);
            }

            y += 32.0F;

            const int messageLines =
                EstimateLines(content_.message, 13.0F, innerWidth);

            const float messageHeight =
                static_cast<float>(messageLines) * 18.0F;

            if (apply)
            {
                Place(messageText_, x0, oy + y, innerWidth, messageHeight);
            }

            y += messageHeight + (messageHeight > 0.0F ? 14.0F : 6.0F);

            if (itemCount > 0)
            {
                float listHeight =
                    static_cast<float>(itemCount) * RowHeight + 20.0F;

                if (hasTotal)
                {
                    listHeight += RowHeight + 8.0F;
                }

                if (apply)
                {
                    Place(listSurface_, x0, oy + y, innerWidth, listHeight);

                    const float labelWidth =
                        innerWidth - 32.0F - ValueWidth;

                    for (std::size_t i = 0; i < itemCount; ++i)
                    {
                        const float rowY =
                            oy + y + 10.0F +
                            static_cast<float>(i) * RowHeight;

                        Place(itemRows_[i].label, x0 + 16.0F, rowY, labelWidth, RowHeight);
                        Place(itemRows_[i].value, x0 + innerWidth - 16.0F - ValueWidth, rowY, ValueWidth, RowHeight);
                    }

                    if (hasTotal)
                    {
                        const float totalY =
                            oy + y + 10.0F +
                            static_cast<float>(itemCount) * RowHeight;

                        Place(totalDivider_, x0 + 16.0F, totalY + 4.0F, innerWidth - 32.0F, 1.0F);
                        Place(totalLabel_, x0 + 16.0F, totalY + 8.0F, labelWidth, RowHeight);
                        Place(totalValue_, x0 + innerWidth - 16.0F - ValueWidth, totalY + 8.0F, ValueWidth, RowHeight);
                    }
                }

                y += listHeight + 14.0F;
            }

            const std::size_t noticeCount =
                std::min(content_.notices.size(), noticeRows_.size());

            for (std::size_t i = 0; i < noticeCount; ++i)
            {
                const int lines = EstimateLines(
                    content_.notices[i].text,
                    13.0F,
                    innerWidth - 28.0F);

                const float boxHeight =
                    16.0F + static_cast<float>(lines) * 18.0F;

                if (apply)
                {
                    Place(noticeRows_[i].box, x0, oy + y, innerWidth, boxHeight);
                    Place(noticeRows_[i].text, x0 + 14.0F, oy + y + 4.0F, innerWidth - 28.0F, boxHeight - 8.0F);
                }

                y += boxHeight + 8.0F;
            }

            if (!content_.notices.empty())
            {
                y += 4.0F;
            }

            const int footnoteLines =
                EstimateLines(content_.footnote, 12.0F, innerWidth);

            if (footnoteLines > 0)
            {
                const float footnoteHeight =
                    static_cast<float>(footnoteLines) * 16.0F;

                if (apply)
                {
                    Place(footnoteText_, x0, oy + y, innerWidth, footnoteHeight);
                }

                y += footnoteHeight + 18.0F;
            }
            else
            {
                y += 6.0F;
            }

            if (apply)
            {
                const float confirmWidth = 150.0F;
                const float cancelWidth = 120.0F;
                const float right = ox + cardWidth - CardPadding;

                Place(confirmButton_, right - confirmWidth, oy + y, confirmWidth, 44.0F);
                Place(cancelButton_, right - confirmWidth - 12.0F - cancelWidth, oy + y, cancelWidth, 44.0F);
            }

            y += 44.0F + 24.0F;

            return y;
        };

        const float cardHeight = run(false, 0.0F, 0.0F);

        const float cardX = (windowWidth - cardWidth) / 2.0F;

        const float cardY =
            std::max(20.0F, (windowHeight - cardHeight) / 2.0F);

        Place(card_, cardX, cardY, cardWidth, cardHeight);

        static_cast<void>(run(true, cardX, cardY));
    }
}
