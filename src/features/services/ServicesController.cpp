#include "features/services/ServicesController.h"

#include "core/Logger.h"
#include "system/Cleaner.h"
#include "system/ElevatedServices.h"

#include <string>
#include <utility>

namespace real_optimizer::features::services
{
    ServicesController::ServicesController(
        roui::UiDispatcher& dispatcher)
        : dispatcher_(dispatcher),
          elevated_(system::Cleaner::IsElevated())
    {
    }

    ServicesController::~ServicesController()
    {
        Shutdown();
    }

    const std::vector<system::ServiceDefinition>&
    ServicesController::Definitions() const noexcept
    {
        return system::Services::Definitions();
    }

    bool ServicesController::IsBusy() const noexcept
    {
        return busy_;
    }

    bool ServicesController::IsElevated() const noexcept
    {
        return elevated_;
    }

    std::vector<std::wstring> ServicesController::BackedUpNames() const
    {
        return system::Services::BackedUpNames();
    }

    void ServicesController::JoinWorker()
    {
        if (worker_.joinable())
        {
            worker_.join();
        }
    }

    void ServicesController::Shutdown()
    {
        shuttingDown_ = true;
        JoinWorker();
    }

    void ServicesController::StartRefresh()
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
                const auto& definitions =
                    system::Services::Definitions();

                for (std::size_t i = 0; i < definitions.size(); ++i)
                {
                    const system::ServiceState state =
                        system::Services::Query(definitions[i].name);

                    dispatcher_.Post(
                        [this, i, state]()
                        {
                            if (!shuttingDown_ && onServiceLoaded)
                            {
                                onServiceLoaded(i, state);
                            }
                        });
                }

                dispatcher_.Post(
                    [this]()
                    {
                        busy_ = false;

                        if (!shuttingDown_ && onRefreshFinished)
                        {
                            onRefreshFinished();
                        }
                    });
            });
    }

    void ServicesController::StartApply(
        std::vector<std::size_t> disable,
        std::vector<std::size_t> enable)
    {
        const auto& definitions = system::Services::Definitions();

        system::ServiceRequest request;

        for (const std::size_t index : disable)
        {
            if (index < definitions.size())
            {
                request.disable.push_back(definitions[index].name);
            }
        }

        for (const std::size_t index : enable)
        {
            if (index < definitions.size())
            {
                request.enable.push_back(definitions[index].name);
            }
        }

        if (request.disable.empty() && request.enable.empty())
        {
            return;
        }

        StartChange(std::move(request));
    }

    void ServicesController::StartRestore()
    {
        system::ServiceRequest request;
        request.restore = true;

        StartChange(std::move(request));
    }

    void ServicesController::StartChange(
        system::ServiceRequest request)
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
                std::vector<system::ServiceChange> changes;

                core::Logger::Info(
                    L"Alteração de serviços iniciada: " +
                    std::to_wstring(request.disable.size()) +
                    L" para desativar, " +
                    std::to_wstring(request.enable.size()) +
                    L" para reativar" +
                    (request.restore ? L", restaurar tudo." : L"."));

                if (elevated)
                {
                    changes = system::Services::Execute(request);
                }
                else
                {
                    const system::ElevationStatus status =
                        system::RunServicesElevated(request, changes);

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
                            L"Não foi possível alterar os serviços como administrador.");
                    }
                }

                for (const system::ServiceChange& change : changes)
                {
                    switch (change.outcome)
                    {
                    case system::ServiceOutcome::Changed:
                        ++summary.changed;
                        break;
                    case system::ServiceOutcome::Unchanged:
                        ++summary.unchanged;
                        break;
                    case system::ServiceOutcome::NotInstalled:
                        ++summary.notInstalled;
                        break;
                    case system::ServiceOutcome::InUse:
                        ++summary.inUse;
                        break;
                    case system::ServiceOutcome::Failed:
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
