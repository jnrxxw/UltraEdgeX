#pragma once

#include "system/SafetyLevel.h"
#include "system/StartupManager.h"

#include <string>

namespace real_optimizer::system
{
    // O que o UeX mostra sobre uma entrada de inicialização.
    struct StartupInfo
    {
        // Nome para a tela (o nome amigável, se o app for conhecido).
        std::wstring displayName;

        // O que o app faz e o que muda ao desativar.
        std::wstring description;

        SafetyLevel safety = SafetyLevel::Attention;

        // false = o UeX não conhece este app.
        bool known = false;
    };

    // Lista interna de apps conhecidos. Regra de ouro: app que o UeX
    // não reconhece NUNCA é Seguro; vem como Atenção.
    class StartupCatalog final
    {
    public:
        StartupCatalog() = delete;

        [[nodiscard]] static StartupInfo Describe(
            const StartupApp& app);

        // Critério único do que o UeX recomenda desativar: app
        // conhecido, de nível Seguro e que está ativo agora.
        // Usado por "Marcar recomendados".
        [[nodiscard]] static bool IsRecommendedToDisable(
            const StartupApp& app);
    };
}
