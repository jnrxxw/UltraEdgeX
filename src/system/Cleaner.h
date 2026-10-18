#pragma once

#include "system/SafetyLevel.h"

#include <cstdint>
#include <string>
#include <vector>

namespace real_optimizer::system
{
    // Um local a ser limpo. "path" aceita variáveis de ambiente
    // (ex.: %TEMP%). Se "pattern" estiver vazio, vale tudo dentro
    // da pasta; senão só arquivos com esse nome (aceita * e ?).
    struct CleanTarget
    {
        std::wstring path;
        std::wstring pattern;
        bool recursive = true;
    };

    struct CleanCategory
    {
        std::wstring id;
        std::wstring name;
        std::wstring description;

        SafetyLevel safety = SafetyLevel::Safe;
        bool requiresAdmin = false;
        bool selectedByDefault = false;

        std::vector<CleanTarget> targets;
    };

    struct ScanResult
    {
        std::uint64_t bytes = 0;
        std::uint64_t files = 0;

        // Pelo menos uma das pastas existe.
        bool available = false;

        // Alguma pasta não pôde ser lida (falta de permissão),
        // então o tamanho pode estar abaixo do real.
        bool partial = false;
    };

    struct CleanResult
    {
        std::uint64_t removedBytes = 0;
        std::uint64_t removedFiles = 0;
        std::uint64_t failedBytes = 0;
        std::uint64_t failedFiles = 0;
    };

    class Cleaner final
    {
    public:
        Cleaner() = delete;

        // Lista fixa de categorias. O app nunca remove nada
        // fora dessa lista.
        [[nodiscard]] static const std::vector<CleanCategory>&
        Categories();

        [[nodiscard]] static const CleanCategory*
        FindCategory(const std::wstring& id);

        // Apenas mede. Não apaga nada.
        [[nodiscard]] static ScanResult Scan(
            const CleanCategory& category);

        // Apaga somente arquivos. Ignora o que estiver em uso,
        // nunca segue atalhos de pasta (junctions/symlinks) e
        // nunca remove a pasta raiz.
        [[nodiscard]] static CleanResult Clean(
            const CleanCategory& category);

        [[nodiscard]] static bool IsElevated() noexcept;
    };
}
