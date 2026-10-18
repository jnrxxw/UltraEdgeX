#include "system/ElevatedStartup.h"

#include "core/Logger.h"

#include <windows.h>

#include <filesystem>
#include <fstream>
#include <sstream>

namespace real_optimizer::system
{
    namespace
    {
        const std::wstring HelperSwitch = L"--startup-elevated";

        // Nomes de entradas podem ter espaço, vírgula e acentos.
        // Na linha de comando e no arquivo de resultado eles viajam
        // como texto hexadecimal do UTF-8, que só tem [0-9a-f].
        // Formato do identificador:  M-<hex>  ou  W-<hex>

        std::string ToUtf8(const std::wstring& text)
        {
            if (text.empty())
            {
                return {};
            }

            const int size = WideCharToMultiByte(
                CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                nullptr, 0, nullptr, nullptr);

            if (size <= 0)
            {
                return {};
            }

            std::string result(static_cast<std::size_t>(size), '\0');

            WideCharToMultiByte(
                CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                result.data(), size, nullptr, nullptr);

            return result;
        }

        std::wstring FromUtf8(const std::string& text)
        {
            if (text.empty())
            {
                return {};
            }

            const int size = MultiByteToWideChar(
                CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                nullptr, 0);

            if (size <= 0)
            {
                return {};
            }

            std::wstring result(static_cast<std::size_t>(size), L'\0');

            MultiByteToWideChar(
                CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                result.data(), size);

            return result;
        }

        std::string ToHex(const std::string& bytes)
        {
            static const char Digits[] = "0123456789abcdef";

            std::string hex;
            hex.reserve(bytes.size() * 2);

            for (const char c : bytes)
            {
                const auto value = static_cast<unsigned char>(c);
                hex.push_back(Digits[value >> 4]);
                hex.push_back(Digits[value & 0x0F]);
            }

            return hex;
        }

        int HexValue(char c)
        {
            if (c >= '0' && c <= '9')
            {
                return c - '0';
            }

            if (c >= 'a' && c <= 'f')
            {
                return c - 'a' + 10;
            }

            return -1;
        }

        bool FromHex(const std::string& hex, std::string& bytes)
        {
            bytes.clear();

            if (hex.empty() || hex.size() % 2 != 0)
            {
                return false;
            }

            for (std::size_t i = 0; i < hex.size(); i += 2)
            {
                const int high = HexValue(hex[i]);
                const int low = HexValue(hex[i + 1]);

                if (high < 0 || low < 0)
                {
                    return false;
                }

                bytes.push_back(static_cast<char>((high << 4) | low));
            }

            return true;
        }

        std::string Encode(const StartupTarget& target)
        {
            const char code =
                target.source == StartupSource::LocalMachine32 ? 'W' : 'M';

            return std::string(1, code) + "-" + ToHex(ToUtf8(target.name));
        }

        // Só entradas de HKLM são aceitas no ajudante elevado.
        bool Decode(const std::string& id, StartupTarget& target)
        {
            if (id.size() < 3 || id[1] != '-')
            {
                return false;
            }

            if (id[0] == 'M')
            {
                target.source = StartupSource::LocalMachine;
            }
            else if (id[0] == 'W')
            {
                target.source = StartupSource::LocalMachine32;
            }
            else
            {
                return false;
            }

            std::string bytes;

            if (!FromHex(id.substr(2), bytes))
            {
                return false;
            }

            target.name = FromUtf8(bytes);

            return !target.name.empty();
        }

        std::wstring Widen(const std::string& text)
        {
            return std::wstring(text.begin(), text.end());
        }

        std::wstring Join(const std::vector<StartupTarget>& items)
        {
            std::wstring text;

            for (std::size_t i = 0; i < items.size(); ++i)
            {
                text += (i > 0 ? L"," : L"") + Widen(Encode(items[i]));
            }

            return text;
        }

        // Formato dos argumentos:  D:a,b  E:c  RM
        std::wstring EncodeRequest(const StartupRequest& request)
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

            if (request.restoreMachine)
            {
                arguments += L"RM ";
            }

            return arguments;
        }

        void DecodeList(
            const std::wstring& text,
            std::vector<StartupTarget>& targets)
        {
            for (const std::wstring& item : SplitList(text))
            {
                std::string narrow;

                for (const wchar_t c : item)
                {
                    narrow.push_back(c < 128 ? static_cast<char>(c) : '?');
                }

                StartupTarget target;

                if (Decode(narrow, target))
                {
                    targets.push_back(std::move(target));
                }
            }
        }

        StartupRequest DecodeRequest(const std::wstring& arguments)
        {
            StartupRequest request;

            std::wistringstream parser(arguments);
            std::wstring token;

            while (parser >> token)
            {
                if (token.rfind(L"D:", 0) == 0)
                {
                    DecodeList(token.substr(2), request.disable);
                }
                else if (token.rfind(L"E:", 0) == 0)
                {
                    DecodeList(token.substr(2), request.enable);
                }
                else if (token == L"RM")
                {
                    request.restoreMachine = true;
                }
            }

            // O ajudante nunca mexe em HKCU.
            request.restoreCurrentUser = false;

            return request;
        }
    }

    bool IsStartupHelperCommand(const std::wstring& commandLine)
    {
        return IsHelperCommand(commandLine, HelperSwitch);
    }

    int RunStartupHelper(const std::wstring& commandLine)
    {
        HelperCommand command;

        if (!ParseHelperCommand(commandLine, HelperSwitch, command))
        {
            core::Logger::Warning(
                L"Ajudante de inicialização recusou os parâmetros recebidos.");

            return 2;
        }

        const StartupRequest request = DecodeRequest(command.arguments);

        if (request.IsEmpty())
        {
            return 2;
        }

        core::Logger::Info(
            L"Alteração de inicialização com permissão de administrador iniciada.");

        std::ofstream output(
            std::filesystem::path(command.resultPath),
            std::ios::trunc);

        if (!output)
        {
            core::Logger::Error(
                L"Ajudante de inicialização não conseguiu gravar o resultado.");

            return 3;
        }

        // Só entradas que existem agora nas chaves Run de HKLM são
        // alteradas (StartupManager::Execute).
        for (const StartupChange& change : StartupManager::Execute(request))
        {
            output << Encode(change.target) << ' '
                   << static_cast<int>(change.outcome) << ' '
                   << change.error << '\n';
        }

        return 0;
    }

    ElevationStatus RunStartupElevated(
        const StartupRequest& request,
        std::vector<StartupChange>& changes)
    {
        changes.clear();

        const std::wstring arguments = EncodeRequest(request);

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

            std::string id;
            int outcome = 0;
            unsigned long error = 0;

            if (!(parser >> id >> outcome >> error))
            {
                continue;
            }

            StartupChange change;

            if (!Decode(id, change.target))
            {
                continue;
            }

            change.outcome = static_cast<StartupOutcome>(outcome);
            change.error = error;
            changes.push_back(std::move(change));
        }

        return changes.empty()
                   ? ElevationStatus::Failed
                   : ElevationStatus::Success;
    }
}
