# Technical Architecture

| Projeto | UltraEdgeX (UeX) |
|----------|----------------|
| Documento | Technical Architecture |
| Versão | 1.1 |
| Status | Draft |
| Autor | Junior |
| Última atualização | Outubro/2026 |

---

# Índice

1. Objetivo
2. Tecnologias
3. Arquitetura Geral
4. Camadas da Aplicação
5. Estrutura do Projeto
6. Fluxo de Inicialização
7. Padrão de uma Feature
8. Comunicação entre Módulos
9. Elevação (permissão de administrador)
10. Sistema de Configurações
11. Sistema de Logs
12. Gerenciamento de Memória
13. Threading
14. Tratamento de Erros
15. Build System
16. Princípios da Arquitetura
17. Desvios conhecidos

---

# 1. Objetivo

Este documento define a arquitetura técnica do UltraEdgeX (UeX).

Seu objetivo é estabelecer padrões para que o projeto permaneça organizado, escalável e de fácil manutenção durante todo o desenvolvimento. Ele descreve o que existe hoje; o que ainda é plano está marcado como *previsto*.

---

# 2. Tecnologias

## Linguagem

C++20

---

## Sistema Operacional

Windows 10

Windows 11

---

## Interface

Win32 API

Direct2D

DirectWrite

A camada de interface é própria do projeto: **ROUI** (Real Optimizer UI), DEC-008.

---

## Informações do sistema

Registro do Windows, DXGI (placa de vídeo), `powrprof` (plano de energia) e APIs Win32. O projeto evita WMI (DEC-016).

---

## Build

CMake

Compilador: MSVC.

---

## IDE Recomendada

Visual Studio 2022

Visual Studio Code

---

# 3. Arquitetura Geral

O software é organizado em módulos independentes, em quatro blocos:

app

↓

features

↓

system

↓

Windows API

O ROUI (`roui`) é a camada de interface, usada por `app` e `features`, e não conhece nenhuma funcionalidade do produto.

---

# 4. Camadas da Aplicação

UI (`roui` e as páginas em `features`)

↓

Application (`app` e os controllers de cada feature)

↓

Services (`system`)

↓

Platform (Windows)

### UI

Controles do ROUI e as páginas de cada feature.

Responsável apenas pela interface. Nunca altera o Windows (DEC-010).

---

### Application

Coordena a aplicação: inicialização, o `MainShell` (barra lateral e troca de páginas) e os controllers.

---

### Services

Implementam as funcionalidades, sem conhecer a interface:

Cleaner

Services

StartupManager e StartupCatalog

SystemInfo e SystemAnalyzer

Elevação (`ElevatedProcess` e os ajudantes)

---

### Platform

Comunicação com o Windows.

Registry

Filesystem

Win32

DXGI

---

# 5. Estrutura do Projeto

```
src/
  app/        Main, Application (entrada e ciclo de vida)
  core/       Logger, formatação de tamanhos
  features/
    shell/      MainShell: barra lateral e páginas
    dashboard/  Painel
    cleaner/    Limpeza (página e controller)
    startup/    Inicialização (página e controller)
    services/   Serviços (página e controller)
  roui/
    controls/   Button, Card, Checkbox, ConfirmDialog, DrivePicker,
                ImageBox, Panel, ProgressRing, ScrollPanel,
                SurfacePanel, Text
    core/       Theme, UIElement, Geometry, VisualTypes
    layout/     StackPanel
    rendering/  Renderer (Direct2D e DirectWrite)
    window/     Window, UiDispatcher
  system/     Cleaner, Services, StartupManager, StartupCatalog,
              SystemInfo, SystemAnalyzer, SafetyLevel, Elevated*
  resources/  recursos do executável
assets/       logo e ícone
docs/         esta documentação
```

*Previsto:* pasta `tests/`.

---

# 6. Fluxo de Inicialização

main()

↓

Verificar se foi iniciado como ajudante elevado (ver seção 9). Se sim, executa só a tarefa e encerra, sem criar janela.

↓

Criar Application

↓

Carregar preferência de tema

↓

Criar Janela Principal

↓

Criar o MainShell (barra lateral, páginas, ConfirmDialog)

↓

Exibir o Painel

↓

Painel lê as informações do sistema e atualiza CPU, memória e disco a cada segundo

A análise completa (Health Score e alertas) roda em segundo plano quando o usuário pede ou quando volta ao Painel depois de já ter analisado uma vez.

---

# 7. Padrão de uma Feature

Cada funcionalidade que altera o Windows segue o mesmo desenho (DEC-010):

Página (`XxxPage`)

↓ intenção do usuário

Controller (`XxxController`)

↓ trabalho fora da thread da interface

Service em `system/` (leitura e alteração)

↓ resultado

`UiDispatcher` devolve para a thread da interface

↓

Controller avisa a página por eventos (callbacks)

Regras:

• A página só desenha e coleta cliques. Não chama o Windows.

• O controller decide o que roda em segundo plano e nunca bloqueia a interface.

• O service não conhece a interface.

• Alterações ficam pendentes até o usuário confirmar, em um `ConfirmDialog`.

• Antes de alterar, o service grava o necessário para voltar atrás, quando a alteração é reversível.

---

# 8. Comunicação entre Módulos

A comunicação usa controllers, callbacks e o `UiDispatcher`.

Exemplo

Limpeza terminou

↓

Controller avisa a página (evento)

↓

Página atualiza o resultado

↓

Ao voltar ao Painel, `OnShown` refaz a análise e atualiza os alertas

