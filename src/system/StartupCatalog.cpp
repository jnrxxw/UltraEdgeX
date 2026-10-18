#include "system/StartupCatalog.h"

#include <cwctype>
#include <initializer_list>
#include <string>
#include <vector>

namespace real_optimizer::system
{
    namespace
    {
        struct Rule
        {
            // Trechos em minúsculas. O app é reconhecido se o nome da
            // entrada ou o nome do .exe contiver algum deles.
            std::vector<std::wstring> tokens;

            const wchar_t* displayName;
            const wchar_t* description;
            SafetyLevel safety;
        };

        std::wstring Lower(std::wstring text)
        {
            for (wchar_t& c : text)
            {
                c = static_cast<wchar_t>(std::towlower(c));
            }

            return text;
        }

        // Nome do arquivo do programa, a partir do comando
        // (com ou sem aspas, com argumentos).
        std::wstring ExecutableName(const std::wstring& command)
        {
            std::wstring path;

            if (!command.empty() && command.front() == L'"')
            {
                const std::size_t end = command.find(L'"', 1);

                path = end == std::wstring::npos
                           ? command.substr(1)
                           : command.substr(1, end - 1);
            }
            else
            {
                const std::wstring lower = Lower(command);
                const std::size_t exe = lower.find(L".exe");

                if (exe != std::wstring::npos)
                {
                    path = command.substr(0, exe + 4);
                }
                else
                {
                    const std::size_t space = command.find(L' ');

                    path = space == std::wstring::npos
                               ? command
                               : command.substr(0, space);
                }
            }

            const std::size_t slash = path.find_last_of(L"\\/");

            if (slash != std::wstring::npos)
            {
                path = path.substr(slash + 1);
            }

            return Lower(path);
        }

        const std::vector<Rule>& Rules()
        {
            static const std::vector<Rule> rules = {
                // ---- Seguro: apps de conveniência ----
                {{L"spotify"},
                 L"Spotify",
                 L"Abre o Spotify junto com o Windows. Você pode abrir manualmente quando quiser.",
                 SafetyLevel::Safe},
                {{L"discord"},
                 L"Discord",
                 L"Abre o Discord ao ligar o PC. Você pode abrir manualmente quando quiser.",
                 SafetyLevel::Safe},
                // O Teams vem antes do Steam: "msteams" contém "steam".
                {{L"squirrel.teams", L"ms-teams", L"msteams", L"teams.exe"},
                 L"Microsoft Teams",
                 L"Abre o Teams ao ligar o PC. Você não recebe mensagens até abri-lo.",
                 SafetyLevel::Safe},
                {{L"steam"},
                 L"Steam",
                 L"Abre a Steam ao ligar o PC. Seus jogos continuam funcionando; é só abrir a Steam quando for jogar.",
                 SafetyLevel::Safe},
                {{L"epicgameslauncher"},
                 L"Epic Games Launcher",
                 L"Abre o launcher da Epic ao ligar o PC. Pode ser aberto manualmente.",
                 SafetyLevel::Safe},
                {{L"battle.net"},
                 L"Battle.net",
                 L"Abre o launcher da Blizzard ao ligar o PC. Pode ser aberto manualmente.",
                 SafetyLevel::Safe},
                {{L"onedrive"},
                 L"Microsoft OneDrive",
                 L"A sincronização de arquivos só começa depois que você abrir o OneDrive.",
                 SafetyLevel::Safe},
                {{L"dropbox"},
                 L"Dropbox",
                 L"A sincronização de arquivos só começa depois que você abrir o Dropbox.",
                 SafetyLevel::Safe},
                {{L"skype"},
                 L"Skype",
                 L"Abre o Skype ao ligar o PC. Você não recebe chamadas até abri-lo.",
                 SafetyLevel::Safe},
                {{L"zoomopener"},
                 L"Zoom",
                 L"Abre o Zoom ao ligar o PC. Ele abre normalmente quando você entrar em uma reunião.",
                 SafetyLevel::Safe},
                {{L"slack"},
                 L"Slack",
                 L"Abre o Slack ao ligar o PC. Você não recebe mensagens até abri-lo.",
                 SafetyLevel::Safe},
                {{L"telegram"},
                 L"Telegram",
                 L"Abre o Telegram ao ligar o PC. Você não recebe mensagens até abri-lo.",
                 SafetyLevel::Safe},
                {{L"whatsapp"},
                 L"WhatsApp",
                 L"Abre o WhatsApp ao ligar o PC. Você não recebe mensagens até abri-lo.",
                 SafetyLevel::Safe},
                {{L"aamupdater", L"creative cloud", L"creativecloud", L"ccxprocess"},
                 L"Adobe (atualizador / Creative Cloud)",
                 L"Mantém os apps da Adobe atualizados em segundo plano. Os programas continuam funcionando sem ele.",
                 SafetyLevel::Safe},
                {{L"googlechromeautolaunch"},
                 L"Google Chrome (início antecipado)",
                 L"Deixa o Chrome pré-carregado. Sem ele, só a primeira abertura depois de ligar o PC fica um pouco mais lenta.",
                 SafetyLevel::Safe},
                {{L"microsoftedgeautolaunch"},
                 L"Microsoft Edge (início antecipado)",
                 L"Deixa o Edge pré-carregado. Sem ele, só a primeira abertura depois de ligar o PC fica um pouco mais lenta.",
                 SafetyLevel::Safe},

                // ---- Seguro: apps da Microsoft Store ----
                // (o nome é "<pacote>\\<tarefa>", então o trecho vem do pacote)
                {{L"windowsterminal"},
                 L"Terminal do Windows",
                 L"Abre o Terminal ao entrar no Windows. Você pode abrir manualmente quando precisar.",
                 SafetyLevel::Safe},
                {{L"gamingapp", L"xbox"},
                 L"Xbox",
                 L"Abre o app Xbox ao ligar o PC. Seus jogos continuam funcionando; abra o app quando precisar.",
                 SafetyLevel::Safe},
                {{L"copilot", L"officehub"},
                 L"Microsoft 365 / Copilot",
                 L"Abre o app em segundo plano ao ligar o PC. Ele abre normalmente quando você usar.",
                 SafetyLevel::Safe},
                {{L"translucenttb"},
                 L"TranslucentTB",
                 L"Deixa a barra de tarefas transparente. Sem ele, a barra volta ao visual padrão do Windows.",
                 SafetyLevel::Safe},
                {{L"claude"},
                 L"Claude",
                 L"Abre o app Claude ao ligar o PC. Pode ser aberto manualmente quando quiser.",
                 SafetyLevel::Safe},

                // ---- Atenção: conhecidos, mas com impacto ----
                {{L"securityhealth"},
                 L"Segurança do Windows (ícone)",
                 L"Mostra o ícone e os avisos da Segurança do Windows. A proteção continua ativa, mas você pode deixar de ver alertas.",
                 SafetyLevel::Attention},
                {{L"avast", L"avgui", L"kaspersky", L"norton", L"mcafee",
                  L"bitdefender", L"malwarebytes", L"avira"},
                 L"Antivírus",
                 L"Parte do seu antivírus. Desativar pode deixar o PC desprotegido logo depois de ligar.",
                 SafetyLevel::Attention},
                {{L"rtkaud", L"realtek", L"rthdvcpl", L"nahimic"},
                 L"Áudio (gerenciador do fabricante)",
                 L"Controla efeitos de som e avisos de fones. Desativar pode tirar esses recursos.",
                 SafetyLevel::Attention},
                {{L"nvidia", L"nvbackend", L"radeonsoftware", L"amdnoise",
                  L"igfxtray", L"intel(r) graphics"},
                 L"Placa de vídeo (painel do fabricante)",
                 L"Ajustes e perfis da placa de vídeo. O driver continua funcionando, mas alguns recursos podem parar.",
                 SafetyLevel::Attention},
                {{L"syntp", L"synaptics", L"etdctrl", L"elantech", L"apoint"},
                 L"Touchpad (driver)",
                 L"Gestos e rolagem do touchpad podem parar de funcionar se for desativado.",
                 SafetyLevel::Attention},
                {{L"logitech", L"logioptions", L"lghub", L"razer", L"steelseries",
                  L"corsair", L"icue", L"armoury"},
                 L"Software de periférico",
                 L"Perfis, macros e iluminação do mouse, teclado ou headset podem não carregar se for desativado.",
                 SafetyLevel::Attention},
            };

            return rules;
        }

