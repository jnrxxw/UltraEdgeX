#pragma once

#include "roui/window/UiDispatcher.h"
#include "system/Cleaner.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>

namespace real_optimizer::features::cleaner
{
    struct CleanSummary
    {
        std::uint64_t removedBytes = 0;
        std::uint64_t removedFiles = 0;
        std::uint64_t failedBytes = 0;
        std::uint64_t failedFiles = 0;

        // O usuário recusou o pedido de administrador.
        bool elevationCancelled = false;

        // Não foi possível limpar as categorias de administrador.
        bool elevationFailed = false;
    };

    // Coordena o Cleaner: roda análise e limpeza fora da thread
    // da interface, registra no log e avisa a UI por eventos.
    // Todos os eventos são chamados na thread da interface.
    class CleanerController final
    {
    public:
        explicit CleanerController(roui::UiDispatcher& dispatcher);
        ~CleanerController();

        CleanerController(const CleanerController&) = delete;
        CleanerController& operator=(const CleanerController&) = delete;

        [[nodiscard]] const std::vector<system::CleanCategory>&
        Categories() const noexcept;

        [[nodiscard]] bool IsBusy() const noexcept;
        [[nodiscard]] bool IsElevated() const noexcept;

        // Eventos (definidos pela UI).
        std::function<void()> onScanStarted;
        std::function<void(std::size_t, const system::ScanResult&)>
            onCategoryScanned;
        std::function<void()> onScanFinished;
        std::function<void()> onCleanStarted;
        std::function<void(const CleanSummary&)> onCleanFinished;

        // Apenas mede os arquivos. Não apaga nada.
        void StartScan();

        // indices = posições em Categories().
        void StartClean(std::vector<std::size_t> indices);

        // Abre o app como administrador. Retorna true se a nova
        // cópia foi aberta (a atual deve ser fechada).
        [[nodiscard]] bool RestartAsAdministrator();

        // Espera o trabalho em andamento terminar.
        void Shutdown();

    private:
        void JoinWorker();

        roui::UiDispatcher& dispatcher_;
        std::thread worker_;

        // Só é lido/escrito na thread da interface.
        bool busy_ = false;
        bool shuttingDown_ = false;

        bool elevated_ = false;
    };
}
