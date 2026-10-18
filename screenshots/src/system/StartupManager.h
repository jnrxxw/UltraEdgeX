#pragma once

#include <string>
#include <vector>

namespace real_optimizer::system
{
    // De onde vem a entrada de inicialização.
    enum class StartupSource
    {
        CurrentUser,   // HKCU\...\Run (só este usuário)
        LocalMachine,  // HKLM\...\Run (todos os usuários)
        LocalMachine32, // HKLM\...\WOW6432Node\...\Run (apps de 32 bits)
        PackagedApp     // apps da Microsoft Store (tarefa de inicialização do app)
    };

    struct StartupApp
    {
        std::wstring name;
        std::wstring command;

        StartupSource source =
            StartupSource::CurrentUser;

        // Indica se o aplicativo está
        // habilitado para iniciar com o Windows.
        bool enabled = true;

        // true = o estado é controlado pelo sistema (política ou o
        // próprio app) e o usuário não pode alterar por aqui.
        bool locked = false;

        // Motivo mostrado ao usuário quando locked = true.
        std::wstring lockReason;
    };

    // Identifica uma entrada existente (origem + nome do valor).
    struct StartupTarget
    {
        StartupSource source = StartupSource::CurrentUser;
        std::wstring name;
    };

    enum class StartupOutcome
    {
        Changed,
        Unchanged, // já estava como pedido
        NotFound,  // a entrada não existe mais
        Failed
    };

    struct StartupChange
    {
        StartupTarget target;
        StartupOutcome outcome = StartupOutcome::Failed;
        unsigned long error = 0;
    };

    struct StartupRequest
    {
        std::vector<StartupTarget> disable;
        std::vector<StartupTarget> enable;

        // Reativa o que o UeX desativou, separado por escopo porque
        // o escopo "todos os usuários" exige administrador.
        bool restoreCurrentUser = false;
        bool restoreMachine = false;

        [[nodiscard]] bool IsEmpty() const noexcept
        {
            return disable.empty() &&
                   enable.empty() &&
                   !restoreCurrentUser &&
                   !restoreMachine;
        }
    };

    class StartupManager final
    {
    public:
        StartupManager() = delete;

        // Só leitura. Não precisa de administrador.
        [[nodiscard]]
        static std::vector<StartupApp>
        GetStartupApps();

        // true para as origens que gravam em HKLM.
        [[nodiscard]] static bool RequiresAdministrator(
            StartupSource source) noexcept;

        // Executa um pedido. Só mexe em entradas que existem agora
        // nas chaves Run, e só no escopo que o processo consegue
        // alterar (o resto volta como Failed).
        //
        // Nada é apagado: o app só muda a marca "habilitado" que o
        // próprio Windows usa (StartupApproved), então é reversível.
        // Antes de desativar, anota no backup; sem backup, não altera.
        [[nodiscard]] static std::vector<StartupChange> Execute(
            const StartupRequest& request);

        // Entradas que o UeX desativou e ainda podem ser restauradas.
        [[nodiscard]] static std::vector<StartupTarget> BackedUp();
    };
}
