# UI Design System

| Projeto | UltraEdgeX (UeX) |
|----------|----------------|
| Documento | UI Design System |
| Versão | 1.1 |
| Status | Draft |
| Autor | Junior |
| Última atualização | Outubro/2026 |

---

# 1. Filosofia da Interface

A interface deve transmitir:

• Clareza

• Rapidez

• Organização

• Segurança

• Modernidade

O usuário deve conseguir utilizar o software sem consultar um manual.

Toda informação importante deve ser visível.

---

# 2. Grid e medidas

O software utiliza um grid consistente, com espaçamentos preferencialmente múltiplos de 8.

Exemplos

8 px

16 px

24 px

32 px

48 px

64 px

Medidas fixas do layout:

| Elemento | Valor |
|----------|-------|
| Barra lateral | 232 px de largura |
| Margem lateral das páginas | 40 px |
| Tamanho mínimo da área de conteúdo | 1180×700 |

Alguns valores internos (linhas de lista com 56 a 68 px, paddings de 22 e 26 px) não são múltiplos de 8; foram ajustados para encaixar o conteúdo. Novos componentes devem preferir múltiplos de 8.

---

# 3. Bordas

Raios usados no app:

| Elemento | Raio |
|----------|------|
| Cards, painéis principais e caixas de diálogo | 16 px |
| Menus (escolha de disco) | 14 px |
| Áreas internas de listas | 12 px |
| Linhas de aviso e itens de menu | 10 px |
| Selos (Seguro/Atenção) e interruptor | totalmente arredondados |

---

# 4. Cards

O card de métrica (`Card`) mostra:

Título

Valor em destaque

Descrição

Barra de progresso

Opcionalmente, uma **seta de seleção** no canto superior direito, que abre um menu (por exemplo, para escolher o disco). A seta só aparece quando há o que escolher.

Padding interno consistente (22 px).

---

# 5. Botões

Estilos existentes

Primary (preenchido na cor de destaque)

Secondary (contorno)

Nav (item da barra lateral)

Segment (opção de um seletor segmentado, como Escuro/Claro)

Estados existentes

Normal

Hover

Pressed

Disabled

Enquanto uma ação está em andamento, o botão muda o texto (por exemplo, "Analisando…", "Aplicando…") e fica desativado.

*Previstos:* Ghost, Danger e um estado Loading próprio.

---

# 6. Inputs

Ainda não existem campos de texto. Quando forem necessários, devem ter:

Label

Placeholder

Mensagem de erro

Ícone opcional

---

# 7. Sidebar

Sempre fixa.

Itens organizados verticalmente:

Painel

Limpeza

Inicialização

Serviços

Configurações (com o selo "Em breve")

Texto curto.

Indicador de página ativa.

No rodapé, o bloco "Este computador" mostra o nome do computador e o sistema.

---

# 8. Cabeçalho das páginas

Padrão de toda página:

Título (28 px, Bold)

Subtítulo (14 px)

Ação principal à direita (por exemplo, "Analisar sistema" no Painel e "Atualizar" em Inicialização)

O seletor de tema (Escuro / Claro) fica no canto superior direito.

---

# 9. Páginas

## Painel

O Painel é a tela principal.

Elementos

Cards de CPU, memória e disco (o de disco com seta de escolha)

Health Score (anel) e alertas com atalho para a página que resolve

Informações do sistema em duas colunas (10 itens)

Aplicativos de inicialização ativos, com atalho para a página

## Páginas de lista (Limpeza, Inicialização e Serviços)

Padrão

Cabeçalho (título, subtítulo e ação)

Linha de status

Lista dentro de uma superfície, com rolagem

Em cada linha: interruptor ou caixa de seleção, nome, descrição, selo de nível (Seguro ou Atenção) e estado

Rodapé com resumo ("Nenhuma alteração pendente") e o botão de aplicar

Regra: a lista só muda algo depois de **Aplicar alterações** e de uma confirmação.

---

# 10. Gráficos

Existentes

Circular (anel do Health Score)

Barra (progresso)

Previstos

Radar

Linha

Todos devem utilizar a paleta oficial.

Nunca utilizar excesso de cores.

---

# 11. Badges

Existentes

Seguro (verde suave)

Atenção (amarelo suave)

Previstos

Error

Info

---

# 12. Tooltips

Ainda não existem. Quando forem criados, todo botão importante deverá possuir um tooltip em linguagem simples.

---

# 13. Dialogs e menus

## ConfirmDialog

Confirmação feita pelo componente `ConfirmDialog` do ROUI: escurece a janela, mostra um cartão com o que será afetado e bloqueia o resto da tela. Esc cancela; o Enter não confirma ações destrutivas.

Aceita até 16 itens, até 3 avisos (informação ou atenção) e uma linha de total opcional.

