# Product Vision

| Projeto | UltraEdgeX (UeX) |
|----------|----------------|
| Documento | Product Vision |
| Versão | 1.1 |
| Status | Em desenvolvimento |
| Autor | Junior |
| Última atualização | Outubro/2026 |

> O nome do produto é **UltraEdgeX**, abreviado como **UeX** na interface (DEC-012). "Real Optimizer" foi o nome provisório inicial e permanece apenas em nomes técnicos do código (namespace `real_optimizer` e a camada ROUI).

---

# 1. Visão

O UltraEdgeX é um software moderno de otimização e monitoramento para Windows.

O objetivo do projeto não é apenas remover arquivos temporários ou alterar configurações do sistema, mas oferecer ao usuário uma visão clara da saúde do computador, explicar cada recomendação e permitir que todas as otimizações sejam executadas com segurança e transparência.

---

# 2. Missão

Criar o melhor software de otimização para Windows baseado em quatro pilares:

• Transparência

• Segurança

• Desempenho

• Simplicidade

---

# 3. Público-alvo

## Usuário comum

Deseja melhorar o desempenho do computador sem precisar entender detalhes técnicos.

---

# 4. Filosofia

Antes de qualquer alteração o software deve:

Analisar

↓

Explicar

↓

Recomendar

↓

Executar

↓

Mostrar os resultados

O usuário nunca deve sentir que perdeu o controle do computador.

---

# 5. Valores

## Transparência

Toda otimização deverá explicar:

- o que será feito;
- por que será feito;
- quais riscos existem.

---

## Segurança

Nenhuma alteração potencialmente perigosa será executada sem confirmação.

Sempre que possível o software deverá permitir desfazer alterações importantes.

---

## Honestidade

Nunca prometer ganhos irreais de desempenho.

As recomendações serão baseadas nas características reais do sistema.

Na prática: o UeX não mostra "impacto" ou "ganho estimado" que não consiga medir de forma confiável, e o que ele não reconhece nunca é tratado como seguro.

---

## Simplicidade

Mesmo usuários iniciantes devem conseguir utilizar o programa sem necessidade de manuais.

---

## Performance

O próprio UeX deve consumir poucos recursos do computador.

---

# 6. Diferenciais

## Health Score

Pontuação que representa a saúde geral do sistema.

*Estado atual:* implementado no Painel como uma pontuação única, reduzida pelos alertas encontrados na análise.

---

## Plano de Otimização

Antes de executar qualquer alteração o software monta um plano completo das ações recomendadas.

*Estado atual:* parcial. A análise do Painel gera alertas ordenados por gravidade, cada um com atalho para a página certa (Limpeza ou Serviços), e toda alteração passa por uma confirmação que lista o que será feito.

---

## Confidence Score

Cada recomendação possui um nível de confiança indicando o quanto aquela otimização é indicada para aquele computador.

*Estado atual:* ainda não implementado.

---

## Níveis de Segurança

🟢 Seguro

Pode ser executado sem riscos conhecidos.

🟡 Atenção

Pode alterar o comportamento do sistema.

🔴 Avançado

Recomendado apenas para usuários experientes.

*Estado atual:* a interface usa **Seguro** e **Atenção**. O nível **Avançado** está previsto, mas nenhuma função da V1 precisa dele.

---

## Explicações detalhadas

Cada serviço, processo ou configuração apresenta uma explicação simples sobre sua função e o impacto esperado da alteração.

---

## Interface moderna

Interface minimalista inspirada em aplicações modernas do Windows.

---

# 7. Princípios de UX

O usuário deve entender o estado do computador nos primeiros cinco segundos após abrir o software.

As ações mais comuns devem exigir o menor número possível de cliques.

As recomendações devem ser claras, objetivas e compreensíveis.

Nenhuma informação importante deve ficar escondida.

---

# 8. Objetivos da versão 1.0

Painel (Dashboard)

Health Score

Limpeza (Cleaner)

Serviços

Inicialização (Startup Manager)

Informações de hardware

Configurações

Logs

Modo escuro e modo claro

O andamento de cada item está no [Roadmap](07-Roadmap.md) e na [V1 Specification](V1_Specification.md).

---

# 9. O que NÃO faz parte da versão 1.0

Driver Updater

Benchmark

Plugins

Game Booster

IA

Sincronização em nuvem

---

# 10. Nosso lema

"O UeX não toma decisões pelo usuário.

Ele analisa, explica, recomenda e executa apenas aquilo que o usuário entende e aprova."

---

# 11. Product Principles (Princípios do Produto)

Esses princípios servem como regras para qualquer funcionalidade nova. Antes de implementá-la, verificamos se ela respeita estes pontos:

1. **Toda funcionalidade deve resolver um problema real.** Não adicionaremos recursos apenas para aumentar a lista de funcionalidades.
2. **Explicar antes de executar.** O usuário deve entender o impacto de qualquer alteração importante.
3. **Segurança acima de desempenho.** Nunca sacrificaremos a estabilidade do Windows por um ganho mínimo de performance.
4. **Desempenho do próprio software importa.** O UeX deve iniciar rapidamente e consumir poucos recursos.
5. **Consistência visual.** Toda nova tela deve seguir o mesmo Design System, sem exceções.
