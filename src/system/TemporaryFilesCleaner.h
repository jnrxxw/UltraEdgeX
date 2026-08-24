#pragma once

#include <cstdint>

namespace real_optimizer::system
{
    struct TemporaryCleanupResult
    {
        std::uint64_t removedBytes = 0;
        std::uint64_t removedFiles = 0;

        std::uint64_t failedBytes = 0;
        std::uint64_t failedFiles = 0;

        bool valid = false;
    };

    class TemporaryFilesCleaner final
    {
    public:
        TemporaryFilesCleaner() = delete;

        [[nodiscard]]
        static TemporaryCleanupResult Clean() noexcept;
    };
}