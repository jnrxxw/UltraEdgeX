#include "roui/window/UiDispatcher.h"

#include <memory>
#include <utility>

namespace real_optimizer::roui
{
    void UiDispatcher::Attach(HWND window) noexcept
    {
        window_.store(window);
    }

    void UiDispatcher::Detach() noexcept
    {
        window_.store(nullptr);
    }

    bool UiDispatcher::Post(
        std::function<void()> task
    ) const
    {
        const HWND window = window_.load();

        if (window == nullptr || !task)
        {
            return false;
        }

        auto* heapTask =
            new std::function<void()>(std::move(task));

        if (!PostMessageW(
                window,
                Message,
                0,
                reinterpret_cast<LPARAM>(heapTask)))
        {
            delete heapTask;
            return false;
        }

        return true;
    }

    void UiDispatcher::Execute(LPARAM parameter)
    {
        std::unique_ptr<std::function<void()>> task(
            reinterpret_cast<std::function<void()>*>(parameter));

        if (task != nullptr && *task)
        {
            (*task)();
        }
    }
}
