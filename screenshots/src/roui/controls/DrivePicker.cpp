#include "roui/controls/DrivePicker.h"

#include "roui/core/Theme.h"
#include "roui/rendering/Renderer.h"

#include <windows.h>

#include <algorithm>
#include <utility>

namespace real_optimizer::roui
{
    namespace
    {
        constexpr float MenuWidth = 340.0F;
        constexpr float MenuPadding = 8.0F;
        constexpr float RowStep = 58.0F;
        constexpr float RowHeight = 54.0F;
        constexpr float ScreenMargin = 12.0F;
    }

    void DrivePicker::Show(
        std::vector<DrivePickerItem> items,
        std::size_t selectedIndex,
        const Rect& anchor,
        SelectHandler onSelect,
        std::function<void()> onClose)
    {
        if (items.size() > MaxItems)
        {
            items.resize(MaxItems);
        }

        items_ = std::move(items);
        selected_ = selectedIndex < items_.size() ? selectedIndex : None;
        hover_ = selected_;
        pressed_ = None;
        anchor_ = anchor;
        onSelect_ = std::move(onSelect);
        onClose_ = std::move(onClose);

        SetVisible(true);
    }

    void DrivePicker::Hide()
    {
        const bool wasOpen = IsVisible();

        SetVisible(false);

        hover_ = None;
        pressed_ = None;
        onSelect_ = nullptr;

        std::function<void()> close = std::move(onClose_);
        onClose_ = nullptr;

        if (wasOpen && close)
        {
            close();
        }
    }

    bool DrivePicker::IsOpen() const noexcept
    {
        return IsVisible();
    }

    void DrivePicker::Choose(std::size_t index)
    {
        SelectHandler handler = std::move(onSelect_);
        onSelect_ = nullptr;

        Hide();

        if (handler && index < items_.size())
        {
            handler(index);
        }
    }

    bool DrivePicker::OnKeyDown(unsigned int virtualKey)
    {
        if (!IsVisible())
        {
            return false;
        }

        const std::size_t count = items_.size();

        switch (virtualKey)
        {
        case VK_ESCAPE:
            Hide();
            break;

        case VK_DOWN:
            if (count > 0)
            {
                hover_ = hover_ == None ? 0 : (hover_ + 1) % count;
            }
            break;

        case VK_UP:
            if (count > 0)
            {
                hover_ = hover_ == None ? count - 1
                                        : (hover_ + count - 1) % count;
            }
            break;

        case VK_RETURN:
            if (hover_ != None)
            {
                Choose(hover_);
            }
            break;

        default:
            break;
        }

        // Enquanto aberto, nenhuma tecla vai para o resto do app.
        return true;
    }

    //
    // GEOMETRIA
    //

    Rect DrivePicker::MenuRect() const noexcept
    {
        const Rect& area = GetBounds();

        const float width =
            std::min(MenuWidth, std::max(120.0F, area.width - ScreenMargin * 2.0F));

        const float height =
            MenuPadding * 2.0F + static_cast<float>(items_.size()) * RowStep;

        float x = anchor_.x + anchor_.width - width;

        x = std::min(x, area.x + area.width - width - ScreenMargin);
        x = std::max(x, area.x + ScreenMargin);

        float y = anchor_.y + anchor_.height + 6.0F;

        if (y + height > area.y + area.height - ScreenMargin)
        {
            y = std::max(
                area.y + ScreenMargin,
                area.y + area.height - ScreenMargin - height);
        }

        return Rect{x, y, width, height};
    }

    Rect DrivePicker::RowRect(std::size_t index) const noexcept
    {
        const Rect menu = MenuRect();

        return Rect{
            menu.x + MenuPadding,
            menu.y + MenuPadding + static_cast<float>(index) * RowStep,
            menu.width - MenuPadding * 2.0F,
            RowHeight};
    }

    bool DrivePicker::InsideMenu(const Point& point) const noexcept
    {
        const Rect menu = MenuRect();

        return point.x >= menu.x &&
               point.x <= menu.x + menu.width &&
               point.y >= menu.y &&
               point.y <= menu.y + menu.height;
    }

