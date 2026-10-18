#include "system/Cleaner.h"

#include <windows.h>

#include <cwctype>

namespace real_optimizer::system
{
    namespace
    {
        constexpr int MaxDepth = 48;

        std::wstring ExpandVariables(const std::wstring& text)
        {
            const DWORD needed =
                ExpandEnvironmentStringsW(text.c_str(), nullptr, 0);

            if (needed == 0)
            {
                return {};
            }

            std::wstring result(needed, L'\0');

            const DWORD written = ExpandEnvironmentStringsW(
                text.c_str(),
                result.data(),
                needed);

            if (written == 0 || written > needed)
            {
                return {};
            }

            result.resize(written - 1);

            // Variável não resolvida: não arrisca.
            if (result.find(L'%') != std::wstring::npos)
            {
                return {};
            }

            while (!result.empty() && result.back() == L'\\')
            {
                result.pop_back();
            }

            return result;
        }

        bool WildcardMatch(
            const std::wstring& pattern,
            const std::wstring& text)
        {
            std::size_t p = 0;
            std::size_t t = 0;
            std::size_t star = std::wstring::npos;
            std::size_t mark = 0;

            while (t < text.size())
            {
                if (
                    p < pattern.size() &&
                    (pattern[p] == L'?' ||
                     std::towlower(pattern[p]) ==
                         std::towlower(text[t])))
                {
                    ++p;
                    ++t;
                }
                else if (
                    p < pattern.size() &&
                    pattern[p] == L'*')
                {
                    star = p++;
                    mark = t;
                }
                else if (star != std::wstring::npos)
                {
                    p = star + 1;
                    t = ++mark;
                }
                else
                {
                    return false;
                }
            }

            while (p < pattern.size() && pattern[p] == L'*')
            {
                ++p;
            }

            return p == pattern.size();
        }

        enum class PathState
        {
            Missing,   // não existe
            Directory, // existe e é uma pasta
            Denied     // existe, mas o Windows não deixa ler
        };

        PathState GetPathState(const std::wstring& path)
        {
            const DWORD attributes =
                GetFileAttributesW(path.c_str());

            if (attributes == INVALID_FILE_ATTRIBUTES)
            {
                return GetLastError() == ERROR_ACCESS_DENIED
                           ? PathState::Denied
                           : PathState::Missing;
            }

            return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0
                       ? PathState::Directory
                       : PathState::Missing;
        }

        std::size_t PathDepth(const std::wstring& path)
        {
            std::size_t depth = 0;
            bool inPart = false;

            for (const wchar_t c : path)
            {
                if (c == L'\\' || c == L'/')
                {
                    inPart = false;
                }
                else if (!inPart)
                {
                    inPart = true;
                    ++depth;
                }
            }

            return depth;
        }

        // Trava de segurança: nada de limpar raízes como C:\ ou
        // C:\Windows por inteiro.
        bool IsSafeTarget(const CleanTarget& target, const std::wstring& expanded)
        {
            if (expanded.empty())
            {
                return false;
            }

            if (target.pattern.empty() || target.recursive)
            {
                return PathDepth(expanded) >= 3;
            }

            return true;
        }

        std::uint64_t FileSize(const WIN32_FIND_DATAW& data)
        {
            return (static_cast<std::uint64_t>(data.nFileSizeHigh)
                    << 32) |
                   data.nFileSizeLow;
        }

        bool IsDotEntry(const wchar_t* name)
        {
            return name[0] == L'.' &&
                   (name[1] == L'\0' ||
                    (name[1] == L'.' && name[2] == L'\0'));
        }

        void ScanDirectory(
            const std::wstring& directory,
            const std::wstring& pattern,
            bool recursive,
            int depth,
            ScanResult& result)
        {
            if (depth > MaxDepth)
            {
                result.partial = true;
                return;
            }

            WIN32_FIND_DATAW data{};

            const HANDLE find = FindFirstFileExW(
                (directory + L"\\*").c_str(),
                FindExInfoBasic,
                &data,
                FindExSearchNameMatch,
                nullptr,
                FIND_FIRST_EX_LARGE_FETCH);

            if (find == INVALID_HANDLE_VALUE)
            {
                if (GetLastError() == ERROR_ACCESS_DENIED)
                {
                    result.partial = true;
                }

                return;
            }

            do
            {
                if (IsDotEntry(data.cFileName))
                {
                    continue;
                }

                // Nunca segue atalhos de pasta nem links.
                if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
                {
                    continue;
                }

                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (recursive)
                    {
                        ScanDirectory(
                            directory + L"\\" + data.cFileName,
                            pattern,
                            recursive,
                            depth + 1,
                            result);
                    }
                }
                else if (
                    pattern.empty() ||
                    WildcardMatch(pattern, data.cFileName))
                {
                    result.bytes += FileSize(data);
                    ++result.files;
                }
            } while (FindNextFileW(find, &data));