Nenhuma página deve conhecer detalhes internos de outra. O `MainShell` é quem liga as páginas (por exemplo, abre a página de Serviços quando o Painel pede).

---

# 9. Elevação (permissão de administrador)

O app abre sem administrador. Quando uma ação precisa disso (limpar pastas do sistema, alterar serviços, alterar inicialização de todos os usuários), ela pede só na hora de aplicar (DEC-015).

Como funciona:

1. O controller chama `RunSelfElevated`, que abre o próprio executável com o verbo `runas` (o Windows mostra o pedido) e um parâmetro de ajudante: `--clean-elevated`, `--services-elevated` ou `--startup-elevated`.

2. O `main()` reconhece o parâmetro antes de criar a janela, executa só a tarefa e grava o resultado em um arquivo temporário.

3. O processo principal lê e **valida** o arquivo de resultado e mostra o resumo.

Regras de segurança:

• O ajudante só aceita itens de uma lista fixa ou que existem agora no sistema (por exemplo, nomes que estão na chave `Run` de HKLM). Nomes com espaço, vírgula ou acento viajam em hexadecimal.

• Alterações do usuário atual (HKCU) nunca vão para o ajudante elevado.

• Se o usuário recusar, o resto da ação continua normalmente e o app informa.

• A Limpeza também tem **Reabrir como admin** (`--open-cleaner`), que reabre o app já na página de Limpeza.

---

# 10. Sistema de Configurações

Todas as preferências do usuário ficarão centralizadas na tela de Configurações (*pendente*).

Hoje:

• Tema (escuro ou claro): salvo por usuário em `HKCU\Software\UltraEdgeX`, valor `Theme`.

• Backups de alterações (ex.: configuração original de serviços em `%LOCALAPPDATA%\UltraEdgeX\services-backup.txt`).

*Previsto:* idioma, atualizações e preferências como lembrar o disco escolhido no Painel.

---

# 11. Sistema de Logs

Todas as ações importantes são registradas pelo `Logger`.

Arquivo:

`%LOCALAPPDATA%\UltraEdgeX\logs\ultraedgex.log`

• Níveis: Info, Warning e Error.

• Seguro para várias threads e para mais de um processo ao mesmo tempo.

• Quando passa de 1 MB, o arquivo antigo vira `ultraedgex.old.log`.

Objetivo

Facilitar depuração.

Gerar histórico.

Melhorar suporte.

*Pendente:* tela de Logs dentro do app.

---

# 12. Gerenciamento de Memória

O projeto utiliza recursos modernos do C++.

Preferência por:

RAII

std::unique_ptr (os controles da interface são donos dos filhos)

Microsoft::WRL::ComPtr para recursos do Direct2D e DirectWrite

std::shared_ptr apenas quando necessário.

Evitar alocação manual.

Evitar vazamentos de memória.

---

# 13. Threading

A Interface nunca pode travar.

Operações demoradas rodam em threads separadas e devolvem o resultado pelo `UiDispatcher`.

Exemplos

Análise do Painel

Medição e limpeza

Leitura e alteração de serviços e de inicialização

Os controllers só tocam no próprio estado na thread da interface e esperam a thread de trabalho terminar ao encerrar (`Shutdown`).

Leituras rápidas de hardware (disco, placa de vídeo) acontecem na thread da interface, de forma sob demanda (por exemplo, ao abrir o menu de discos), e não a cada atualização de tela.

---

# 14. Tratamento de Erros

Todos os erros devem ser tratados.

O usuário nunca deve receber mensagens técnicas.

Exemplo

Errado

Erro 0x00000457

Correto

Não foi possível concluir esta operação.

Verifique se você possui permissões de administrador.

O código técnico fica no log.

---

# 15. Build System

Utilizado:

CMake

Bibliotecas do Windows ligadas: `d2d1`, `dwrite`, `dxgi`, `powrprof`, `advapi32`, entre outras (ver `CMakeLists.txt`).

Objetivos

Compilação organizada.

Escalabilidade.

Compatibilidade com múltiplas IDEs.

Regra: todo arquivo `.cpp` novo precisa ser listado no `CMakeLists.txt`.

---

# 16. Princípios da Arquitetura

Uma classe deve possuir apenas uma responsabilidade.

Separar Interface da Lógica.

Evitar dependências desnecessárias.

Criar código reutilizável.

Priorizar legibilidade.

Documentar componentes públicos.

Utilizar nomes claros.

Evitar código duplicado.

Um critério, um lugar: regras usadas em mais de uma tela ficam em um único ponto (por exemplo `Services::IsRecommendedToDisable` e `StartupCatalog::IsRecommendedToDisable`), para os números sempre baterem.

Priorizar desempenho sem comprometer a manutenção.

A arquitetura deve permitir crescimento do projeto sem necessidade de grandes refatorações.

---

# 17. Desvios conhecidos

Pontos em que o código ainda não segue a regra de nenhum módulo conhecer outro:

• O Painel usa o `StartupCatalog` diretamente (nomes amigáveis dos apps de inicialização) e a análise do Painel usa `Cleaner` e `Services` para contar os alertas. É aceito porque essas classes são a fonte única dos critérios, e isso evita números diferentes entre telas. A alternativa seria uma camada de análise que concentre tudo.

• O `StartupController` e o `StartupManager` ainda têm código de "restaurar" que a tela de Inicialização não usa mais (a página não tem o botão).

Esses pontos estão em [Ideas](10-Ideas.md) como itens de limpeza.
