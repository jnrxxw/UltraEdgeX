#include "features/startup/StartupController.h"

#include "core/Logger.h"
#include "system/Cleaner.h"
#include "system/ElevatedStartup.h"

#include <string>
#include <utility>

namespace real_optimizer::features::startup
{
    StartupController::StartupController(
        roui::UiDispatcher& dispatcher)
        : dispatcher_(dispatcher),
          elevated_(system::Cleaner::IsElevated())
    {
    }

    StartupController::~StartupController()
    {
        Shutdown();
    }

    bool StartupController::IsBusy() const noexcept
    {
        return busy_;
    }

    bool StartupController::IsElevated() const noexcept
    {
        return elevated_;
    }

    std::vector<system::StartupTarget> StartupController::BackedUp() const
    {
        return system::StartupManager::BackedUp();
    }

    void StartupController::JoinWorker()
    {
        if (worker_.joinable())
        {
            worker_.join();
        }
    }

    void StartupController::Shutdown()
    {
        shuttingDown_ = true;
        JoinWorker();
    }

    void StartupController::StartRefresh()
    {
        if (busy_ || shuttingDown_)
        {
            return;
        }

        busy_ = true;
        JoinWorker();

        if (onRefreshStarted)
        {
            onRefreshStarted();
        }

        worker_ = std::thread(
            [this]()
            {
                std::vector<system::StartupApp> apps =
                    system::StartupManager::GetStartupApps();

                dispatcher_.Post(
                    [this, apps = std::move(apps)]() mutable
                    {
                        busy_ = false;

                        if (!shuttingDown_ && onRefreshFinished)
                        {
                            onRefreshFinished(std::move(apps));
                        }
                    });
            });
    }

    void StartupController::StartApply(
        std::vector<system::StartupTarget> disable,
        std::vector<system::StartupTarget> enable)
    {
        system::StartupRequest request;
        request.disable = std::move(disable);
        request.enable = std::move(enable);

        if (request.IsEmpty())
        {
            return;
        }

        StartChange(std::move(request));
    }

    void StartupController::StartRestore()
    {
        system::StartupRequest request;

        // Só pede administrador se houver algo de "todos os usuários".
        for (const system::StartupTarget& target : BackedUp())
        {
            if (system::StartupManager::RequiresAdministrator(target.source))
            {
                request.restoreMachine = true;
            }
            else
            {
                request.restoreCurrentUser = true;
            }
        }

        if (request.IsEmpty())
        {
            return;
        }

        StartChange(std::move(request));
    }

    void StartupController::StartChange(system::StartupRequest request)
    {
        if (busy_ || shuttingDown_)
        {
            return;
        }

        busy_ = true;
        JoinWorker();

        if (onApplyStarted)
        {
            onApplyStarted();
        }

        const bool elevated = elevated_;

        worker_ = std::thread(
            [this, request = std::move(request), elevated]()
            {
                ApplySummary summary;
                std::vector<system::StartupChange> changes;

                core::Logger::Info(
                    L"Alteração de inicialização iniciada: " +
                    std::to_wstring(request.disable.size()) +
                    L" para desativar, " +
                    std::to_wstring(request.enable.size()) +
                    L" para reativar" +
                    (request.restoreCurrentUser || request.restoreMachine
                         ? L", restaurar."
                         : L"."));

                // Separa o que este processo consegue alterar (usuário
                // atual) do que exige administrador (todos os usuários).
                system::StartupRequest userRequest;
                system::StartupRequest machineRequest;

                for (const auto& target : request.disable)
                {
                    (system::StartupManager::RequiresAdministrator(target.source)
                         ? machineRequest
                         : userRequest)
                        .disable.push_back(target);
                }

                for (const auto& target : request.enable)
                {
                    (system::StartupManager::RequiresAdministrator(target.source)
                         ? machineRequest
                         : userRequest)
                        .enable.push_back(target);
                }

                userRequest.restoreCurrentUser = request.restoreCurrentUser;
                machineRequest.restoreMachine = request.restoreMachine;

                if (!userRequest.IsEmpty())
                {
                    changes = system::StartupManager::Execute(userRequest);
                }

                if (!machineRequest.IsEmpty())
                {
                    std::vector<system::StartupChange> machineChanges;

                    if (elevated)
                    {
                        machineChanges =
                            system::StartupManager::Execute(machineRequest);
                    }
                    else
                    {
                        const system::ElevationStatus status =
                            system::RunStartupElevated(
                                machineRequest,
                                machineChanges);

                        if (status == system::ElevationStatus::Cancelled)
                        {
                            summary.elevationCancelled = true;

                            core::Logger::Warning(
                                L"Permissão de administrador recusada pelo usuário.");
                        }
                        else if (status == system::ElevationStatus::Failed)
                        {
                            summary.elevationFailed = true;

                            core::Logger::Error(
                                L"Não foi possível alterar a inicialização como administrador.");
                        }
                    }

                    changes.insert(
                        changes.end(),
                        machineChanges.begin(),
                        machineChanges.end());
                }

                for (const system::StartupChange& change : changes)
                {
                    switch (change.outcome)
                    {
                    case system::StartupOutcome::Changed:
                        ++summary.changed;
                        break;
                    case system::StartupOutcome::Unchanged:
                        ++summary.unchanged;
                        break;
                    case system::StartupOutcome::NotFound:
                        ++summary.notFound;
                        break;
                    case system::StartupOutcome::Failed:
                        ++summary.failed;
                        break;
                    }
                }

                dispatcher_.Post(
                    [this, summary]()
                    {
                        busy_ = false;

                        if (!shuttingDown_ && onApplyFinished)
                        {
                            onApplyFinished(summary);
                        }
                    });
            });
    }
}
