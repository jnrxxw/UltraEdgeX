#include "system/SystemInfo.h"

#include <windows.h>

#include <string>

namespace
{
    std::uint64_t FileTimeToUint64(
        const FILETIME &time) noexcept
    {
        ULARGE_INTEGER value{};

        value.LowPart = time.dwLowDateTime;
        value.HighPart = time.dwHighDateTime;

        return value.QuadPart;
    }
}

namespace real_optimizer::system
{
    MemoryInfo SystemInfo::GetMemoryInfo() noexcept
    {
        MEMORYSTATUSEX status{};
        status.dwLength = sizeof(status);

        if (!GlobalMemoryStatusEx(&status))
        {
            return {};
        }

        MemoryInfo info{};

        info.totalBytes =
            static_cast<std::uint64_t>(
                status.ullTotalPhys);

        info.availableBytes =
            static_cast<std::uint64_t>(
                status.ullAvailPhys);

        info.usagePercent =
            static_cast<unsigned int>(
                status.dwMemoryLoad);

        return info;
    }

    DiskInfo SystemInfo::GetSystemDiskInfo() noexcept
    {
        wchar_t windowsDirectory[MAX_PATH]{};

        const UINT length =
            GetWindowsDirectoryW(
                windowsDirectory,
                MAX_PATH);

        if (
            length == 0 ||
            length >= MAX_PATH)
        {
            return {};
        }

        wchar_t rootPath[] = L"C:\\";

        if (
            windowsDirectory[0] != L'\0' &&
            windowsDirectory[1] == L':')
        {
            rootPath[0] =
                windowsDirectory[0];
        }

        ULARGE_INTEGER freeBytesAvailable{};
        ULARGE_INTEGER totalBytes{};
        ULARGE_INTEGER totalFreeBytes{};

        if (!GetDiskFreeSpaceExW(
                rootPath,
                &freeBytesAvailable,
                &totalBytes,
                &totalFreeBytes))
        {
            return {};
        }

        DiskInfo info{};

        info.totalBytes =
            static_cast<std::uint64_t>(
                totalBytes.QuadPart);

        info.freeBytes =
            static_cast<std::uint64_t>(
                totalFreeBytes.QuadPart);

        return info;
    }

    CpuInfo SystemInfo::GetCpuInfo() noexcept
    {
        static std::uint64_t previousIdle = 0;
        static std::uint64_t previousKernel = 0;
        static std::uint64_t previousUser = 0;
        static bool hasPreviousSample = false;

        FILETIME idleTime{};
        FILETIME kernelTime{};
        FILETIME userTime{};

        if (!GetSystemTimes(
                &idleTime,
                &kernelTime,
                &userTime))
        {
            return {};
        }

        const std::uint64_t idle =
            FileTimeToUint64(idleTime);

        const std::uint64_t kernel =
            FileTimeToUint64(kernelTime);

        const std::uint64_t user =
            FileTimeToUint64(userTime);

        if (!hasPreviousSample)
        {
            previousIdle = idle;
            previousKernel = kernel;
            previousUser = user;
            hasPreviousSample = true;

            return {};
        }

        const std::uint64_t idleDelta =
            idle - previousIdle;

        const std::uint64_t kernelDelta =
            kernel - previousKernel;

        const std::uint64_t userDelta =
            user - previousUser;

        previousIdle = idle;
        previousKernel = kernel;
        previousUser = user;

        const std::uint64_t totalDelta =
            kernelDelta + userDelta;

        if (totalDelta == 0)
        {
            return {};
        }

        const std::uint64_t busyDelta =
            totalDelta > idleDelta
                ? totalDelta - idleDelta
                : 0;

        unsigned int usage =
            static_cast<unsigned int>(
                (busyDelta * 100ULL) /
                totalDelta);

        if (usage > 100)
        {
            usage = 100;
        }

        CpuInfo info{};
        info.usagePercent = usage;
        info.valid = true;

        return info;
    }

    SystemDetails SystemInfo::GetSystemDetails()
    {
        SystemDetails details{};

        //
        // MEMORY
        //

        const MemoryInfo memory =
            GetMemoryInfo();

        details.totalMemoryBytes =
            memory.totalBytes;

        //
        // COMPUTER NAME
        //

        wchar_t computerNameBuffer[MAX_COMPUTERNAME_LENGTH + 1]{};

        DWORD computerNameSize =
            MAX_COMPUTERNAME_LENGTH + 1;

        if (GetComputerNameW(
                computerNameBuffer,
                &computerNameSize))
        {
            details.computerName =
                computerNameBuffer;
        }
        else
        {
            details.computerName =
                L"Unknown";
        }

        //
        // ARCHITECTURE
        //

        SYSTEM_INFO systemInfo{};

        GetNativeSystemInfo(
            &systemInfo);

        switch (
            systemInfo.wProcessorArchitecture)
        {
        case PROCESSOR_ARCHITECTURE_AMD64:
            details.architecture =
                L"64-bit (x64)";
            break;

        case PROCESSOR_ARCHITECTURE_ARM64:
            details.architecture =
                L"64-bit (ARM64)";
            break;

        case PROCESSOR_ARCHITECTURE_INTEL:
            details.architecture =
                L"32-bit (x86)";
            break;

        default:
            details.architecture =
                L"Unknown";
            break;
        }

        //
        // WINDOWS VERSION
        //
        // Para o Beta vamos usar a família do
        // sistema operacional em vez de depender
        // de APIs antigas de versionamento.
        //

        if (
            systemInfo.wProcessorArchitecture ==
                PROCESSOR_ARCHITECTURE_AMD64 ||
            systemInfo.wProcessorArchitecture ==
                PROCESSOR_ARCHITECTURE_ARM64)
        {
            details.operatingSystem =
                L"Windows 64-bit";
        }
        else
        {
            details.operatingSystem =
                L"Windows";
        }

        //
        // PROCESSOR NAME
        //
        // Lemos o nome informado pelo próprio Windows.
        //

        HKEY processorKey = nullptr;

        constexpr const wchar_t *ProcessorPath =
            L"HARDWARE\\DESCRIPTION\\System\\"
            L"CentralProcessor\\0";

        if (
            RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                ProcessorPath,
                0,
                KEY_READ,
                &processorKey) == ERROR_SUCCESS)
        {
            wchar_t processorBuffer[256]{};

            DWORD processorBufferSize =
                sizeof(processorBuffer);

            DWORD valueType = 0;

            const LONG result =
                RegQueryValueExW(
                    processorKey,
                    L"ProcessorNameString",
                    nullptr,
                    &valueType,
                    reinterpret_cast<LPBYTE>(
                        processorBuffer),
                    &processorBufferSize);

            if (
                result == ERROR_SUCCESS &&
                valueType == REG_SZ)
            {
                details.processorName =
                    processorBuffer;
            }

            RegCloseKey(
                processorKey);
        }

        if (details.processorName.empty())
        {
            details.processorName =
                L"Unknown processor";
        }

        return details;
    }

}
