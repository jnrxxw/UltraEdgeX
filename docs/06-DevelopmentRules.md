# Development Rules

| Projeto | UltraEdgeX (UeX) |
|----------|----------------|
| Documento | Development Rules |
| Última atualização | Outubro/2026 |

---

# Código

Uma classe = uma responsabilidade.

---

Nada de arquivos gigantes.

---

Sempre documentar código público.

---

Compilar sem warnings.

---

Commits pequenos.

---

Testar antes do commit.

---

Utilizar RAII sempre que possível.

---

Evitar código duplicado.

---

Sempre utilizar nomes claros.

---

Separar Interface da Lógica.

---

Nenhuma alteração sem revisão.

---

Todo arquivo `.cpp` novo entra no `CMakeLists.txt`.

---

Código sem uso é removido ou registrado em [Ideas](10-Ideas.md); não fica esquecido.

---

# Segurança (regras do produto)

Estas regras vêm da filosofia do UeX e valem para qualquer funcionalidade nova.

1. **A interface nunca altera o Windows.** A página comunica a intenção; o controller e o service fazem o trabalho (DEC-010).

2. **Nada é alterado sem confirmação.** A confirmação lista o que será feito.

3. **Desconhecido nunca é Seguro.** O que o UeX não reconhece vem como Atenção e fora de ações em lote (DEC-013).

4. **Reversível sempre que possível.** Prefira marcar como desativado a apagar, e grave a configuração original antes de alterar.

5. **Administrador só na hora de aplicar.** O app abre sem administrador; o pedido do Windows aparece quando a ação precisa (DEC-015).

6. **Nunca seguir atalhos de pasta** (junctions/links) em limpezas, nunca remover a raiz de um alvo e recusar raízes como `C:\` e `C:\Windows`.

7. **Listas fixas para o que é perigoso.** Limpeza e Serviços usam listas fixas de itens; nada é descoberto e apagado por palpite (DEC-017).

8. **Não prometer o que não dá para medir.** Sem "ganho de X%" nem "impacto" estimado.

---

# Interface

Nenhuma cor fixa: usar `Theme::Current()` ou `ColorRole`.

Operação demorada roda fora da thread da interface e devolve pelo `UiDispatcher`.

Mensagens em linguagem simples; o detalhe técnico vai para o log.

Texto longo deve caber: encurtar com "…" ou deixar quebrar linha, nunca vazar para fora do componente.

Toda tela nova segue o [UI Design System](05-UIDesignSystem.md).

---

# Antes de entregar uma mudança

1. Compilar sem warnings.

2. Testar no Windows (claro e escuro; com e sem administrador, quando a ação pedir).

3. Atualizar o [Changelog](08-Changelog.md) e, se houve decisão nova, o [Decision Log](09-DecisionLog.md).

4. Marcar o que ficou sem testar.
