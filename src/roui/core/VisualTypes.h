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
}