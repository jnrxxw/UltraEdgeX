#include "roui/controls/ImageBox.h"

#include "roui/rendering/Renderer.h"

#include <utility>

namespace real_optimizer::roui
{
    void ImageBox::SetResource(
        HMODULE module,
        int resourceId)
    {
        resourceModule_ = module;
        resourceId_ = resourceId;
        bitmap_.Reset();
    }

    void ImageBox::SetFilePath(
        std::wstring filePath)
    {
        filePath_ = std::move(filePath);
        bitmap_.Reset();
    }

    void ImageBox::SetOpacity(
        float opacity) noexcept
    {
        opacity_ = opacity;
    }

    void ImageBox::OnRender(
        rendering::Renderer& renderer)
    {
        if (bitmap_ == nullptr)
        {
            if (resourceId_ != 0)
            {
                bitmap_ = renderer.CreateBitmapFromResource(
                    resourceModule_,
                    resourceId_);
            }

            if (bitmap_ == nullptr && !filePath_.empty())
            {
                bitmap_ = renderer.CreateBitmapFromFile(filePath_);
            }

            if (bitmap_ == nullptr && !filePath_.empty())
            {
                wchar_t exeBuffer[MAX_PATH]{};
                if (GetModuleFileNameW(nullptr, exeBuffer, MAX_PATH) > 0)
                {
                    std::wstring exeDir(exeBuffer);
                    const auto lastSlash = exeDir.find_last_of(L"\\/");
                    if (lastSlash != std::wstring::npos)
                    {
                        exeDir = exeDir.substr(0, lastSlash + 1);
                        bitmap_ = renderer.CreateBitmapFromFile(exeDir + filePath_);
                        if (bitmap_ == nullptr)
                        {
                            bitmap_ = renderer.CreateBitmapFromFile(exeDir + L"..\\" + filePath_);
                            if (bitmap_ == nullptr)
                            {
                                bitmap_ = renderer.CreateBitmapFromFile(exeDir + L"..\\..\\" + filePath_);
                            }
                        }
                    }
                }
            }
        }

        if (bitmap_ != nullptr)
        {
            if (!renderer.DrawBitmap(bitmap_.Get(), GetBounds(), opacity_))
            {
                bitmap_.Reset();
            }
        }
    }
}
