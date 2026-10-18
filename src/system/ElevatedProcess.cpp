#include "system/ElevatedProcess.h"

#include <windows.h>
#include <objbase.h>
#include <shellapi.h>

#include <cwctype>
#include <filesystem>
#include <fstream>

namespace real_optimizer::system
{
    namespace
    {
        std::wstring ExpandVariables(const std::wstring& text)
        {
            const DWORD needed =
                ExpandEnvironmentStringsW(text.c_str(), nullptr, 0);

            if (needed == 0)
            {
                return {};
            }

            std::wstring result(needed, L'\0');

            const DWORD written = ExpandEnvironmentStringsW(
                text.c_str(),
                result.data(),
                needed);

            if (written == 0 || written > needed)
            {
                return {};
            }

            result.resize(written - 1);
            return result;
        }

        std::wstring AppDirectory()
        {
            return ExpandVariables(L"%LOCALAPPDATA%\\UltraEdgeX");
        }

        bool StartsWithInsensitive(
            const std::wstring& text,
            const std::wstring& prefix)
        {
            if (prefix.empty() || text.size() < prefix.size())
            {
                return false;
            }

            for (std::size_t i = 0; i < prefix.size(); ++i)
            {
                if (std::towlower(text[i]) != std::towlower(prefix[i]))
                {
                    return false;
                }
            }

            return true;
        }
    }

    std::vector<std::wstring> SplitList(const std::wstring& text)
    {
        std::vector<std::wstring> items;
        std::wstring current;

        for (const wchar_t c : text)
        {
            if (c == L',')
            {
                if (!current.empty())
                {
                    items.push_back(current);
                }

                current.clear();
            }
            else if (c != L' ' && c != L'\t')
            {
                current.push_back(c);
            }
        }

        if (!current.empty())
        {
            items.push_back(current);
        }

        return items;
    }

    bool IsHelperCommand(
        const std::wstring& commandLine,
        const std::wstring& helperSwitch)
    {
        return commandLine.rfind(helperSwitch, 0) == 0;
    }

    bool ParseHelperCommand(
        const std::wstring& commandLine,
        const std::wstring& helperSwitch,
        HelperCommand& command)
    {
        if (!IsHelperCommand(commandLine, helperSwitch))
        {
            return false;
        }

        const std::size_t quoteStart = commandLine.find(L'"');

        if (quoteStart == std::wstring::npos)
        {
            return false;
        }

        const std::size_t quoteEnd =
            commandLine.find(L'"', quoteStart + 1);

        if (quoteEnd == std::wstring::npos)
        {
            return false;
        }

        command.resultPath =
            commandLine.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

        command.arguments = commandLine.substr(quoteEnd + 1);

        const std::wstring directory = AppDirectory();

        const bool pathAllowed =
            !directory.empty() &&
            StartsWithInsensitive(command.resultPath, directory + L"\\") &&
            command.resultPath.find(L"..") == std::wstring::npos &&
            command.resultPath.size() > 7 &&
            command.resultPath.compare(
                command.resultPath.size() - 7, 7, L".result") == 0;

        return pathAllowed;
    }

    bool RelaunchElevated(const wchar_t* arguments)
    {
        wchar_t executable[MAX_PATH]{};

        if (GetModuleFileNameW(nullptr, executable, MAX_PATH) == 0)
        {
            return false;
        }

        const HRESULT comResult = CoInitializeEx(
            nullptr,
            COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        SHELLEXECUTEINFOW info{};
        info.cbSize = sizeof(info);
        info.lpVerb = L"runas";
        info.lpFile = executable;
        info.lpParameters = arguments;
        info.nShow = SW_SHOWNORMAL;

        const BOOL started = ShellExecuteExW(&info);

        if (SUCCEEDED(comResult))
        {
            CoUninitialize();
        }

        return started != FALSE;
    }

    ElevationStatus RunSelfElevated(
        const std::wstring& helperSwitch,
        const std::wstring& arguments,
        std::vector<std::string>& resultLines)
    {
        resultLines.clear();

        const std::wstring directory = AppDirectory();

        if (directory.empty())
        {
            return ElevationStatus::Failed;
        }

        CreateDirectoryW(directory.c_str(), nullptr);

        LARGE_INTEGER qpc{};
        QueryPerformanceCounter(&qpc);

        const std::wstring resultPath =
            directory + L"\\task-" +
            std::to_wstring(GetCurrentProcessId()) + L"-" +
            std::to_wstring(qpc.QuadPart) + L".result";

        const std::wstring parameters =
            helperSwitch + L" \"" + resultPath + L"\" " + arguments;

        wchar_t executable[MAX_PATH]{};

        if (GetModuleFileNameW(nullptr, executable, MAX_PATH) == 0)
        {
            return ElevationStatus::Failed;
        }

        // O ShellExecute pode precisar de COM nesta thread.
        const HRESULT comResult = CoInitializeEx(
            nullptr,
            COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        SHELLEXECUTEINFOW info{};
        info.cbSize = sizeof(info);
        info.fMask = SEE_MASK_NOCLOSEPROCESS;
        info.lpVerb = L"runas";
        info.lpFile = executable;
        info.lpParameters = parameters.c_str();
        info.nShow = SW_HIDE;

        const BOOL started = ShellExecuteExW(&info);
        const DWORD startError = started ? 0 : GetLastError();

        ElevationStatus status = ElevationStatus::Failed;

        if (!started)
        {
            status = startError == ERROR_CANCELLED
                         ? ElevationStatus::Cancelled
                         : ElevationStatus::Failed;
        }
        else
        {
            if (info.hProcess != nullptr)
            {
                WaitForSingleObject(info.hProcess, INFINITE);
                CloseHandle(info.hProcess);
            }

            std::ifstream input{std::filesystem::path(resultPath)};
            std::string line;

            while (std::getline(input, line))
            {
                if (!line.empty())
                {
                    resultLines.push_back(line);
                }
            }

            input.close();
            DeleteFileW(resultPath.c_str());

            status = resultLines.empty()
                         ? ElevationStatus::Failed
                         : ElevationStatus::Success;
        }

        if (SUCCEEDED(comResult))
        {
            CoUninitialize();
        }

        return status;
    }
}
