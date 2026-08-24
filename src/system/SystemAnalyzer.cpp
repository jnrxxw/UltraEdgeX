#include "system/SystemAnalyzer.h"

#include "system/SystemInfo.h"
#include "system/TemporaryFilesScanner.h"

#include <algorithm>
#include <cstdint>
#include <string>

namespace real_optimizer::system
{
    AnalysisResult SystemAnalyzer::Analyze()
    {
        AnalysisResult result{};

        const MemoryInfo memory =
            SystemInfo::GetMemoryInfo();

        const DiskInfo disk =
            SystemInfo::GetSystemDiskInfo();

        const CpuInfo cpu =
            SystemInfo::GetCpuInfo();

        const TemporaryFilesInfo temporaryFiles =
            TemporaryFilesScanner::Scan();

        //
        // MEMÓRIA
        //

        if (memory.usagePercent >= 90)
        {
            result.issues.push_back({L"Uso elevado de memória",
                                     L"A memória está com mais de 90% de utilização.",
                                     IssueSeverity::Critical,
                                     20});
        }
        else if (memory.usagePercent >= 80)
        {
            result.issues.push_back({L"Uso de memória acima do recomendado",
                                     L"A memória está com mais de 80% de utilização.",
                                     IssueSeverity::Warning,
                                     10});
        }

        //
        // PROCESSADOR
        //

        if (cpu.valid)
        {
            if (cpu.usagePercent >= 90)
            {
                result.issues.push_back({L"Uso elevado do processador",
                                         L"O processador está com mais de 90% de utilização.",
                                         IssueSeverity::Critical,
                                         15});
            }
            else if (cpu.usagePercent >= 80)
            {
                result.issues.push_back({L"Uso do processador acima do recomendado",
                                         L"O processador está com mais de 80% de utilização.",
                                         IssueSeverity::Warning,
                                         8});
            }
        }

        //
        // DISCO DO SISTEMA
        //

        if (disk.totalBytes > 0)
        {
            const double freePercent =
                (static_cast<double>(disk.freeBytes) /
                 static_cast<double>(disk.totalBytes)) *
                100.0;

            if (freePercent < 10.0)
            {
                result.issues.push_back({L"Espaço em disco crítico",
                                         L"O disco do sistema possui menos de 10% de espaço livre.",
                                         IssueSeverity::Critical,
                                         25});
            }
            else if (freePercent < 20.0)
            {
                result.issues.push_back({L"Pouco espaço em disco",
                                         L"O disco do sistema possui menos de 20% de espaço livre.",
                                         IssueSeverity::Warning,
                                         12});
            }
        }

        //
        // ARQUIVOS TEMPORÁRIOS
        //

        constexpr std::uint64_t Megabyte =
            1024ULL * 1024ULL;

        constexpr std::uint64_t TemporaryFilesThreshold =
            100ULL * Megabyte;

        if (
            temporaryFiles.valid &&
            temporaryFiles.totalBytes >= TemporaryFilesThreshold)
        {
            const std::uint64_t sizeMb =
                temporaryFiles.totalBytes / Megabyte;

            result.issues.push_back({L"Arquivos temporários",
                                     std::to_wstring(sizeMb) +
                                         L" MB de arquivos temporários encontrados.",
                                     IssueSeverity::Warning,
                                     5});
        }

        //
        // PONTUAÇÃO
        //

        for (const AnalysisIssue &issue : result.issues)
        {
            result.healthScore -=
                issue.scorePenalty;
        }

        result.healthScore =
            std::clamp(
                result.healthScore,
                0,
                100);

        return result;
    }
}