        std::wstring ShortenPath(const std::wstring& text)
        {
            constexpr std::size_t MaxLength = 70;

            if (text.size() <= MaxLength)
            {
                return text;
            }

            return text.substr(0, MaxLength - 1) + L"…";
        }
    }

    StartupInfo StartupCatalog::Describe(const StartupApp& app)
    {
        const std::wstring name = Lower(app.name);
        const std::wstring executable = ExecutableName(app.command);

        for (const Rule& rule : Rules())
        {
            for (const std::wstring& token : rule.tokens)
            {
                if (name.find(token) != std::wstring::npos ||
                    executable.find(token) != std::wstring::npos)
                {
                    StartupInfo info;
                    info.displayName = rule.displayName;
                    info.description = rule.description;
                    info.safety = rule.safety;
                    info.known = true;
                    return info;
                }
            }
        }

        // Desconhecido: nunca é Seguro.
        StartupInfo info;
        info.displayName = app.name;
        info.safety = SafetyLevel::Attention;
        info.known = false;

        if (app.source == StartupSource::PackagedApp)
        {
            // Nome no formato "<pacote>_<id>\\<tarefa>": mostra só o pacote.
            const std::size_t underscore = app.name.find(L'_');
            const std::size_t slash = app.name.find(L'\\');

            const std::size_t end =
                underscore != std::wstring::npos ? underscore : slash;

            if (end != std::wstring::npos && end > 0)
            {
                info.displayName = app.name.substr(0, end);
            }

            info.description =
                L"App da Microsoft Store não reconhecido pelo UeX. Confira antes de desativar.";

            return info;
        }

        info.description =
            L"App não reconhecido pelo UeX. Confira antes de desativar: " +
            (executable.empty() ? ShortenPath(app.command) : executable);

        return info;
    }

    bool StartupCatalog::IsRecommendedToDisable(const StartupApp& app)
    {
        if (!app.enabled || app.locked)
        {
            return false;
        }

        const StartupInfo info = Describe(app);

        return info.known && info.safety == SafetyLevel::Safe;
    }
}
