#include "system/SystemInfo.h"

#include <mutex>

#include <windows.h>
#include <winioctl.h>

#include <dxgi.h>
#include <powrprof.h>

#include <cwctype>
#include <string>
#include <vector>

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

    std::wstring Trim(std::wstring text)
    {
        const auto isSpace = [](wchar_t c)
        { return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n' || c == L'\0'; };

        while (!text.empty() && isSpace(text.back()))
        {
            text.pop_back();
        }

        std::size_t start = 0;

        while (start < text.size() && isSpace(text[start]))
        {
            ++start;
        }

        return text.substr(start);
    }

    std::wstring Lower(std::wstring text)
    {
        for (wchar_t &c : text)
        {
            c = static_cast<wchar_t>(std::towlower(c));
        }

        return text;
    }

    bool Contains(const std::wstring &text, const wchar_t *part)
    {
        return Lower(text).find(part) != std::wstring::npos;
    }

    std::wstring ReadRegistryString(
        HKEY root,
        const wchar_t *path,
        const wchar_t *name)
    {
        HKEY key = nullptr;

        if (RegOpenKeyExW(root, path, 0, KEY_READ, &key) != ERROR_SUCCESS)
        {
            return {};
        }

        wchar_t buffer[512]{};
        DWORD size = sizeof(buffer) - sizeof(wchar_t);
        DWORD type = 0;

        const LONG result = RegQueryValueExW(
            key,
            name,
            nullptr,
            &type,
            reinterpret_cast<LPBYTE>(buffer),
            &size);

        RegCloseKey(key);

        if (result != ERROR_SUCCESS ||
            (type != REG_SZ && type != REG_EXPAND_SZ))
        {
            return {};
        }

        return Trim(buffer);
    }

    // Textos que os fabricantes deixam sem preencher.
    bool IsPlaceholder(const std::wstring &text)
    {
        if (text.empty())
        {
            return true;
        }

        static const wchar_t *const Placeholders[] = {
            L"to be filled",
            L"default string",
            L"system product name",
            L"system manufacturer",
            L"system version",
            L"not applicable",
            L"not specified",
            L"o.e.m",
            L"unknown"};

        for (const wchar_t *placeholder : Placeholders)
        {
            if (Contains(text, placeholder))
            {
                return true;
            }
        }

        return false;
    }

    void RemoveText(std::wstring &text, const std::wstring &part)
    {
        const std::wstring lowerText = Lower(text);
        const std::wstring lowerPart = Lower(part);

        std::size_t position = 0;

        while ((position = lowerText.find(lowerPart)) != std::wstring::npos)
        {
            text.erase(position, part.size());
            return;
        }
    }

    // "ASUSTeK COMPUTER INC." -> "ASUS", "Micro-Star ..." -> "MSI".
    std::wstring CleanVendor(std::wstring vendor)
    {
        vendor = Trim(vendor);
        const std::wstring lower = Lower(vendor);

        if (lower.find(L"asus") != std::wstring::npos)
        {
            return L"ASUS";
        }

        if (lower.find(L"micro-star") != std::wstring::npos ||
            lower == L"msi")
        {
            return L"MSI";
        }

        if (lower.find(L"gigabyte") != std::wstring::npos)
        {
            return L"Gigabyte";
        }

        if (lower.find(L"asrock") != std::wstring::npos)
        {
            return L"ASRock";
        }

        if (lower.find(L"dell") != std::wstring::npos)
        {
            return L"Dell";
        }

        if (lower.find(L"lenovo") != std::wstring::npos)
        {
            return L"Lenovo";
        }

        if (lower.find(L"hewlett") != std::wstring::npos ||
            lower == L"hp")
        {
            return L"HP";
        }

        for (const wchar_t *suffix : {L" Co., Ltd.", L" Co.,Ltd.", L", Inc.",
                                      L" Inc.", L" Corporation", L" Corp.",
                                      L" Technology", L" COMPUTER"})
        {
            RemoveText(vendor, suffix);
        }

        return Trim(vendor);
    }

    // Placa de vídeo principal: ignora o adaptador de software do Windows
    // e, havendo mais de uma placa, escolhe a com mais memória dedicada.
    void ReadGpu(std::wstring &name, std::uint64_t &memoryBytes)
    {
        IDXGIFactory1 *factory = nullptr;

        if (FAILED(CreateDXGIFactory1(
                __uuidof(IDXGIFactory1),
                reinterpret_cast<void **>(&factory))) ||
            factory == nullptr)
        {
            return;
        }

        for (UINT index = 0;; ++index)
        {
            IDXGIAdapter1 *adapter = nullptr;

            if (factory->EnumAdapters1(index, &adapter) ==
                DXGI_ERROR_NOT_FOUND)
            {
                break;
            }

            DXGI_ADAPTER_DESC1 desc{};

            if (SUCCEEDED(adapter->GetDesc1(&desc)))
            {
                const bool software =
                    (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0 ||
                    (desc.VendorId == 0x1414 && desc.DeviceId == 0x8C);

                const auto dedicated =
                    static_cast<std::uint64_t>(desc.DedicatedVideoMemory);

                if (!software &&
                    (name.empty() || dedicated > memoryBytes))
                {
                    name = desc.Description;
                    memoryBytes = dedicated;
                }
            }

            adapter->Release();
        }

        factory->Release();
    }

    struct StaticDetails
    {
        std::wstring windowsName;
        std::wstring windowsEdition;
        std::wstring windowsVersion;
        std::wstring gpuName;
        std::uint64_t gpuMemoryBytes = 0;
        std::wstring motherboard;
        std::wstring biosVersion;
        std::wstring biosDate;
    };

    // O que não muda com o PC ligado: lido uma vez só.
    StaticDetails ReadStaticDetails()
    {
        StaticDetails data;

        // Windows
        constexpr const wchar_t *VersionPath =
            L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

        const std::wstring productName =
            ReadRegistryString(HKEY_LOCAL_MACHINE, VersionPath, L"ProductName");

        std::wstring displayVersion =
            ReadRegistryString(HKEY_LOCAL_MACHINE, VersionPath, L"DisplayVersion");

        if (displayVersion.empty())
        {
            displayVersion =
                ReadRegistryString(HKEY_LOCAL_MACHINE, VersionPath, L"ReleaseId");
        }

        const int build = _wtoi(
            ReadRegistryString(
                HKEY_LOCAL_MACHINE, VersionPath, L"CurrentBuildNumber")
                .c_str());

        // O registro ainda diz "Windows 10" no Windows 11; o número da
        // versão (build 22000 ou maior) é o que diferencia.
        data.windowsName =
            build >= 22000 ? L"Windows 11"
                           : (build > 0 ? L"Windows 10" : L"Windows");

        if (Contains(productName, L"home"))
        {
            data.windowsEdition = L"Home";
        }
        else if (Contains(productName, L"enterprise"))
        {
            data.windowsEdition = L"Enterprise";
        }
        else if (Contains(productName, L"education"))
        {
            data.windowsEdition = L"Education";
        }
        else if (Contains(productName, L"pro"))
        {
            data.windowsEdition = L"Pro";
        }

        data.windowsVersion = displayVersion;

        // Placa de vídeo
        ReadGpu(data.gpuName, data.gpuMemoryBytes);

        // Placa-mãe e BIOS
        constexpr const wchar_t *BiosPath =
            L"HARDWARE\\DESCRIPTION\\System\\BIOS";

        auto biosValue = [&](const wchar_t *name)
        {
            std::wstring value =
                ReadRegistryString(HKEY_LOCAL_MACHINE, BiosPath, name);

            return IsPlaceholder(value) ? std::wstring() : value;
        };

        std::wstring vendor = CleanVendor(biosValue(L"BaseBoardManufacturer"));
        std::wstring product = biosValue(L"BaseBoardProduct");

        if (product.empty())
        {
            // PCs de marca: o modelo do computador diz mais que a placa.
            vendor = CleanVendor(biosValue(L"SystemManufacturer"));
            product = biosValue(L"SystemProductName");
        }

        data.motherboard = Trim(vendor + (vendor.empty() ? L"" : L" ") + product);

        data.biosVersion = biosValue(L"BIOSVersion");

        // O Windows guarda a data no formato mm/dd/aaaa.
        const std::wstring date = biosValue(L"BIOSReleaseDate");

        if (date.size() == 10 && date[2] == L'/' && date[5] == L'/')
        {
            data.biosDate =
                date.substr(3, 2) + L"/" + date.substr(0, 2) + L"/" + date.substr(6);
        }
        else
        {
            data.biosDate = date;
        }

        return data;
    }

    // SSD ou HD, pelo próprio driver do disco: HD tem "penalidade de
    // busca" (cabeça mecânica). Não precisa de administrador.
    real_optimizer::system::DriveKind DetectDriveKind(wchar_t letter)
    {
        using real_optimizer::system::DriveKind;

        const wchar_t path[] = {L'\\', L'\\', L'.', L'\\', letter, L':', L'\0'};

        const HANDLE handle = CreateFileW(
            path,
            0,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr);

        if (handle == INVALID_HANDLE_VALUE)
        {
            return DriveKind::Unknown;
        }

        STORAGE_PROPERTY_QUERY query{};
        query.PropertyId = StorageDeviceSeekPenaltyProperty;
        query.QueryType = PropertyStandardQuery;

        DEVICE_SEEK_PENALTY_DESCRIPTOR descriptor{};
        DWORD returned = 0;

        const BOOL ok = DeviceIoControl(
            handle,
            IOCTL_STORAGE_QUERY_PROPERTY,
            &query,
            sizeof(query),
            &descriptor,
            sizeof(descriptor),
            &returned,
            nullptr);

        CloseHandle(handle);

        if (!ok || returned < sizeof(descriptor))
        {
            return DriveKind::Unknown;
        }

        return descriptor.IncursSeekPenalty != FALSE ? DriveKind::Hdd
                                                     : DriveKind::Ssd;
    }

    std::wstring ReadPowerPlan()
    {
        GUID *scheme = nullptr;
        std::wstring plan;

        if (PowerGetActiveScheme(nullptr, &scheme) == ERROR_SUCCESS &&
            scheme != nullptr)
        {
            wchar_t buffer[256]{};
            DWORD size = sizeof(buffer) - sizeof(wchar_t);

            if (PowerReadFriendlyName(
                    nullptr,
                    scheme,
                    nullptr,
                    nullptr,
                    reinterpret_cast<PUCHAR>(buffer),
                    &size) == ERROR_SUCCESS)
            {
                plan = Trim(buffer);
            }
        }

        if (scheme != nullptr)
        {
            LocalFree(scheme);
        }

        return plan;
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

    DiskInfo SystemInfo::GetDiskInfo(
        const std::wstring &root) noexcept
    {
        if (root.empty())
        {
            return {};
        }

        ULARGE_INTEGER freeBytesAvailable{};
        ULARGE_INTEGER totalBytes{};
        ULARGE_INTEGER totalFreeBytes{};

        if (!GetDiskFreeSpaceExW(
                root.c_str(),
                &freeBytesAvailable,
                &totalBytes,
                &totalFreeBytes))
        {
            return {};
        }

        DiskInfo info{};

        info.totalBytes =
            static_cast<std::uint64_t>(totalBytes.QuadPart);

        info.freeBytes =
            static_cast<std::uint64_t>(totalFreeBytes.QuadPart);

        return info;
    }

    std::vector<DriveInfo> SystemInfo::GetFixedDrives()
    {
        std::vector<DriveInfo> drives;

        wchar_t windowsDirectory[MAX_PATH]{};

        const UINT windowsLength =
            GetWindowsDirectoryW(windowsDirectory, MAX_PATH);

        wchar_t systemLetter = L'C';

        if (windowsLength > 2 && windowsDirectory[1] == L':')
        {
            systemLetter = static_cast<wchar_t>(
                std::towupper(windowsDirectory[0]));
        }

        const DWORD mask = GetLogicalDrives();

        for (unsigned int index = 0; index < 26; ++index)
        {
            if ((mask & (1U << index)) == 0)
            {
                continue;
            }

            const wchar_t letter =
                static_cast<wchar_t>(L'A' + index);

            const wchar_t root[4] = {letter, L':', L'\\', L'\0'};

            // Só discos fixos: ignora pendrive, cartão, CD/DVD e rede.
            if (GetDriveTypeW(root) != DRIVE_FIXED)
            {
                continue;
            }

            const DiskInfo space = GetDiskInfo(root);

            // Sem leitura (ex.: volume bloqueado) ou sem tamanho.
            if (space.totalBytes == 0)
            {
                continue;
            }

            DriveInfo drive{};

            drive.root = root;
            drive.totalBytes = space.totalBytes;
            drive.freeBytes = space.freeBytes;
            drive.isSystem = letter == systemLetter;
            drive.kind = DetectDriveKind(letter);

            wchar_t label[MAX_PATH + 1]{};

            if (GetVolumeInformationW(
                    root,
                    label,
                    MAX_PATH + 1,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    0))
            {
                drive.label = Trim(label);
            }

            drives.push_back(std::move(drive));
        }

        return drives;
    }

    CpuInfo SystemInfo::GetCpuInfo() noexcept
    {
        // A UI (timer) e a análise (thread de trabalho) podem
        // chamar ao mesmo tempo; o estado da amostra é protegido.
        static std::mutex sampleMutex;
        const std::lock_guard<std::mutex> lock(sampleMutex);

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

        //
        // HARDWARE E SISTEMA (leitura única)
        //

        static const StaticDetails staticDetails = ReadStaticDetails();

        details.windowsName = staticDetails.windowsName;
        details.windowsEdition = staticDetails.windowsEdition;
        details.windowsVersion = staticDetails.windowsVersion;
        details.gpuName = staticDetails.gpuName;
        details.gpuMemoryBytes = staticDetails.gpuMemoryBytes;
        details.motherboard = staticDetails.motherboard;
        details.biosVersion = staticDetails.biosVersion;
        details.biosDate = staticDetails.biosDate;

        if (!details.windowsName.empty())
        {
            details.operatingSystem =
                details.windowsName +
                (details.windowsEdition.empty() ? L"" : L" ") +
                details.windowsEdition;
        }

        //
        // DADOS QUE MUDAM
        //

        details.uptimeSeconds = GetTickCount64() / 1000ULL;
        details.powerPlan = ReadPowerPlan();

        DEVMODEW mode{};
        mode.dmSize = sizeof(mode);

        if (EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode))
        {
            details.displayWidth = mode.dmPelsWidth;
            details.displayHeight = mode.dmPelsHeight;

            // 0 e 1 significam "taxa padrão do hardware".
            details.displayRefreshHz =
                mode.dmDisplayFrequency > 1 ? mode.dmDisplayFrequency : 0;
        }

        return details;
    }

}
