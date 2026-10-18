#pragma once

#include "system/ElevatedProcess.h"
#include "system/Services.h"

#include <string>
#include <vector>

namespace real_optimizer::system
{
    // Aplica um pedido de alteração de serviços com permissão de
    // administrador (o Windows mostra o pedido de confirmação).
    // Bloqueia: chamar fora da thread da interface.
    [[nodiscard]] ElevationStatus RunServicesElevated(
        const ServiceRequest& request,
        std::vector<ServiceChange>& changes);

    // Modo "ajudante": o programa foi iniciado só para isso.
    [[nodiscard]] bool IsServicesHelperCommand(
        const std::wstring& commandLine);

    [[nodiscard]] int RunServicesHelper(
        const std::wstring& commandLine);
}
