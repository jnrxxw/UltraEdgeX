#pragma once

#include "system/ElevatedProcess.h"
#include "system/StartupManager.h"

#include <string>
#include <vector>

namespace real_optimizer::system
{
    // Aplica alterações de inicialização do escopo "todos os usuários"
    // com permissão de administrador (o Windows mostra o pedido de
    // confirmação). Só aceita entradas de HKLM.
    // Bloqueia: chamar fora da thread da interface.
    [[nodiscard]] ElevationStatus RunStartupElevated(
        const StartupRequest& request,
        std::vector<StartupChange>& changes);

    // Modo "ajudante": o programa foi iniciado só para isso.
    [[nodiscard]] bool IsStartupHelperCommand(
        const std::wstring& commandLine);

    [[nodiscard]] int RunStartupHelper(
        const std::wstring& commandLine);
}
