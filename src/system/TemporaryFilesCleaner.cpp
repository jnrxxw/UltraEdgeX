#include "system/TemporaryFilesCleaner.h"

#include <windows.h>

#include <filesystem>
#include <system_error>
#include <vector>

namespace real_optimizer::system
{
    TemporaryCleanupResult
    TemporaryFilesCleaner::Clean() noexcept
    {
        TemporaryCleanupResult result{};

        wchar_t tempPath[MAX_PATH]{};

        const DWORD pathLength =
            GetTempPathW(
                MAX_PATH,
                tempPath);

        if (
            pathLength == 0 ||
            pathLength >= MAX_PATH)
        {
            return result;
        }

        const std::filesystem::path rootPath(
            tempPath);

        std::error_code error;

        if (!std::filesystem::exists(
                rootPath,
                error))
        {
            return result;
        }

        //
        // Primeiro coletamos os arquivos.
        //
        // Não excluímos enquanto percorremos
        // o iterator para evitar invalidá-lo.
        //

        std::vector<std::filesystem::path>
            files;

        std::vector<std::filesystem::path>
            directories;

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

            const auto path =
                iterator->path();

            std::error_code entryError;

            if (iterator->is_regular_file(
                    entryError))
            {
                files.push_back(path);
            }
            else if (
                iterator->is_directory(
                    entryError))
            {
                directories.push_back(path);
            }

            iterator.increment(error);
        }

        //
        // Arquivos
        //

        for (const auto &file : files)
        {
            std::error_code sizeError;

            const auto fileSize =
                std::filesystem::file_size(
                    file,
                    sizeError);

            const std::uint64_t size =
                sizeError
                    ? 0
                    : static_cast<std::uint64_t>(
                          fileSize);

            std::error_code removeError;

            const bool removed =
                std::filesystem::remove(
                    file,
                    removeError);

            if (
                removed &&
                !removeError)
            {
                result.removedBytes += size;
                ++result.removedFiles;
            }
            else
            {
                result.failedBytes += size;
                ++result.failedFiles;
            }
        }

        //
        // Diretórios vazios
        //
        // Fazemos de trás para frente para
        // remover primeiro os mais internos.
        //

        for (
            auto iterator =
                directories.rbegin();

            iterator !=
            directories.rend();

            ++iterator)
        {
            std::error_code removeError;

            std::filesystem::remove(
                *iterator,
                removeError);
        }

        result.valid = true;

        return result;
    }
}