#include "system/SystemAnalyzer.h"

#include "system/SystemInfo.h"
#include "core/ByteFormat.h"
#include "system/Cleaner.h"
#include "system/Services.h"

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

        // Mesmo critério da página Limpeza: soma as categorias
        // que ela já deixa marcadas (nível Seguro).
        std::uint64_t cleanableBytes = 0;
        bool cleanablePartial = false;

        for (const CleanCategory &category : Cleaner::Categories())
        {
            if (
                category.safety == SafetyLevel::Safe &&
                category.selectedByDefault)
            {
                const ScanResult scan = Cleaner::Scan(category);

                cleanableBytes += scan.bytes;
                cleanablePartial = cleanablePartial || scan.partial;
            }
        }

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

        if (cleanableBytes >= TemporaryFilesThreshold)
        {
            result.issues.push_back({L"Arquivos temporários",
                                     std::wstring(cleanablePartial
                                                      ? L"Pelo menos "
                                                      : L"") +
                                         core::FormatBytes(cleanableBytes) +
                                         (cleanablePartial
                                              ? L" para remover."
                                              : L" podem ser removidos."),
                                     IssueSeverity::Warning,
                                     5,
                                     IssueAction::OpenCleaner});
        }

        //
        // SERVIÇOS DESNECESSÁRIOS
        //
        // Só conta serviços de nível Seguro (mesmo critério do botão
        // "Marcar recomendados"). Não promete ganho de desempenho:
        // o efeito de desativar serviços é pequeno.
        //

        const std::size_t unnecessaryServices =
            Services::CountRecommendedToDisable();

        if (unnecessaryServices > 0)
        {
            result.issues.push_back({unnecessaryServices == 1
                                         ? L"1 serviço desnecessário"
                                         : std::to_wstring(unnecessaryServices) +
                                               L" serviços desnecessários",
                                     L"Podem ser desativados com segurança.",
                                     IssueSeverity::Info,
                                     3,
                                     IssueAction::OpenServices});
        }

        //
        // PONTUAÇÃO
        //

        for (const AnalysisIssue &issue : result.issues)
        {
            result.healthScore -=
                issue.scorePenalty;
        }

        // Os mais graves primeiro: o Painel mostra só os primeiros.
        std::stable_sort(
            result.issues.begin(),
            result.issues.end(),
            [](const AnalysisIssue &a, const AnalysisIssue &b)
            {
                if (a.severity != b.severity)
                {
                    return static_cast<int>(a.severity) >
                           static_cast<int>(b.severity);
                }

                return a.scorePenalty > b.scorePenalty;
            });

        result.healthScore =
            std::clamp(
                result.healthScore,
                0,
                100);

        return result;
    }
}