#include "app/Application.h"

#include "system/ElevatedCleaner.h"
#include "system/ElevatedServices.h"
#include "system/ElevatedStartup.h"

#include <windows.h>

#include <string>

int WINAPI wWinMain(
    HINSTANCE instance,
    HINSTANCE previousInstance,
    PWSTR commandLine,
    int showCommand
)
{
    static_cast<void>(previousInstance);

    const std::wstring arguments =
        commandLine != nullptr ? commandLine : L"";

    // Iniciado com permissão de administrador só para limpar
    // (sem abrir janela). Veja system/ElevatedCleaner.
    if (real_optimizer::system::IsElevatedHelperCommand(arguments))
    {
        return real_optimizer::system::RunElevatedHelper(arguments);
    }

    // Idem, para alterar serviços do Windows.
    if (real_optimizer::system::IsServicesHelperCommand(arguments))
    {
        return real_optimizer::system::RunServicesHelper(arguments);
    }

    // Idem, para alterar a inicialização de todos os usuários.
    if (real_optimizer::system::IsStartupHelperCommand(arguments))
    {
        return real_optimizer::system::RunStartupHelper(arguments);
    }

    real_optimizer::app::Application application;

    return application.Run(
        instance,
        showCommand
    );
}
