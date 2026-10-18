# Software Design Document (SDD)

| Projeto | UltraEdgeX (UeX) |
|----------|----------------|
| Documento | Software Design Document |
| Versão | 1.1 |
| Status | Em desenvolvimento |
| Autor | Junior |
| Última atualização | Outubro/2026 |

---

# Índice

1. Objetivo
2. Escopo
3. Objetivos da Versão 1.0
4. Arquitetura Funcional
5. Fluxo Principal
6. Módulos
7. Funcionalidades Principais
8. Regras do Sistema
9. Requisitos Não Funcionais
10. Fora do Escopo
11. Roadmap Técnico
12. Glossário
13. Casos de Uso

---

# 1. Objetivo

O UltraEdgeX (UeX) é um software para Windows desenvolvido para monitorar, analisar e otimizar o sistema operacional de forma segura, transparente e intuitiva.

Seu principal objetivo é ajudar o usuário a compreender o estado do computador antes de executar qualquer alteração, apresentando recomendações claras e permitindo total controle sobre cada otimização.

---

# 2. Escopo

A versão 1.0 será focada na criação de uma base sólida para o software.

O projeto priorizará qualidade, estabilidade, desempenho e experiência do usuário em vez da quantidade de funcionalidades.

---

# 3. Objetivos da Versão 1.0

A primeira versão deverá permitir:

• Monitoramento do computador

• Health Score

• Plano de Otimização

• Limpeza (Cleaner)

• Serviços

• Inicialização (Startup Manager)

• Informações do hardware

• Configurações

• Sistema de Logs

---

# 4. Arquitetura Funcional

O software é dividido em módulos independentes. Cada módulo corresponde a uma página da barra lateral ou a uma capacidade compartilhada.

Painel (Dashboard)

↓

Monitoring

↓

Limpeza

↓

Inicialização

↓

Serviços

↓

Hardware

↓

Configurações

↓

Logs

Cada módulo será responsável apenas pela sua própria funcionalidade. Os detalhes técnicos estão em [Technical Architecture](04-TechnicalArchitecture.md).

---

# 5. Fluxo Principal

Inicialização

↓

Carregar Configurações (tema)

↓

Inicializar módulos

↓

Coletar informações

↓

Calcular Health Score

↓

Exibir Painel

↓

Analisar Sistema (botão "Analisar sistema")

↓

Gerar alertas e Plano de Otimização

↓

Usuário revisa

↓

Executar alterações (na página do módulo, com confirmação)

↓

Atualizar Health Score

↓

Registrar Log

---

# 6. Módulos

Legenda de estado: **Pronto**, **Parcial** ou **Pendente**.

## Painel (Dashboard) — Pronto

### Responsabilidades

• Exibir o Health Score.

• Mostrar o resumo do computador (10 itens em duas colunas: sistema, computador, processador, placa de vídeo, memória, placa-mãe, BIOS, monitor, tempo ligado e plano de energia).

• Mostrar CPU, memória e disco em tempo real. O card de disco permite escolher qual HD ou SSD exibir.

• Mostrar alertas importantes, cada um com atalho para a página que resolve.

• Mostrar os aplicativos de inicialização ativos.

### Fora do Escopo

• Não executa otimizações.

• Não altera configurações.

• Não modifica arquivos.

---

## Monitoring — Parcial

### Responsabilidades

• Coletar informações do computador.

• CPU

• GPU

• RAM

• Disco (todos os HDs e SSDs fixos)

• Sistema Operacional

• Placa-mãe e BIOS

• Monitor e plano de energia

• Temperaturas (previsto, ainda não implementado)

A coleta evita WMI e usa registro, DXGI e APIs do Windows (DEC-016).

### Fora do Escopo

• Não altera nenhuma configuração do sistema.

---

## Limpeza (Cleaner) — Pronto

### Responsabilidades

• Medir 9 categorias fixas de arquivos temporários e caches (usuário e Windows, Windows Update, cache de entrega, relatórios de erro, dumps de memória, miniaturas, shaders e Prefetch).

• Explicar cada categoria e exibir o nível de segurança.

• Executar a limpeza somente após confirmação, pedindo administrador só na hora de limpar.

### Fora do Escopo

• Não remove arquivos pessoais.

• Não altera Registro do Windows.

• Não remove programas instalados.

• Não esvazia a Lixeira (DEC-018).

---

## Inicialização (Startup Manager) — Pronto

### Responsabilidades

• Listar os programas que iniciam com o Windows: chaves `Run` do usuário atual, de todos os usuários e de apps de 32 bits, e os apps da Microsoft Store.

• Explicar cada item e exibir o nível de segurança. Item que o UeX não reconhece vem como Atenção (DEC-013).

• Permitir ativar e desativar, de forma reversível e sem apagar nada (DEC-014).

• Confirmar antes de aplicar.

### Fora do Escopo

• Não encerra processos em execução.

• Não mostra "impacto na inicialização", porque o UeX não consegue medir isso de forma confiável (honestidade).

• Pasta Inicializar do menu Iniciar e tarefas agendadas ficam para depois.

---

## Serviços — Pronto

### Responsabilidades

• Listar uma lista fixa de 15 serviços do Windows (8 de nível Seguro e 7 de Atenção) com descrição do que fazem e do impacto de desativar (DEC-017).

