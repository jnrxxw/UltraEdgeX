DEC-001

Tipografia oficial:

Inter

---

DEC-002

Arquitetura:

Modular

---

DEC-003

Framework UI:

Win32 + Direct2D

---

DEC-004

Adicionar Health Score

---

DEC-005

Cleaner sempre explica antes de executar.

---

DEC-006

Toda recomendação importante deverá responder quatro perguntas:

1-O que é isso?

2-Por que estamos recomendando esta alteração?

3-Qual será o benefício esperado?

4-Existe algum risco ou impacto?

---

DEC-007

Toda funcionalidade nova deverá passar por uma revisão de produto antes de ser desenvolvida.

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

## Engenharia de Plataforma

- [ ] Configurar CMake
- [ ] Criar estrutura inicial de `src`
- [ ] Criar classe `Application`
- [ ] Criar abstração `Window`
- [ ] Inicializar Direct2D
- [ ] Inicializar DirectWrite
- [ ] Criar `UIElement`
- [ ] Criar componente `Text`
- [ ] Criar componente `Button`
- [ ] Validar mouse, hover e clique
- [ ] Criar primeira demonstração do ROUI

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

Aprovado.

---

## DEC-010 — Separação entre UI e lógica

Os componentes do ROUI não poderão executar diretamente funcionalidades como limpeza, monitoramento ou alterações no Windows.

A interface deverá comunicar a intenção do usuário para a camada de aplicação, que encaminhará a ação à Feature responsável.

### Status

Aprovado.