Nenhuma ação crítica será executada sem confirmação.

## DrivePicker

Menu pequeno, ancorado abaixo da seta do card. Não escurece a janela. Fecha ao escolher, ao clicar fora, com Esc ou ao trocar de página. Aceita setas do teclado e Enter. Mostra até 8 itens, cada um com título, detalhe e barra de uso.

---

# 14. Ícones

Mesmo estilo visual.

Mesmo tamanho base.

Mesmo alinhamento.

Hoje a interface quase não usa ícones; as marcas (check e seta) são desenhadas com linhas simples no próprio controle.

---

# 15. Animações

Duração curta.

Transições suaves.

Jamais prejudicar desempenho.

Hoje não há animações.

---

# 16. Responsividade

A interface funciona a partir de 1180×700 de área de conteúdo, e o layout é recalculado ao redimensionar.

Textos longos são encurtados com "…" conforme a largura disponível (por exemplo, as informações do sistema no Painel).

Escala ideal:

1920x1080

---

# 17. Estados

Estados implementados

Normal

Hover

Pressed

Disabled

Success (verde) e aviso (amarelo), por cores semânticas

Estados previstos

Focused

Loading (como estado do componente)

Error

---

# 18. Acessibilidade

Contraste adequado.

Textos legíveis.

Ícones acompanhados de texto quando necessário.

Navegação intuitiva.

O teclado funciona em caixas de diálogo e menus (Esc, setas, Enter). Navegação completa por teclado em todas as telas é um objetivo futuro.

---

# 19. Temas

A aplicação possui dois temas, **escuro** e **claro**, definidos em `roui/core/Theme.cpp`.

Nenhum componente deve usar cor fixa. As cores vêm da paleta do tema atual (`Theme::Current()`) ou de um `ColorRole` (no caso de textos), para que a troca de tema funcione sem reconstruir a interface.

A barra de título da janela também acompanha o tema.

| Token | Escuro | Claro | Uso |
|-------|--------|-------|-----|
| background | `#0A0C0D` | `#F4F7F8` | fundo da aplicação |
| sidebar | `#0D1012` | `#FFFFFF` | barra lateral |
| surface | `#101517` | `#FFFFFF` | cards e listas |
| surfaceInset | `#0C1113` | `#F7F9FA` | áreas internas |
| border | `#1A2327` | `#E3E8EA` | bordas |
| outline | `#2A3A3E` | `#CBD5D9` | contornos de botões |
| divider | `#182024` | `#EDF1F2` | divisores |
| hover | `#12181A` | `#F1F4F5` | item sob o mouse |
| textPrimary | `#E6EAEC` | `#14181A` | títulos e valores |
| textSecondary | `#9AA6AB` | `#5B676C` | descrições |
| textMuted | `#7D8A8F` | `#66737A` | informações discretas |
| accent (preenchimentos) | `#AC00FF` | `#AC00FF` | botões, anel, barras |
| accentText (textos) | `#C766FF` | `#AC00FF` | textos e números em destaque |
| accentSoft | `#1F1030` | `#F1E3FF` | fundo suave do destaque |
| onAccent | `#FFFFFF` | `#FFFFFF` | texto sobre o accent |
| track | `#1B2427` | `#E6ECEE` | trilho de barras e anel |
| success (nível Seguro) | `#4ADE80` | `#15803D` | texto de sucesso |
| successSoft | `#10281A` | `#DCFCE7` | fundo do selo Seguro |
| warning (nível Atenção) | `#FBBF24` | `#A16207` | texto de aviso |
| warningSoft | `#2C220A` | `#FEF3C7` | fundo do selo Atenção |

---

# 20. Inventário de componentes (ROUI)

| Componente | Função |
|------------|--------|
| `Window` | janela, loop de mensagens, teclado |
| `Renderer` | desenho com Direct2D e DirectWrite |
| `UIElement` | base de todo elemento (posição, visibilidade, mouse) |
| `Panel` | contêiner; um filho pode ser modal |
| `SurfacePanel` | superfície com cantos arredondados |
| `Text` | texto com cor semântica |
| `Button` | Primary, Secondary, Nav e Segment |
| `Checkbox` | caixa de seleção ou interruptor (Switch) |
| `Card` | métrica com barra de progresso e seta opcional |
| `ProgressRing` | anel de progresso (Health Score) |
| `ScrollPanel` | lista com rolagem |
| `ConfirmDialog` | confirmação modal |
| `DrivePicker` | menu de escolha de disco |
| `ImageBox` | imagem |
| `StackPanel` | layout em pilha |
| `UiDispatcher` | entrega resultados de threads de trabalho para a interface |

---

# 21. Consistência

Todo componente novo deverá seguir este documento.

Nenhum componente poderá utilizar um estilo diferente do restante da aplicação.
