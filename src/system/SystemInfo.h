#pragma once

#include <cstdint>
#include <string>

namespace real_optimizer::system
{
    struct MemoryInfo
    {
        std::uint64_t totalBytes = 0;
        std::uint64_t availableBytes = 0;
        unsigned int usagePercent = 0;
    };

    struct DiskInfo
    {
        std::uint64_t totalBytes = 0;
        std::uint64_t freeBytes = 0;
    };

    struct CpuInfo
    {
        unsigned int usagePercent = 0;
        bool valid = false;
    };

    struct SystemDetails
    {
        std::wstring operatingSystem;
        std::wstring processorName;
        std::wstring architecture;
        std::wstring computerName;

        std::uint64_t totalMemoryBytes = 0;
    };

    class SystemInfo final
    {
    public:
        SystemInfo() = delete;

        [[nodiscard]]
        static MemoryInfo GetMemoryInfo() noexcept;

        [[nodiscard]]
        static DiskInfo GetSystemDiskInfo() noexcept;

        [[nodiscard]]
        static CpuInfo GetCpuInfo() noexcept;

        [[nodiscard]]
        static SystemDetails GetSystemDetails();
    };
}