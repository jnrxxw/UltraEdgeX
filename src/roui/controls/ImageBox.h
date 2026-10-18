#pragma once

#include "roui/core/UIElement.h"

#include <d2d1.h>
#include <string>
#include <windows.h>
#include <wrl/client.h>

namespace real_optimizer::roui
{
    class ImageBox final : public UIElement
    {
    public:
        ImageBox() = default;
        ~ImageBox() override = default;

        void SetResource(HMODULE module, int resourceId);
        void SetFilePath(std::wstring filePath);
        void SetOpacity(float opacity) noexcept;

    protected:
        void OnRender(
            rendering::Renderer& renderer
        ) override;

    private:
        HMODULE resourceModule_ = nullptr;
        int resourceId_ = 0;
        std::wstring filePath_;
        float opacity_ = 1.0F;

        Microsoft::WRL::ComPtr<ID2D1Bitmap> bitmap_;
    };
}
