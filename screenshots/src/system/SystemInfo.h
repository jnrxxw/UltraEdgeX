#pragma once

#include <cstdint>
#include <string>
#include <vector>

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

    enum class DriveKind
    {
        Unknown,
        Ssd,
        Hdd
    };

    // Um disco fixo (HD ou SSD), identificado pela letra.
    struct DriveInfo
    {
        std::wstring root;  // "C:\\"
        std::wstring label; // nome do volume, pode ser vazio

        DriveKind kind = DriveKind::Unknown;

        std::uint64_t totalBytes = 0;
        std::uint64_t freeBytes = 0;

        // true = o disco onde o Windows está instalado.
        bool isSystem = false;
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

        // Sistema: "Windows 11" / "Windows 10", edição ("Pro", "Home")
        // e versão ("24H2"). Vazio quando não foi possível ler.
        std::wstring windowsName;
        std::wstring windowsEdition;
        std::wstring windowsVersion;

        // Placa de vídeo principal (a com mais memória dedicada).
        // gpuMemoryBytes = 0 quando a placa usa a memória do sistema.
        std::wstring gpuName;
        std::uint64_t gpuMemoryBytes = 0;

        // Placa-mãe e BIOS, como o fabricante informa ao Windows.
        std::wstring motherboard;
        std::wstring biosVersion;
        std::wstring biosDate;

        // Dados que mudam com o tempo (lidos a cada chamada).
        std::uint64_t uptimeSeconds = 0;
        std::wstring powerPlan;

        // Monitor principal. refreshHz = 0 quando desconhecido.
        unsigned int displayWidth = 0;
        unsigned int displayHeight = 0;
        unsigned int displayRefreshHz = 0;
    };

    class SystemInfo final
    {
    public:
        SystemInfo() = delete;

        [[nodiscard]]
        static MemoryInfo GetMemoryInfo() noexcept;

        [[nodiscard]]
        static DiskInfo GetSystemDiskInfo() noexcept;

        // Espaço de um disco específico (root = "D:\\"). Vazio/zerado
        // quando o disco não pode ser lido.
        [[nodiscard]]
        static DiskInfo GetDiskInfo(
            const std::wstring& root) noexcept;

        // Discos fixos (HDs e SSDs, internos ou externos que o Windows
        // trata como fixos). Não inclui pendrives, cartões, CD/DVD nem
        // unidades de rede. Pode levar alguns milissegundos: use sob
        // demanda, não a cada atualização de tela.
        [[nodiscard]]
        static std::vector<DriveInfo> GetFixedDrives();

        [[nodiscard]]
        static CpuInfo GetCpuInfo() noexcept;

        [[nodiscard]]
        static SystemDetails GetSystemDetails();
    };
}