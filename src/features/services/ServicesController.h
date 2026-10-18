#pragma once

#include "roui/window/UiDispatcher.h"
#include "system/Services.h"

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>

namespace real_optimizer::features::services
{
    struct ApplySummary
    {
        std::size_t changed = 0;
        std::size_t unchanged = 0;
        std::size_t inUse = 0;
        std::size_t failed = 0;
        std::size_t notInstalled = 0;

        // O usuário recusou o pedido de administrador.
        bool elevationCancelled = false;

        // Não foi possível concluir como administrador.
        bool elevationFailed = false;
    };

    // Coordena os serviços: lê o estado e aplica alterações fora
    // da thread da interface, registra no log e avisa a UI por
    // eventos (todos chamados na thread da interface).
    class ServicesController final
    {
    public:
        explicit ServicesController(roui::UiDispatcher& dispatcher);
        ~ServicesController();

        ServicesController(const ServicesController&) = delete;
        ServicesController& operator=(const ServicesController&) = delete;

        [[nodiscard]] const std::vector<system::ServiceDefinition>&
        Definitions() const noexcept;

        [[nodiscard]] bool IsBusy() const noexcept;
        [[nodiscard]] bool IsElevated() const noexcept;

        // Serviços que o UeX alterou e podem ser restaurados.
        [[nodiscard]] std::vector<std::wstring> BackedUpNames() const;

        // Eventos (definidos pela UI).
        std::function<void()> onRefreshStarted;
        std::function<void(std::size_t, const system::ServiceState&)>
            onServiceLoaded;
        std::function<void()> onRefreshFinished;
        std::function<void()> onApplyStarted;
        std::function<void(const ApplySummary&)> onApplyFinished;

        // Só lê. Não altera nada.
        void StartRefresh();

        // Índices em Definitions().
        void StartApply(
            std::vector<std::size_t> disable,
            std::vector<std::size_t> enable);

        // Volta tudo que o UeX alterou à configuração original.
        void StartRestore();

        void Shutdown();

    private:
        void StartChange(system::ServiceRequest request);
        void JoinWorker();

        roui::UiDispatcher& dispatcher_;
        std::thread worker_;

        // Só é lido/escrito na thread da interface.
        bool busy_ = false;
        bool shuttingDown_ = false;

        bool elevated_ = false;
    };
}
