# Decision Log

Cada decisão tem número fixo. Uma decisão nunca é renumerada; quando muda, a nova decisão a substitui e o status da antiga passa a "Substituída".

---

## DEC-001 — Tipografia oficial

Tipografia oficial: **Inter**.

### Nota

O app ainda usa Segoe UI; a troca para a Inter está pendente (ver Brand Guide).

### Status

Aprovado.

---

## DEC-002 — Arquitetura

Arquitetura **modular**.

### Status

Aprovado.

---

## DEC-003 — Framework de interface

Framework UI: **Win32 + Direct2D**.

### Status

Aprovado.

---

## DEC-004 — Health Score

Adicionar **Health Score**.

### Status

Aprovado. Implementado como pontuação única.

---

## DEC-005 — Cleaner explica antes de executar

O Cleaner sempre explica antes de executar.

### Status

Aprovado.

---

## DEC-006 — Quatro perguntas por recomendação

Toda recomendação importante deverá responder quatro perguntas:

1. O que é isso?
2. Por que estamos recomendando esta alteração?
3. Qual será o benefício esperado?
4. Existe algum risco ou impacto?

### Status

Aprovado.

---

## DEC-007 — Revisão de produto antes de desenvolver

Toda funcionalidade nova deverá passar por uma revisão de produto antes de ser desenvolvida:

Qual problema resolve?

↓

Quem vai utilizar?

↓

Como o usuário encontra essa função?

↓

É realmente necessária?

↓

Pode confundir o usuário?

↓

Vale a pena manter para a versão 1.0?

### Status

Aprovado.

---

## DEC-008 — Nome da camada de interface

Foi definido o nome **ROUI — Real Optimizer UI** para a camada de interface reutilizável do projeto.

### Motivo

Centralizar renderização, componentes, entrada, layout e temas em uma camada independente das funcionalidades do produto.

### Status

Aprovado.

---

## DEC-009 — Escopo inicial do ROUI

A primeira versão do ROUI terá apenas os componentes necessários para validar a plataforma:

- Window
- Renderer
- UIElement
- Panel
- Text
- Button

Componentes adicionais serão implementados apenas quando houver necessidade real.

### Motivo

Evitar complexidade prematura e manter o desenvolvimento focado no produto.

### Status

Aprovado. Cumprida: os componentes novos (SurfacePanel, Checkbox, ScrollPanel, ConfirmDialog, DrivePicker e outros) entraram só quando uma tela precisou.

---

## DEC-010 — Separação entre UI e lógica

Os componentes do ROUI não poderão executar diretamente funcionalidades como limpeza, monitoramento ou alterações no Windows.

A interface deverá comunicar a intenção do usuário para a camada de aplicação, que encaminhará a ação à Feature responsável.

### Status

Aprovado.

---

## DEC-011 — Cor primária

A cor primária do produto é o roxo **`#AC00FF`**. Em textos sobre fundo escuro, usa-se `#C766FF` para manter o contraste.

### Motivo

Destaque forte sobre o fundo escuro, alinhado ao estilo moderno e levemente gamer definido no Brand Guide.

### Status

Aprovado. Aplicado na v0.1.1.

---

## DEC-012 — Nome do produto

O nome oficial é **UltraEdgeX**, abreviado como **UeX** na interface. A barra de título e telas institucionais usam o nome completo.

"Real Optimizer" era o nome provisório. Continua apenas em nomes técnicos do código (namespace `real_optimizer` e a camada ROUI).

### Status

Aprovado.

---

## DEC-013 — Nível de segurança na Inicialização

Aplicativos de inicialização não têm lista fixa (cada PC é diferente). O UeX só marca como **Seguro** apps que estão no catálogo interno. Qualquer app não reconhecido vem como **Atenção** e desmarcado para ação em lote. Nada de segurança nem de componentes do Windows entra como Seguro.

### Motivo

Seguir a filosofia do produto: nenhuma recomendação por palpite. Errar para o lado conservador.

### Status

Aprovado. (Antes numerada como DEC-011.)

---

## DEC-014 — Alterações de inicialização reversíveis

O UeX nunca apaga uma entrada de inicialização. Ele só muda a marca de habilitado que o próprio Windows usa: `StartupApproved` para as chaves `Run`, e o estado da tarefa para apps da Loja. É o mesmo mecanismo do Gerenciador de Tarefas.

Para voltar atrás, basta reativar o interruptor e aplicar de novo.

### Motivo

Segurança e reversibilidade: o usuário nunca perde o item.

### Status

Aprovado.

---

## DEC-015 — Administrador só na hora de aplicar

O app abre sem permissão de administrador. Quando uma ação precisa dela (pastas do sistema, serviços, inicialização de todos os usuários), o UeX abre um processo auxiliar elevado só para aquela tarefa. O Windows mostra o pedido de confirmação.

O auxiliar só aceita itens de uma lista fixa ou que existem agora no sistema, e o resultado é validado pelo processo principal.

### Motivo

Menos poder por padrão, e o usuário vê exatamente quando o administrador é usado.

### Status

Aprovado.

---

## DEC-016 — Coleta de informações sem WMI

As informações do sistema são lidas pelo registro, DXGI e APIs do Windows, sem WMI.

### Motivo

WMI exige COM e costuma demorar mais para responder, o que pesaria na abertura do app. As fontes escolhidas são leituras diretas e rápidas.

### Status

Aprovado.

---

## DEC-017 — Listas fixas para Limpeza e Serviços

A Limpeza trabalha com 9 categorias fixas, e a página de Serviços com 15 serviços fixos. O UeX não descobre nem apaga nada fora dessas listas.

### Motivo

Ações que podem quebrar o Windows não podem depender de palpite.

### Status

Aprovado.

---

## DEC-018 — Lixeira fora da V1

A limpeza da Lixeira não faz parte da V1. A Limpeza atual cobre as 9 categorias de DEC-017.

### Motivo

Foco em estabilidade e na conclusão de Logs e Configurações antes de ampliar a Limpeza. Pode ser reavaliada depois.

### Status

Aprovado.
