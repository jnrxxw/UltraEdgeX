#pragma once

namespace real_optimizer::roui
{
    struct Color
    {
        float red = 1.0F;
        float green = 1.0F;
        float blue = 1.0F;
        float alpha = 1.0F;

        static constexpr Color FromRgb(
            unsigned int red,
            unsigned int green,
            unsigned int blue
        ) noexcept
        {
            return {
                static_cast<float>(red) / 255.0F,
                static_cast<float>(green) / 255.0F,
                static_cast<float>(blue) / 255.0F,
                1.0F
            };
        }

        // Mistura duas cores. amount = 0 retorna "from", 1 retorna "to".
        static constexpr Color Mix(
            const Color& from,
            const Color& to,
            float amount
        ) noexcept
        {
            return {
                from.red + (to.red - from.red) * amount,
                from.green + (to.green - from.green) * amount,
                from.blue + (to.blue - from.blue) * amount,
                from.alpha + (to.alpha - from.alpha) * amount
            };
        }
    };

    enum class FontWeight
    {
        Normal,
        SemiBold,
        Bold
    };

    enum class HorizontalAlignment
    {
        Left,
        Center,
        Right
    };

    enum class VerticalAlignment
    {
        Top,
        Center,
        Bottom
    };

    // Papel semântico de uma cor de texto.
    // Resolvido em tempo de renderização pelo tema atual,
    // então trocar o tema atualiza tudo sem reconstruir a UI.
    enum class ColorRole
    {
        Custom,
        Primary,
        Secondary,
        Muted,
        Accent,
        OnAccent,
        Success,
        Warning
    };
}
