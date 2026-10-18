#pragma once

#include <string>
#include <vector>

namespace real_optimizer::system
{
    enum class ElevationStatus
    {
        Success,
        Cancelled, // o usuário recusou o pedido de administrador
        Failed
    };

    // ---- Lado do app principal ----

    // Abre uma cópia do próprio programa com permissão de
    // administrador (o Windows mostra o pedido de confirmação):
    //   <exe> <helperSwitch> "<arquivo de resultado>" <arguments>
    // Espera terminar e devolve as linhas do arquivo de resultado.
    // Bloqueia: chamar fora da thread da interface.
    [[nodiscard]] ElevationStatus RunSelfElevated(
        const std::wstring& helperSwitch,
        const std::wstring& arguments,
        std::vector<std::string>& resultLines);

    // Abre outra cópia do programa como administrador, sem esperar.
    // Retorna false se o usuário recusou ou se não foi possível.
    [[nodiscard]] bool RelaunchElevated(const wchar_t* arguments);

    // ---- Lado do ajudante (processo elevado) ----

    struct HelperCommand
    {
        std::wstring resultPath;
        std::wstring arguments;
    };

    [[nodiscard]] bool IsHelperCommand(
        const std::wstring& commandLine,
        const std::wstring& helperSwitch);

    // Extrai e VALIDA a linha de comando. O arquivo de resultado só
    // pode ficar em %LOCALAPPDATA%\UltraEdgeX, terminar em ".result"
    // e não conter "..", para o ajudante não escrever em qualquer
    // lugar do disco com privilégios de administrador.
    [[nodiscard]] bool ParseHelperCommand(
        const std::wstring& commandLine,
        const std::wstring& helperSwitch,
        HelperCommand& command);

    // Separa "a,b,c" em partes (sem espaços).
    [[nodiscard]] std::vector<std::wstring> SplitList(
        const std::wstring& text);
}
