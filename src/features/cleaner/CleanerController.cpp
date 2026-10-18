#include "features/cleaner/CleanerController.h"

#include "core/Logger.h"
#include "system/ElevatedCleaner.h"

#include <string>
#include <utility>

namespace real_optimizer::features::cleaner
{
    namespace
    {
        std::wstring Megabytes(std::uint64_t bytes)
        {
            return std::to_wstring(bytes / (1024ULL * 1024ULL)) +
                   L" MB";
        }

        void Accumulate(
            CleanSummary& summary,
            const system::CleanResult& result)
        {
            summary.removedBytes += result.removedBytes;
            summary.removedFiles += result.removedFiles;
            summary.failedBytes += result.failedBytes;
            summary.failedFiles += result.failedFiles;
        }
    }

    CleanerController::CleanerController(
        roui::UiDispatcher& dispatcher)
        : dispatcher_(dispatcher),
          elevated_(system::Cleaner::IsElevated())
    {
    }

    CleanerController::~CleanerController()
    {
        Shutdown();
    }

    const std::vector<system::CleanCategory>&
    CleanerController::Categories() const noexcept
    {
        return system::Cleaner::Categories();
    }

    bool CleanerController::IsBusy() const noexcept
    {
        return busy_;
    }

    bool CleanerController::IsElevated() const noexcept
    {
        return elevated_;
    }

    bool CleanerController::RestartAsAdministrator()
    {
        if (busy_ || elevated_)
        {
            return false;
        }

        core::Logger::Info(
            L"Reabrindo o app como administrador.");

        const bool started = system::RelaunchAsAdministrator();

        if (!started)
        {
            core::Logger::Warning(
                L"O pedido de administrador foi recusado ou falhou.");
        }

        return started;
    }

    void CleanerController::JoinWorker()
    {
        if (worker_.joinable())
        {
            worker_.join();
        }
    }

    void CleanerController::Shutdown()
    {
        shuttingDown_ = true;
        JoinWorker();
    }

    void CleanerController::StartScan()
    {
        if (busy_ || shuttingDown_)
        {
            return;
        }

        busy_ = true;
        JoinWorker();

        if (onScanStarted)
        {
            onScanStarted();
        }

        core::Logger::Info(L"Análise de limpeza iniciada.");

        worker_ = std::thread(
            [this]()
            {
                const auto& categories = system::Cleaner::Categories();

                std::uint64_t totalBytes = 0;

                for (std::size_t i = 0; i < categories.size(); ++i)
                {
                    const system::ScanResult result =
                        system::Cleaner::Scan(categories[i]);

                    totalBytes += result.bytes;

                    core::Logger::Info(
                        L"Análise: " + categories[i].name + L" = " +
                        Megabytes(result.bytes) +
                        (result.partial ? L" (parcial)" : L""));

                    dispatcher_.Post(
                        [this, i, result]()
                        {
                            if (!shuttingDown_ && onCategoryScanned)
                            {
                                onCategoryScanned(i, result);
                            }
                        });
                }

                core::Logger::Info(
                    L"Análise de limpeza concluída: " +
                    Megabytes(totalBytes) + L" encontrados.");

                dispatcher_.Post(
                    [this]()
                    {
                        busy_ = false;

                        if (!shuttingDown_ && onScanFinished)
                        {
                            onScanFinished();
                        }
                    });
            });
    }

    void CleanerController::StartClean(
        std::vector<std::size_t> indices)
    {
        if (busy_ || shuttingDown_ || indices.empty())
        {
            return;
        }

        const auto& categories = system::Cleaner::Categories();

        std::vector<system::CleanCategory> selected;

        for (const std::size_t index : indices)
        {
            if (index < categories.size())
            {
                selected.push_back(categories[index]);
            }
        }

        if (selected.empty())
        {
            return;
        }

        busy_ = true;
        JoinWorker();

        if (onCleanStarted)
        {
            onCleanStarted();
        }

        const bool elevated = elevated_;

        worker_ = std::thread(
            [this, selected, elevated]()
            {
                CleanSummary summary;
                std::vector<std::wstring> adminIds;

                core::Logger::Info(
                    L"Limpeza iniciada com " +
                    std::to_wstring(selected.size()) +
                    L" categoria(s).");

                for (const auto& category : selected)
                {
                    if (category.requiresAdmin && !elevated)
                    {
                        adminIds.push_back(category.id);
                        continue;
                    }

                    const system::CleanResult result =
                        system::Cleaner::Clean(category);

                    Accumulate(summary, result);

                    core::Logger::Info(
                        L"Limpeza: " + category.name + L": " +
                        Megabytes(result.removedBytes) +
                        L" removidos, " +
                        std::to_wstring(result.failedFiles) +
                        L" arquivos ignorados (em uso ou sem permissão).");
                }

                if (!adminIds.empty())
                {
                    core::Logger::Info(
                        L"Pedindo permissão de administrador para " +
                        std::to_wstring(adminIds.size()) +
                        L" categoria(s).");

                    std::vector<system::ElevatedCategoryResult> results;

                    const system::ElevationStatus status =
                        system::RunCleanElevated(adminIds, results);

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
                            L"Não foi possível concluir a limpeza como administrador.");
                    }
                    else
                    {
                        for (const auto& entry : results)
                        {
                            Accumulate(summary, entry.result);
                        }
                    }
                }

                core::Logger::Info(
                    L"Limpeza concluída: " +
                    Megabytes(summary.removedBytes) + L" removidos.");

                dispatcher_.Post(
                    [this, summary]()
                    {
                        busy_ = false;

                        if (!shuttingDown_ && onCleanFinished)
                        {
                            onCleanFinished(summary);
                        }
                    });
            });
    }
}
