#include "roui/core/Theme.h"

#include <windows.h>

namespace real_optimizer::roui
{
    namespace
    {
        constexpr Palette DarkPalette{
            Color::FromRgb(10, 12, 13),    // background
            Color::FromRgb(13, 16, 18),    // sidebar
            Color::FromRgb(16, 21, 23),    // surface
            Color::FromRgb(12, 17, 19),    // surfaceInset
            Color::FromRgb(26, 35, 39),    // border
            Color::FromRgb(42, 58, 62),    // outline
            Color::FromRgb(24, 32, 36),    // divider
            Color::FromRgb(18, 24, 26),    // hover
            Color::FromRgb(230, 234, 236), // textPrimary
            Color::FromRgb(154, 166, 171), // textSecondary
            Color::FromRgb(125, 138, 143), // textMuted
            Color::FromRgb(172, 0, 255),   // accent
            Color::FromRgb(199, 102, 255), // accentText
            Color::FromRgb(31, 16, 48),    // accentSoft
            Color::FromRgb(255, 255, 255), // onAccent
            Color::FromRgb(27, 36, 39),    // track
            Color::FromRgb(74, 222, 128),  // success
            Color::FromRgb(16, 40, 26),    // successSoft
            Color::FromRgb(251, 191, 36),  // warning
            Color::FromRgb(44, 34, 10)     // warningSoft
        };

        constexpr Palette LightPalette{
            Color::FromRgb(244, 247, 248), // background
            Color::FromRgb(255, 255, 255), // sidebar
            Color::FromRgb(255, 255, 255), // surface
            Color::FromRgb(247, 249, 250), // surfaceInset
            Color::FromRgb(227, 232, 234), // border
            Color::FromRgb(203, 213, 217), // outline
            Color::FromRgb(237, 241, 242), // divider
            Color::FromRgb(241, 244, 245), // hover
            Color::FromRgb(20, 24, 26),    // textPrimary
            Color::FromRgb(91, 103, 108),  // textSecondary
            Color::FromRgb(102, 115, 122), // textMuted
            Color::FromRgb(172, 0, 255),   // accent
            Color::FromRgb(172, 0, 255),   // accentText
            Color::FromRgb(241, 227, 255), // accentSoft
            Color::FromRgb(255, 255, 255), // onAccent
            Color::FromRgb(230, 236, 238), // track
            Color::FromRgb(21, 128, 61),   // success
            Color::FromRgb(220, 252, 231), // successSoft
            Color::FromRgb(161, 98, 7),    // warning
            Color::FromRgb(254, 243, 199)  // warningSoft
        };

        ThemeMode currentMode = ThemeMode::Dark;

        constexpr wchar_t RegistryKey[] = L"Software\\UltraEdgeX";
        constexpr wchar_t RegistryValue[] = L"Theme";
    }

    const Palette& Theme::Current() noexcept
    {
        return currentMode == ThemeMode::Light
                   ? LightPalette
                   : DarkPalette;
    }

    ThemeMode Theme::GetMode() noexcept
    {
        return currentMode;
    }

    void Theme::SetMode(ThemeMode mode) noexcept
    {
        currentMode = mode;
    }

    void Theme::Toggle() noexcept
    {
        currentMode = currentMode == ThemeMode::Dark
                          ? ThemeMode::Light
                          : ThemeMode::Dark;
    }

    Color Theme::Resolve(
        ColorRole role,
        const Color& custom
    ) noexcept
    {
        const Palette& palette = Current();

        switch (role)
        {
        case ColorRole::Primary:
            return palette.textPrimary;

        case ColorRole::Secondary:
            return palette.textSecondary;

        case ColorRole::Muted:
            return palette.textMuted;

        case ColorRole::Accent:
            return palette.accentText;

        case ColorRole::OnAccent:
            return palette.onAccent;

        case ColorRole::Success:
            return palette.success;

        case ColorRole::Warning:
            return palette.warning;

        case ColorRole::Custom:
        default:
            return custom;
        }
    }

    void Theme::LoadPreference() noexcept
    {
        DWORD value = 0;
        DWORD size = sizeof(value);

        const LSTATUS status = RegGetValueW(
            HKEY_CURRENT_USER,
            RegistryKey,
            RegistryValue,
            RRF_RT_REG_DWORD,
            nullptr,
            &value,
            &size);

        if (status == ERROR_SUCCESS)
        {
            currentMode = value == 1
                              ? ThemeMode::Light
                              : ThemeMode::Dark;
        }
    }

    void Theme::SavePreference() noexcept
    {
        const DWORD value =
            currentMode == ThemeMode::Light ? 1 : 0;

        RegSetKeyValueW(
            HKEY_CURRENT_USER,
            RegistryKey,
            RegistryValue,
            REG_DWORD,
            &value,
            sizeof(value));
    }
}
