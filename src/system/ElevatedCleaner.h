#pragma once

#include "system/Cleaner.h"
#include "system/ElevatedProcess.h"

#include <string>
#include <vector>

namespace real_optimizer::system
{
    struct ElevatedCategoryResult
    {
        std::wstring id;
        CleanResult result;
    };

    // Limpa as categorias indicadas com permissão de administrador
    // (o Windows mostra o pedido de confirmação) e devolve os
    // resultados. Bloqueia: chamar fora da thread da interface.
    [[nodiscard]] ElevationStatus RunCleanElevated(
        const std::vector<std::wstring>& categoryIds,
        std::vector<ElevatedCategoryResult>& results);

    // Abre outra cópia do programa com permissão de administrador
    // (já na página Limpeza). Retorna false se o usuário recusou
    // o pedido do Windows ou se não foi possível abrir.
    // O chamador deve fechar a cópia atual quando retornar true.
    [[nodiscard]] bool RelaunchAsAdministrator();

    // Modo "ajudante": o programa foi iniciado só para limpar.
    [[nodiscard]] bool IsElevatedHelperCommand(
        const std::wstring& commandLine);

    [[nodiscard]] int RunElevatedHelper(
        const std::wstring& commandLine);
}
