#pragma once

#include "roui/window/UiDispatcher.h"
#include "system/StartupManager.h"

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>

namespace real_optimizer::features::startup
{
    struct ApplySummary
    {
        std::size_t changed = 0;
        std::size_t unchanged = 0;
        std::size_t notFound = 0;
        std::size_t failed = 0;

        // O usuário recusou o pedido de administrador.
        bool elevationCancelled = false;

        // Não foi possível concluir como administrador.
        bool elevationFailed = false;
    };

    // Coordena a inicialização: lê a lista e aplica alterações fora
    // da thread da interface, registra no log e avisa a UI por
    // eventos (todos chamados na thread da interface).
    class StartupController final
    {
    public:
        explicit StartupController(roui::UiDispatcher& dispatcher);
        ~StartupController();

        StartupController(const StartupController&) = delete;
        StartupController& operator=(const StartupController&) = delete;

        [[nodiscard]] bool IsBusy() const noexcept;
        [[nodiscard]] bool IsElevated() const noexcept;

        // Entradas que o UeX desativou e podem ser restauradas.
        [[nodiscard]] std::vector<system::StartupTarget> BackedUp() const;

        // Eventos (definidos pela UI).
        std::function<void()> onRefreshStarted;
        std::function<void(std::vector<system::StartupApp>)>
            onRefreshFinished;
        std::function<void()> onApplyStarted;
        std::function<void(const ApplySummary&)> onApplyFinished;

        // Só lê. Não altera nada.
        void StartRefresh();

        void StartApply(
            std::vector<system::StartupTarget> disable,
            std::vector<system::StartupTarget> enable);

        // Reativa tudo que o UeX desativou.
        void StartRestore();

        void Shutdown();

    private:
        void StartChange(system::StartupRequest request);
        void JoinWorker();

        roui::UiDispatcher& dispatcher_;
        std::thread worker_;

        // Só é lido/escrito na thread da interface.
        bool busy_ = false;
        bool shuttingDown_ = false;

        bool elevated_ = false;
    };
}
