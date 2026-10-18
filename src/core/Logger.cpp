#include "core/Logger.h"

#include <windows.h>

#include <mutex>
#include <string>

namespace real_optimizer::core
{
    namespace
    {
        constexpr unsigned long long MaxLogSize = 1024ULL * 1024ULL;

        std::wstring BuildDirectory()
        {
            wchar_t buffer[MAX_PATH]{};

            const DWORD length = GetEnvironmentVariableW(
                L"LOCALAPPDATA",
                buffer,
                MAX_PATH);

            if (length == 0 || length >= MAX_PATH)
            {
                return {};
            }

            return std::wstring(buffer) + L"\\UltraEdgeX\\logs";
        }

        std::wstring Directory()
        {
            static const std::wstring directory = BuildDirectory();
            return directory;
        }

        std::wstring FilePath()
        {
            const std::wstring directory = Directory();

            return directory.empty()
                       ? std::wstring()
                       : directory + L"\\ultraedgex.log";
        }

        // Cria as pastas e faz a rotação uma vez por execução.
        void Prepare()
        {
            static std::once_flag once;

            std::call_once(
                once,
                []()
                {
                    const std::wstring directory = Directory();

                    if (directory.empty())
                    {
                        return;
                    }

                    const std::size_t separator =
                        directory.find_last_of(L'\\');

                    CreateDirectoryW(
                        directory.substr(0, separator).c_str(),
                        nullptr);

                    CreateDirectoryW(directory.c_str(), nullptr);

                    WIN32_FILE_ATTRIBUTE_DATA data{};

                    if (GetFileAttributesExW(
                            FilePath().c_str(),
                            GetFileExInfoStandard,
                            &data))
                    {
                        const unsigned long long size =
                            (static_cast<unsigned long long>(
                                 data.nFileSizeHigh)
                             << 32) |
                            data.nFileSizeLow;

                        if (size > MaxLogSize)
                        {
                            MoveFileExW(
                                FilePath().c_str(),
                                (directory + L"\\ultraedgex.old.log").c_str(),
                                MOVEFILE_REPLACE_EXISTING);
                        }
                    }
                });
        }

        std::string ToUtf8(const std::wstring& text)
        {
            if (text.empty())
            {
                return {};
            }

            const int size = WideCharToMultiByte(
                CP_UTF8,
                0,
                text.data(),
                static_cast<int>(text.size()),
                nullptr,
                0,
                nullptr,
                nullptr);

            if (size <= 0)
            {
                return {};
            }

            std::string result(static_cast<std::size_t>(size), '\0');

            WideCharToMultiByte(
                CP_UTF8,
                0,
                text.data(),
                static_cast<int>(text.size()),
                result.data(),
                size,
                nullptr,
                nullptr);

            return result;
        }
    }

    std::wstring Logger::GetLogFilePath()
    {
        return FilePath();
    }

    void Logger::Info(const std::wstring& message)
    {
        Write(L"INFO", message);
    }

    void Logger::Warning(const std::wstring& message)
    {
        Write(L"AVISO", message);
    }

    void Logger::Error(const std::wstring& message)
    {
        Write(L"ERRO", message);
    }

    void Logger::Write(
        const wchar_t* level,
        const std::wstring& message)
    {
        const std::wstring path = FilePath();

        if (path.empty())
        {
            return;
        }

        Prepare();

        SYSTEMTIME time{};
        GetLocalTime(&time);

        wchar_t prefix[64]{};

        swprintf(
            prefix,
            64,
            L"%04d-%02d-%02d %02d:%02d:%02d [%ls] ",
            time.wYear,
            time.wMonth,
            time.wDay,
            time.wHour,
            time.wMinute,
            time.wSecond,
            level);

        const std::string line =
            ToUtf8(std::wstring(prefix) + message + L"\r\n");

        if (line.empty())
        {
            return;
        }

        // Abre, escreve em modo append e fecha. Simples e seguro
        // mesmo com mais de um processo escrevendo.
        const HANDLE file = CreateFileW(
            path.c_str(),
            FILE_APPEND_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (file == INVALID_HANDLE_VALUE)
        {
            return;
        }

        DWORD written = 0;

        WriteFile(
            file,
            line.data(),
            static_cast<DWORD>(line.size()),
            &written,
            nullptr);

        CloseHandle(file);
    }
}