• Aplicar mudanças após confirmação, gravando a configuração original antes de alterar.

• Permitir restaurar o padrão.

### Fora do Escopo

• Serviços essenciais do Windows (Windows Update, Defender, rede, áudio etc.) nunca fazem parte da lista.

---

## Hardware Information — Parcial

### Responsabilidades

Exibir:

• CPU

• GPU

• RAM

• Placa-mãe

• SSD/HDD

• Sistema Operacional

O resumo já aparece no Painel. Uma página própria, com mais detalhes, está pendente.

---

## Configurações (Settings) — Parcial

### Responsabilidades

• Tema (pronto; a preferência é salva entre execuções)

• Idioma (pendente)

• Atualizações (pendente)

• Preferências, como lembrar o disco escolhido no Painel (pendente)

---

## Logs — Parcial

### Responsabilidades

Registrar:

• Inicialização

• Análises

• Otimizações

• Erros

• Avisos

O registro em arquivo está pronto. A tela para ler os logs dentro do app está pendente.

---

# 7. Funcionalidades Principais

## Health Score

Representa a saúde geral do computador.

Previsto para ser composto por cinco pilares:

• Performance

• Cleanliness

• Security

• System Health

• Maintenance

*Estado atual:* uma pontuação única, reduzida pelos alertas encontrados (arquivos temporários e serviços desnecessários).

---

## Plano de Otimização

Após analisar o computador, o software criará uma lista organizada contendo todas as recomendações.

Cada item apresentará:

• Descrição

• Benefício esperado

• Possíveis riscos

• Nível de segurança

• Confidence Score (pendente)

---

## Confidence Score

Cada recomendação possuirá uma porcentagem indicando o nível de confiança da recomendação. Ainda não implementado.

---

## Níveis de Segurança

🟢 Seguro

Pode ser executado normalmente.

🟡 Atenção

Pode alterar o comportamento do Windows.

🔴 Avançado

Recomendado apenas para usuários experientes. Previsto, ainda não usado.

---

# 8. Regras do Sistema

O software sempre seguirá o fluxo:

Analisar

↓

Explicar

↓

Recomendar

↓

Executar

↓

Mostrar Resultados

Nunca serão realizadas alterações ocultas.

O usuário sempre terá a decisão final.

Regras adicionais já aplicadas:

• Toda alteração no sistema passa por uma confirmação que lista o que será feito.

• O que o UeX não reconhece nunca é marcado como Seguro.

• Alterações são reversíveis sempre que possível.

• A permissão de administrador só é pedida na hora de aplicar, nunca na abertura do app.

---

# 9. Requisitos Não Funcionais

• Interface rápida.

• Inicialização em poucos segundos.

• Baixo consumo de memória.

• Baixo consumo de CPU.

• Interface responsiva (tamanho mínimo da janela: 1180×700).

• Código modular.

• Fácil manutenção.

• Compatível com Windows 10 e Windows 11.

---

# 10. Fora do Escopo

A versão 1.0 NÃO incluirá:

• Driver Updater

• Benchmark

• Plugins

• IA

• Sincronização em nuvem

• Game Booster

• Overclock

• Otimizações automáticas sem confirmação

• Limpeza da Lixeira

---

# 11. Roadmap Técnico

Versão 1.0

Painel

Limpeza

Serviços

Inicialização

Hardware

Configurações

Logs

Versão 2.0

Benchmark

Plugins

Monitor em tempo real (gráficos)

Health History

Versão 3.0

Marketplace de Plugins

Perfis de desempenho

Sistema inteligente de recomendações

---

# 12. Glossário

Health Score

Pontuação geral da saúde do computador.

---

Confidence Score

Nível de confiança da recomendação.

---

Plano de Otimização

Lista organizada de todas as ações recomendadas para aquele computador.

---

ROUI

Camada de interface própria do projeto (Real Optimizer UI), independente das funcionalidades.

---

UeX

Nome abreviado do UltraEdgeX na interface.

---

# 13. Casos de Uso (Use Cases)

## UC-001 – Analisar o computador

Ator: Usuário

Fluxo:

1. O usuário abre o UeX.
2. Clica em **Analisar sistema**.
3. O sistema coleta informações em segundo plano.
4. Calcula o Health Score.
5. Gera os alertas (Plano de Otimização).
6. Exibe os resultados, cada alerta com atalho para a página certa.

## UC-002 – Executar uma otimização

1. O usuário abre o alerta ou a página do módulo (por exemplo, Limpeza).
2. Lê a explicação e o nível de segurança de cada item.
3. Marca o que deseja e clica em aplicar.
4. O sistema mostra a confirmação com o que será feito.
5. O usuário confirma; se preciso, o Windows pede permissão de administrador.
6. O sistema aplica a alteração e mostra o resultado.
7. O Painel atualiza o Health Score ao ser exibido de novo, e a ação fica registrada nos Logs.

## UC-003 – Escolher qual disco ver no Painel

1. O usuário clica na seta do card de disco (aparece quando há mais de um HD ou SSD).
2. Escolhe um disco no menu.
3. O card mostra tamanho e espaço livre do disco escolhido, até fechar o app.