    std::size_t DrivePicker::IndexAt(const Point& point) const noexcept
    {
        for (std::size_t i = 0; i < items_.size(); ++i)
        {
            const Rect row = RowRect(i);

            if (point.x >= row.x &&
                point.x <= row.x + row.width &&
                point.y >= row.y &&
                point.y <= row.y + RowStep)
            {
                return i;
            }
        }

        return None;
    }

    //
    // MOUSE
    //

    void DrivePicker::OnMouseMove(const Point& point)
    {
        if (!IsVisible())
        {
            return;
        }

        hover_ = IndexAt(point);
    }

    void DrivePicker::OnMouseLeave()
    {
        if (IsVisible())
        {
            hover_ = None;
        }
    }

    void DrivePicker::OnMouseDown(const Point& point)
    {
        if (!IsVisible())
        {
            return;
        }

        // Clique fora do menu fecha (e não chega ao que está embaixo).
        if (!InsideMenu(point))
        {
            Hide();
            return;
        }

        pressed_ = IndexAt(point);
    }

    void DrivePicker::OnMouseUp(const Point& point)
    {
        if (!IsVisible())
        {
            return;
        }

        const std::size_t pressed = pressed_;
        pressed_ = None;

        if (pressed != None && pressed == IndexAt(point))
        {
            Choose(pressed);
        }
    }

    //
    // DESENHO
    //

    void DrivePicker::OnRender(rendering::Renderer& renderer)
    {
        const Palette& palette = Theme::Current();
        const Rect menu = MenuRect();

        // Sombra leve atrás do menu.
        renderer.FillRoundedRectangle(
            Rect{menu.x - 2.0F, menu.y + 3.0F, menu.width + 4.0F, menu.height + 4.0F},
            16.0F,
            Color{0.0F, 0.0F, 0.0F, 0.25F});

        renderer.FillRoundedRectangle(menu, 14.0F, palette.surface);
        renderer.DrawRoundedRectangle(menu, 14.0F, 1.0F, palette.border);

        for (std::size_t i = 0; i < items_.size(); ++i)
        {
            const DrivePickerItem& item = items_[i];
            const Rect row = RowRect(i);

            const bool isSelected = i == selected_;
            const bool isHover = i == hover_;

            if (isSelected)
            {
                renderer.FillRoundedRectangle(row, 10.0F, palette.accentSoft);
            }
            else if (isHover)
            {
                renderer.FillRoundedRectangle(row, 10.0F, palette.hover);
            }

            renderer.DrawText(
                item.title,
                Rect{row.x + 12.0F, row.y + 7.0F, row.width - 48.0F, 20.0F},
                14.0F,
                FontWeight::SemiBold,
                palette.textPrimary,
                HorizontalAlignment::Left,
                VerticalAlignment::Center);

            renderer.DrawText(
                item.detail,
                Rect{row.x + 12.0F, row.y + 27.0F, row.width - 24.0F, 16.0F},
                12.0F,
                FontWeight::Normal,
                palette.textSecondary,
                HorizontalAlignment::Left,
                VerticalAlignment::Center);

            const Rect track{
                row.x + 12.0F,
                row.y + row.height - 10.0F,
                row.width - 24.0F,
                4.0F};

            renderer.FillRoundedRectangle(track, 2.0F, palette.track);

            const float fraction =
                std::clamp(item.usedFraction, 0.0F, 1.0F);

            if (fraction * track.width > 0.5F)
            {
                renderer.FillRoundedRectangle(
                    Rect{track.x, track.y, track.width * fraction, track.height},
                    2.0F,
                    palette.accent);
            }

            // Marca de escolhido.
            if (isSelected)
            {
                const float checkX = row.x + row.width - 22.0F;
                const float checkY = row.y + 17.0F;

                renderer.DrawLine(
                    Point{checkX - 5.0F, checkY},
                    Point{checkX - 1.5F, checkY + 3.5F},
                    2.0F,
                    palette.accentText);

                renderer.DrawLine(
                    Point{checkX - 1.5F, checkY + 3.5F},
                    Point{checkX + 5.0F, checkY - 3.5F},
                    2.0F,
                    palette.accentText);
            }
        }
    }
}
