#pragma once

#include "system/SafetyLevel.h"

#include <string>
#include <vector>

namespace real_optimizer::system
{
    enum class ServiceStartType
    {
        Automatic,
        AutomaticDelayed,
        Manual,
        Disabled,
        Unknown
    };

    struct ServiceDefinition
    {
        std::wstring name;        // nome interno (ex.: SysMain)
        std::wstring displayName; // nome mostrado na tela
        std::wstring description; // o que faz e qual o impacto

        SafetyLevel safety = SafetyLevel::Safe;

        // Configuração padrão do Windows. Usada para reativar um
        // serviço que o UeX não tem backup (ex.: já veio desativado).
        ServiceStartType windowsDefault = ServiceStartType::Manual;
    };

    struct ServiceState
    {
        bool installed = false;

        // false = o serviço existe mas o Windows não deixou ler.
        bool readable = true;

        ServiceStartType startType = ServiceStartType::Unknown;
        bool running = false;
    };

    enum class ServiceOutcome
    {
        Changed,
        Unchanged,    // já estava como pedido
        NotInstalled, // este Windows não tem o serviço
        InUse,        // outros serviços ativos dependem dele
        Failed
    };

    struct ServiceChange
    {
        std::wstring name;
        ServiceOutcome outcome = ServiceOutcome::Failed;
        unsigned long error = 0;
    };

    struct ServiceRequest
    {
        std::vector<std::wstring> disable;
        std::vector<std::wstring> enable;
        bool restore = false;
    };

    class Services final
    {
    public:
        Services() = delete;

        // Lista fixa. O app nunca mexe em serviço fora dela.
        [[nodiscard]] static const std::vector<ServiceDefinition>&
        Definitions();

        [[nodiscard]] static const ServiceDefinition*
        Find(const std::wstring& name);

        // Só leitura. Não precisa de administrador.
        [[nodiscard]] static ServiceState Query(
            const std::wstring& serviceName);

        // Critério único do que é "desnecessário": nível Seguro, existe
        // neste Windows, pode ser lido, não está desativado e está
        // rodando ou com início automático. Serviço manual e parado
        // não muda nada na prática, então não conta.
        // Usado pela página Serviços (Marcar recomendados) e pelo Painel.
        [[nodiscard]] static bool IsRecommendedToDisable(
            const ServiceDefinition& definition,
            const ServiceState& state);

        // Quantos serviços da lista fixa se encaixam no critério acima.
        // Só leitura. Não precisa de administrador.
        [[nodiscard]] static std::size_t CountRecommendedToDisable();

        // ---- Alterações (precisam de administrador) ----

        // Desativa e tenta parar o serviço. Antes de mexer, grava a
        // configuração original no backup. Se houver serviços ativos
        // que dependem dele, não altera nada (InUse).
        [[nodiscard]] static ServiceChange Disable(
            const ServiceDefinition& definition);

        // Reativa: volta para a configuração do backup ou, se não
        // houver, para o padrão do Windows.
        [[nodiscard]] static ServiceChange Enable(
            const ServiceDefinition& definition);

        // Volta tudo que o UeX alterou à configuração original.
        [[nodiscard]] static std::vector<ServiceChange> RestoreAll();

        // Executa um pedido inteiro (só nomes da lista fixa).
        [[nodiscard]] static std::vector<ServiceChange> Execute(
            const ServiceRequest& request);

        // Serviços que o UeX alterou e ainda podem ser restaurados.
        [[nodiscard]] static std::vector<std::wstring> BackedUpNames();
    };
}
