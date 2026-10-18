#pragma once

#include "roui/core/VisualTypes.h"

namespace real_optimizer::roui
{
    enum class ThemeMode
    {
        Dark,
        Light
    };

    struct Palette
    {
        Color background;
        Color sidebar;
        Color surface;
        Color surfaceInset;
        Color border;
        Color outline;
        Color divider;
        Color hover;

        Color textPrimary;
        Color textSecondary;
        Color textMuted;

        Color accent;      // preenchimentos (botões, anel, barras)
        Color accentText;  // texto/números na cor de destaque
        Color accentSoft;  // fundo suave da cor de destaque
        Color onAccent;    // texto sobre o accent
        Color track;       // trilho de barras e anel

        Color success;     // texto de sucesso / nível Seguro
        Color successSoft; // fundo suave de sucesso
        Color warning;     // texto de aviso / nível Atenção
        Color warningSoft; // fundo suave de aviso
    };

    class Theme final
    {
    public:
        Theme() = delete;

        [[nodiscard]] static const Palette& Current() noexcept;
        [[nodiscard]] static ThemeMode GetMode() noexcept;

        static void SetMode(ThemeMode mode) noexcept;
        static void Toggle() noexcept;

        [[nodiscard]] static Color Resolve(
            ColorRole role,
            const Color& custom
        ) noexcept;

        // Preferência salva por usuário em
        // HKCU\Software\UltraEdgeX (valor "Theme").
        static void LoadPreference() noexcept;
        static void SavePreference() noexcept;
    };
}
