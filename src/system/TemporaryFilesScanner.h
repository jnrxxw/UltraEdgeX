#pragma once

#include <cstdint>

namespace real_optimizer::system
{
    struct TemporaryFilesInfo
    {
        std::uint64_t totalBytes = 0;
        std::uint64_t fileCount = 0;
        bool valid = false;
    };

    class TemporaryFilesScanner final
    {
    public:
        TemporaryFilesScanner() = delete;

        [[nodiscard]]
        static TemporaryFilesInfo Scan() noexcept;
    };
}