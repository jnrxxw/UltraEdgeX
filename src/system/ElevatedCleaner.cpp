#include "system/ElevatedCleaner.h"

#include "core/Logger.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace real_optimizer::system
{
    namespace
    {
        const std::wstring HelperSwitch = L"--clean-elevated";

        std::wstring Widen(const std::string& text)
        {
            return std::wstring(text.begin(), text.end());
        }

        std::string Narrow(const std::wstring& text)
        {
            std::string result;

            for (const wchar_t c : text)
            {
                result.push_back(
                    c < 128 ? static_cast<char>(c) : '?');
            }

            return result;
        }
    }

    bool IsElevatedHelperCommand(const std::wstring& commandLine)
    {
        return IsHelperCommand(commandLine, HelperSwitch);
    }

    bool RelaunchAsAdministrator()
    {
        return RelaunchElevated(L"--open-cleaner");
    }

    int RunElevatedHelper(const std::wstring& commandLine)
    {
        // Formato: --clean-elevated "<arquivo de resultado>" id1,id2
        HelperCommand command;

        if (!ParseHelperCommand(commandLine, HelperSwitch, command))
        {
            core::Logger::Warning(
                L"Ajudante elevado recusou os parâmetros recebidos.");

            return 2;
        }

        const std::vector<std::wstring> ids =
            SplitList(command.arguments);

        if (ids.empty())
        {
            return 2;
        }

        core::Logger::Info(
            L"Limpeza com permissão de administrador iniciada.");

        std::ofstream output(
            std::filesystem::path(command.resultPath),
            std::ios::trunc);

        if (!output)
        {
            core::Logger::Error(
                L"Ajudante elevado não conseguiu gravar o resultado.");

            return 3;
        }

        for (const auto& id : ids)
        {
            // Só categorias da lista fixa.
            const CleanCategory* category = Cleaner::FindCategory(id);

            if (category == nullptr)
            {
                continue;
            }

            const CleanResult result = Cleaner::Clean(*category);

            core::Logger::Info(
                L"[admin] " + category->name + L": " +
                std::to_wstring(result.removedBytes / (1024 * 1024)) +
                L" MB removidos, " +
                std::to_wstring(result.failedFiles) +
                L" arquivos ignorados.");

            output << Narrow(id) << ' '
                   << result.removedBytes << ' '
                   << result.removedFiles << ' '
                   << result.failedBytes << ' '
                   << result.failedFiles << '\n';
        }

        return 0;
    }

    ElevationStatus RunCleanElevated(
        const std::vector<std::wstring>& categoryIds,
        std::vector<ElevatedCategoryResult>& results)
    {
        results.clear();

        if (categoryIds.empty())
        {
            return ElevationStatus::Failed;
        }

        std::wstring arguments;

        for (std::size_t i = 0; i < categoryIds.size(); ++i)
        {
            arguments += (i > 0 ? L"," : L"") + categoryIds[i];
        }

        std::vector<std::string> lines;

        const ElevationStatus status =
            RunSelfElevated(HelperSwitch, arguments, lines);

        if (status != ElevationStatus::Success)
        {
            return status;
        }

        for (const std::string& line : lines)
        {
            std::istringstream parser(line);

            std::string id;
            ElevatedCategoryResult entry;

            if (parser >> id >> entry.result.removedBytes >>
                entry.result.removedFiles >>
                entry.result.failedBytes >>
                entry.result.failedFiles)
            {
                entry.id = Widen(id);
                results.push_back(entry);
            }
        }

        return results.empty()
                   ? ElevationStatus::Failed
                   : ElevationStatus::Success;
    }
}
