#pragma once

#include <windows.h>

#include <atomic>
#include <functional>

namespace real_optimizer::roui
{
    // Permite que threads de trabalho agendem código para rodar
    // na thread da interface (a única que pode mexer nos controles).
    class UiDispatcher final
    {
    public:
        static constexpr UINT Message = WM_APP + 1;

        UiDispatcher() = default;

        UiDispatcher(const UiDispatcher&) = delete;
        UiDispatcher& operator=(const UiDispatcher&) = delete;

        void Attach(HWND window) noexcept;
        void Detach() noexcept;

        // Pode ser chamado de qualquer thread.
        // Retorna false se a janela não existe mais.
        bool Post(std::function<void()> task) const;

        // Chamado pela janela ao receber Message (thread da UI).
        static void Execute(LPARAM parameter);

    private:
        std::atomic<HWND> window_{nullptr};
    };
}