            FindClose(find);
        }

        // Retorna true se a pasta ficou vazia (para poder removê-la).
        bool CleanDirectory(
            const std::wstring& directory,
            const std::wstring& pattern,
            bool recursive,
            int depth,
            CleanResult& result)
        {
            if (depth > MaxDepth)
            {
                return false;
            }

            WIN32_FIND_DATAW data{};

            const HANDLE find = FindFirstFileExW(
                (directory + L"\\*").c_str(),
                FindExInfoBasic,
                &data,
                FindExSearchNameMatch,
                nullptr,
                FIND_FIRST_EX_LARGE_FETCH);

            if (find == INVALID_HANDLE_VALUE)
            {
                return false;
            }

            bool empty = true;

            do
            {
                if (IsDotEntry(data.cFileName))
                {
                    continue;
                }

                const std::wstring full =
                    directory + L"\\" + data.cFileName;

                // Atalhos de pasta e links ficam intocados.
                if (data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
                {
                    empty = false;
                    continue;
                }

                if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (!recursive)
                    {
                        empty = false;
                        continue;
                    }

                    const bool childEmpty = CleanDirectory(
                        full,
                        pattern,
                        recursive,
                        depth + 1,
                        result);

                    if (!(childEmpty && RemoveDirectoryW(full.c_str())))
                    {
                        empty = false;
                    }

                    continue;
                }

                if (
                    !pattern.empty() &&
                    !WildcardMatch(pattern, data.cFileName))
                {
                    empty = false;
                    continue;
                }

                const std::uint64_t size = FileSize(data);

                if (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                {
                    SetFileAttributesW(
                        full.c_str(),
                        data.dwFileAttributes &
                            ~static_cast<DWORD>(FILE_ATTRIBUTE_READONLY));
                }

                if (DeleteFileW(full.c_str()))
                {
                    result.removedBytes += size;
                    ++result.removedFiles;
                }
                else
                {
                    // Em uso ou sem permissão: ignora e segue.
                    result.failedBytes += size;
                    ++result.failedFiles;
                    empty = false;
                }
            } while (FindNextFileW(find, &data));

            FindClose(find);

            return empty;
        }

        std::vector<CleanCategory> BuildCategories()
        {
            std::vector<CleanCategory> categories;

            categories.push_back(CleanCategory{
                L"user-temp",
                L"Temporários do usuário",
                L"Arquivos que os programas deixam na pasta Temp enquanto trabalham.",
                SafetyLevel::Safe,
                false,
                true,
                {{L"%TEMP%", L"", true}}});

            categories.push_back(CleanCategory{
                L"windows-temp",
                L"Temporários do Windows",
                L"Arquivos temporários do sistema em C:\\Windows\\Temp.",
                SafetyLevel::Safe,
                true,
                true,
                {{L"%SystemRoot%\\Temp", L"", true}}});

            categories.push_back(CleanCategory{
                L"windows-update",
                L"Cache do Windows Update",
                L"Downloads de atualizações. O Windows baixa de novo se precisar.",
                SafetyLevel::Attention,
                true,
                false,
                {{L"%SystemRoot%\\SoftwareDistribution\\Download", L"", true}}});

            categories.push_back(CleanCategory{
                L"delivery-optimization",
                L"Cache de entrega de atualizações",
                L"Cópias de atualizações guardadas para compartilhar. O Windows recria.",
                SafetyLevel::Safe,
                true,
                true,
                {{L"%SystemRoot%\\ServiceProfiles\\NetworkService\\AppData\\Local\\Microsoft\\Windows\\DeliveryOptimization\\Cache", L"", true},
                 {L"%SystemRoot%\\SoftwareDistribution\\DeliveryOptimization", L"", true}}});

            categories.push_back(CleanCategory{
                L"error-reports",
                L"Relatórios de erro do Windows",
                L"Relatórios antigos de travamentos de programas. Servem só para diagnóstico.",
                SafetyLevel::Safe,
                true,
                true,
                {{L"%ProgramData%\\Microsoft\\Windows\\WER\\ReportArchive", L"", true},
                 {L"%ProgramData%\\Microsoft\\Windows\\WER\\ReportQueue", L"", true},
                 {L"%ProgramData%\\Microsoft\\Windows\\WER\\Temp", L"", true},
                 {L"%LOCALAPPDATA%\\Microsoft\\Windows\\WER", L"", true}}});

            categories.push_back(CleanCategory{
                L"crash-dumps",
                L"Dumps de memória e travamentos",
                L"Arquivos de depuração gerados quando um programa ou o Windows trava.",
                SafetyLevel::Safe,
                true,
                true,
                {{L"%SystemRoot%\\Minidump", L"", true},
                 {L"%SystemRoot%", L"MEMORY.DMP", false},
                 {L"%LOCALAPPDATA%\\CrashDumps", L"", true}}});

            categories.push_back(CleanCategory{
                L"thumbnails",
                L"Cache de miniaturas",
                L"Miniaturas e ícones do Explorer. O Windows recria ao abrir as pastas.",
                SafetyLevel::Safe,
                false,
                true,
                {{L"%LOCALAPPDATA%\\Microsoft\\Windows\\Explorer", L"thumbcache_*.db", false},
                 {L"%LOCALAPPDATA%\\Microsoft\\Windows\\Explorer", L"iconcache_*.db", false}}});

            categories.push_back(CleanCategory{
                L"shader-cache",
                L"Cache de shaders",
                L"Cache gráfico de jogos. Podem engasgar um pouco na primeira abertura depois.",
                SafetyLevel::Attention,
                false,
                false,
                {{L"%LOCALAPPDATA%\\D3DSCache", L"", true},
                 {L"%LOCALAPPDATA%\\NVIDIA\\DXCache", L"", true},
                 {L"%LOCALAPPDATA%\\NVIDIA\\GLCache", L"", true},
                 {L"%LOCALAPPDATA%\\AMD\\DxCache", L"", true},
                 {L"%LOCALAPPDATA%\\AMD\\GLCache", L"", true}}});

            categories.push_back(CleanCategory{
                L"prefetch",
                L"Prefetch",
                L"O Windows recria sozinho. Limpar não acelera o PC e pode atrasar os apps.",
                SafetyLevel::Attention,
                true,
                false,
                {{L"%SystemRoot%\\Prefetch", L"*.pf", false}}});

            return categories;
        }
    }

