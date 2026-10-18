#pragma once

namespace real_optimizer::system
{
    // Nível de segurança mostrado ao usuário (Cleaner e Serviços).
    enum class SafetyLevel
    {
        Safe,     // sem riscos conhecidos
        Attention // pode alterar o comportamento do Windows ou de apps
    };
}
