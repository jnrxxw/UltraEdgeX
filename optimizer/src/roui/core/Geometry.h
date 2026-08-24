#pragma once

namespace real_optimizer::roui
{
    struct Point
    {
        float x = 0.0F;
        float y = 0.0F;
    };

    struct Size
    {
        float width = 0.0F;
        float height = 0.0F;

        [[nodiscard]] bool IsEmpty() const noexcept
        {
            return width <= 0.0F || height <= 0.0F;
        }
    };

    struct Thickness
   {
    float left = 0.0F;
    float top = 0.0F;
    float right = 0.0F;
    float bottom = 0.0F;

    static constexpr Thickness Uniform(float value)
    
     noexcept
      {
        return {
            value,
            value,
            value,
            value
            };
      }
    };

    struct Rect
    {
        float x = 0.0F;
        float y = 0.0F;
        float width = 0.0F;
        float height = 0.0F;

        [[nodiscard]] float Left() const noexcept
        {
            return x;
        }

        [[nodiscard]] float Top() const noexcept
        {
            return y;
        }

        [[nodiscard]] float Right() const noexcept
        {
            return x + width;
        }

        [[nodiscard]] float Bottom() const noexcept
        {
            return y + height;
        }

        [[nodiscard]] bool IsEmpty() const noexcept
        {
            return width <= 0.0F || height <= 0.0F;
        }

        [[nodiscard]] bool Contains(const Point& point) const noexcept
        {
            return point.x >= Left() &&
                   point.x <= Right() &&
                   point.y >= Top() &&
                   point.y <= Bottom();
        }
    };
}