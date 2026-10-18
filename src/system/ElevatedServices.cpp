#include "system/ElevatedServices.h"

#include "core/Logger.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace real_optimizer::system
{
    namespace
    {
        const std::wstring HelperSwitch = L"--services-elevated";

        std::wstring Widen(const std::string& text)
        {
            return std::wstring(text.begin(), text.end());
        }

        std::string Narrow(const std::wstring& text)
        {
            std::string result;

            for (const wchar_t c : text)
            {
                result.push_back(c < 128 ? static_cast<char>(c) : '?');
            }

            return result;
        }

        std::wstring Join(const std::vector<std::wstring>& items)
        {
            std::wstring text;

            for (std::size_t i = 0; i < items.size(); ++i)
            {
                text += (i > 0 ? L"," : L"") + items[i];
            }

            return text;
        }

        // Formato dos argumentos:  D:a,b  E:c  R
        std::wstring Encode(const ServiceRequest& request)
        {
            std::wstring arguments;

            if (!request.disable.empty())
            {
                arguments += L"D:" + Join(request.disable) + L" ";
            }

            if (!request.enable.empty())
            {
                arguments += L"E:" + Join(request.enable) + L" ";
            }

            if (request.restore)
            {
                arguments += L"R ";
            }

            return arguments;
        }

        ServiceRequest Decode(const std::wstring& arguments)
        {
            ServiceRequest request;

            std::wistringstream parser(arguments);
            std::wstring token;

            while (parser >> token)
            {
                if (token.rfind(L"D:", 0) == 0)
                {
                    request.disable = SplitList(token.substr(2));
                }
                else if (token.rfind(L"E:", 0) == 0)
                {
                    request.enable = SplitList(token.substr(2));
                }
                else if (token == L"R")
                {
                    request.restore = true;
                }
            }

            return request;
        }
    }

    bool IsServicesHelperCommand(const std::wstring& commandLine)
    {
        return IsHelperCommand(commandLine, HelperSwitch);
    }

    int RunServicesHelper(const std::wstring& commandLine)
    {
        HelperCommand command;

        if (!ParseHelperCommand(commandLine, HelperSwitch, command))
        {
            core::Logger::Warning(
                L"Ajudante de serviços recusou os parâmetros recebidos.");

            return 2;
        }

        const ServiceRequest request = Decode(command.arguments);

        if (request.disable.empty() &&
            request.enable.empty() &&
            !request.restore)
        {
            return 2;
        }

        core::Logger::Info(
            L"Alteração de serviços com permissão de administrador iniciada.");

        std::ofstream output(
            std::filesystem::path(command.resultPath),
            std::ios::trunc);

        if (!output)
        {
            core::Logger::Error(
                L"Ajudante de serviços não conseguiu gravar o resultado.");

            return 3;
        }

        // Só nomes da lista fixa são aceitos (Services::Execute).
        for (const ServiceChange& change : Services::Execute(request))
        {
            output << Narrow(change.name) << ' '
                   << static_cast<int>(change.outcome) << ' '
                   << change.error << '\n';
        }

        return 0;
    }

    ElevationStatus RunServicesElevated(
        const ServiceRequest& request,
        std::vector<ServiceChange>& changes)
    {
        changes.clear();

        const std::wstring arguments = Encode(request);

        if (arguments.empty())
        {
            return ElevationStatus::Failed;
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

            std::string name;
            int outcome = 0;
            unsigned long error = 0;

            if (parser >> name >> outcome >> error)
            {
                ServiceChange change;
                change.name = Widen(name);
                change.outcome = static_cast<ServiceOutcome>(outcome);
                change.error = error;
                changes.push_back(change);
            }
        }

        return changes.empty()
                   ? ElevationStatus::Failed
                   : ElevationStatus::Success;
    }
}
