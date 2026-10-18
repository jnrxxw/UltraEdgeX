# Ideas

Ideias ainda não desenvolvidas ou adiadas. Toda ideia passa pela revisão de produto da DEC-007 antes de virar tarefa.

Status possíveis: Aprovado · Em análise · Adiado · Para versão futura · Descartado

---

## Produto

### Health Score

Status: Aprovado. Implementado como pontuação única; os cinco pilares seguem previstos.

---

### Confidence Score

Status: Aprovado. Ainda não implementado.

---

### Optimization Plan

Status: Aprovado. Parcial: hoje são alertas com atalho e confirmação em cada página.

---

### Plugin System

Status: Em análise.

---

### AI Assistant

Status: Para versão futura.

---

### Nível "Avançado"

Status: Adiado. Previsto no Product Vision, mas nenhuma função da V1 precisa dele.

---

## Painel

### Alerta de muitos apps de inicialização ativos

Status: Adiado. Mostrar no Painel um alerta quando houver muitos apps de nível Seguro iniciando com o Windows, como já existe para serviços. Deve mostrar só a quantidade, sem prometer ganho.

---

### Lembrar o disco escolhido

Status: Adiado. Depende da tela de Configurações.

---

## Inicialização

### Pasta Inicializar e tarefas agendadas

Status: Adiado. A lista atual mostra o Registro (`Run`) e os apps da Loja. Itens da pasta Inicializar do menu Iniciar e tarefas agendadas ainda não aparecem.

---

### Impacto na inicialização

Status: Em análise. O Gerenciador de Tarefas mostra um "impacto". O UeX só deve mostrar algo parecido se conseguir medir de forma confiável (princípio de honestidade).

---

## Limpeza

### Lixeira

Status: Adiado (DEC-018).

---

## Interface

### Trocar a fonte para Inter

Status: Adiado. Exige embutir a fonte e carregá-la no DirectWrite.

---

### Tooltips e navegação completa por teclado

Status: Adiado.

---

### Cor de erro e de informação no tema

Status: Adiado. Entram junto com a primeira tela que precise de um estado de erro com cor própria.

---

## Páginas

### Página Hardware

Status: Em análise. O Painel já mostra um resumo; a página teria o detalhe (por exemplo, memória, discos e temperaturas).

---

### Temperaturas

Status: Em análise. Prevista no Software Design, ainda sem fonte confiável escolhida.

---

## Qualidade e manutenção

### Código de "restaurar" sem uso na Inicialização

Status: Adiado. O `StartupController` e o `StartupManager` ainda têm essa lógica, e a página não tem mais o botão. Remover ou reaproveitar.

---

### Pasta `tests/`

Status: Adiado. Previsto na arquitetura; criar com testes das partes que não dependem do Windows (por exemplo, o `StartupCatalog` e a formatação de nomes).
