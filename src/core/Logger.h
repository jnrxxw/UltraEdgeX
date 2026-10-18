#pragma once

#include <string>

namespace real_optimizer::core
{
    // Log em arquivo, seguro para uso por várias threads
    // (e por mais de um processo ao mesmo tempo).
    //
    // Arquivo: %LOCALAPPDATA%\UltraEdgeX\logs\ultraedgex.log
    // Quando passa de 1 MB, o arquivo antigo vira ultraedgex.old.log.
    class Logger final
    {
    public:
        Logger() = delete;

        static void Info(const std::wstring& message);
        static void Warning(const std::wstring& message);
        static void Error(const std::wstring& message);

        [[nodiscard]] static std::wstring GetLogFilePath();

    private:
        static void Write(
            const wchar_t* level,
            const std::wstring& message);
    };
}
