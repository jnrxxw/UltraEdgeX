#include "system/TemporaryFilesScanner.h"

#include <windows.h>

#include <filesystem>
#include <system_error>

namespace real_optimizer::system
{
    TemporaryFilesInfo
    TemporaryFilesScanner::Scan() noexcept
    {
        TemporaryFilesInfo info{};

        wchar_t tempPath[MAX_PATH]{};

        const DWORD pathLength =
            GetTempPathW(
                MAX_PATH,
                tempPath);

        if (
            pathLength == 0 ||
            pathLength >= MAX_PATH)
        {
            return info;
        }

        const std::filesystem::path rootPath(
            tempPath);

        std::error_code error;

        if (!std::filesystem::exists(
                rootPath,
                error))
        {
            return info;
        }

        error.clear();

        std::filesystem::recursive_directory_iterator iterator(
            rootPath,
            std::filesystem::directory_options::
                skip_permission_denied,
            error);

        const std::filesystem::
            recursive_directory_iterator end;

        while (iterator != end)
        {
            if (error)
            {
                error.clear();

                iterator.increment(error);
                continue;
            }

            const auto &entry = *iterator;

            std::error_code fileError;

            if (entry.is_regular_file(fileError))
            {
                const auto size =
                    entry.file_size(fileError);

                if (!fileError)
                {
                    info.totalBytes +=
                        static_cast<std::uint64_t>(
                            size);

                    ++info.fileCount;
                }
            }

            iterator.increment(error);
        }

        info.valid = true;

        return info;
    }
}