    const std::vector<CleanCategory>& Cleaner::Categories()
    {
        static const std::vector<CleanCategory> categories =
            BuildCategories();

        return categories;
    }

    const CleanCategory* Cleaner::FindCategory(
        const std::wstring& id)
    {
        for (const auto& category : Categories())
        {
            if (category.id == id)
            {
                return &category;
            }
        }

        return nullptr;
    }

    ScanResult Cleaner::Scan(const CleanCategory& category)
    {
        ScanResult result;

        for (const auto& target : category.targets)
        {
            const std::wstring path = ExpandVariables(target.path);

            if (!IsSafeTarget(target, path))
            {
                continue;
            }

            const PathState state = GetPathState(path);

            // Pasta que existe mas o Windows não deixa ler (precisa de
            // administrador): o tamanho é desconhecido, não "vazio".
            if (state == PathState::Denied)
            {
                result.available = true;
                result.partial = true;
                continue;
            }

            if (state != PathState::Directory)
            {
                continue;
            }

            result.available = true;

            ScanDirectory(
                path,
                target.pattern,
                target.recursive,
                0,
                result);
        }

        return result;
    }

    CleanResult Cleaner::Clean(const CleanCategory& category)
    {
        CleanResult result;

        for (const auto& target : category.targets)
        {
            const std::wstring path = ExpandVariables(target.path);

            if (
                !IsSafeTarget(target, path) ||
                GetPathState(path) != PathState::Directory)
            {
                continue;
            }

            // O retorno indica se a pasta raiz ficou vazia,
            // mas a raiz nunca é removida.
            static_cast<void>(CleanDirectory(
                path,
                target.pattern,
                target.recursive,
                0,
                result));
        }

        return result;
    }

    bool Cleaner::IsElevated() noexcept
    {
        HANDLE token = nullptr;

        if (!OpenProcessToken(
                GetCurrentProcess(),
                TOKEN_QUERY,
                &token))
        {
            return false;
        }

        TOKEN_ELEVATION elevation{};
        DWORD size = sizeof(elevation);

        const BOOL ok = GetTokenInformation(
            token,
            TokenElevation,
            &elevation,
            sizeof(elevation),
            &size);

        CloseHandle(token);

        return ok && elevation.TokenIsElevated != 0;
    }
}
