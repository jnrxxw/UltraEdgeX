#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <string>

namespace real_optimizer::core
{
    // Tamanho legível em português: "4,89 GB", "37,9 MB", "524 KB".
    // Usado pelo Painel e pela Limpeza para os números sempre baterem.
    [[nodiscard]] inline std::wstring FormatBytes(std::uint64_t bytes)
    {
        wchar_t buffer[48]{};

        constexpr double Kilobyte = 1024.0;
        constexpr double Megabyte = Kilobyte * 1024.0;
        constexpr double Gigabyte = Megabyte * 1024.0;

        const double value = static_cast<double>(bytes);

        if (value >= Gigabyte)
        {
            swprintf(buffer, 48, L"%.2f GB", value / Gigabyte);
        }
        else if (value >= Megabyte)
        {
            swprintf(buffer, 48, L"%.1f MB", value / Megabyte);
        }
        else if (value >= Kilobyte)
        {
            swprintf(buffer, 48, L"%.0f KB", value / Kilobyte);
        }
        else
        {
            swprintf(buffer, 48, L"%llu B",
                     static_cast<unsigned long long>(bytes));
        }

        std::wstring text(buffer);

        // Vírgula decimal, como no Brasil.
        std::replace(text.begin(), text.end(), L'.', L',');

        return text;
    }
}
