# Changelog

Formato: `## versão` e, dentro, `### seção`. Mais recente no fim. Datas não foram registradas nas versões antigas.

Estado de teste: as versões até v0.4.0 foram testadas no Windows. As versões **v0.4.1** e **v0.4.2** ainda aguardam teste.

---

## v0.0.1

### Plataforma

- Estrutura inicial do projeto criada.
- CMake configurado.
- Compilação utilizando MSVC.
- Build automatizado funcionando.
- Primeira versão do executável gerada.

Status: Milestone 1 concluído.

---

## v0.0.2

### ROUI

- Implementada a classe `Window`.
- Criada abstração inicial sobre a Win32 API.
- Adicionado loop de mensagens.
- Implementado tratamento de criação, pintura e encerramento.
- Primeira janela do projeto executada com sucesso.

Status: Milestone 2 — Primeira janela do ROUI concluído.

---

## v0.0.3

### ROUI

- Implementado o Renderer com Direct2D.
- Integrado DirectWrite para renderização de texto.
- Adicionado fundo escuro da aplicação.
- Implementado tratamento de redimensionamento da janela.
- Adicionado tratamento para recriação do Render Target.
- Recursos gráficos gerenciados com `Microsoft::WRL::ComPtr`.

Status: Milestone 3 — Renderer com Direct2D e DirectWrite concluído.

---

## v0.1.0

### Documentação

- Primeira documentação criada.
- Product Vision.
- Definição da identidade visual.
- Definição da filosofia do software.
- Planejamento inicial.

---

## v0.1.1

### Interface

- Novo design do dashboard: barra lateral, cartões com barra de progresso, anel de saúde do sistema e lista de aplicativos de inicialização.
- Nome abreviado para UeX na interface.
- Cor primária alterada para roxo `#AC00FF`.
- Adicionados os temas escuro e claro, com alternância na tela e preferência salva entre execuções.

### ROUI

- Novo sistema de tema (`Theme`, `Palette`) com cores semânticas resolvidas em tempo de renderização.
- Novos controles: `SurfacePanel` e `ProgressRing`.
- `Button` ganhou estilos (Primary, Secondary, Nav, Segment); `Card` ganhou barra de progresso; `Text` ganhou `ColorRole`.
- `Renderer` ganhou `DrawCircle` e `DrawArc`.
- Janela com tamanho mínimo e layout recalculado no redimensionamento.

---

## v0.1.2

### Interface

- A barra de título da janela agora acompanha o tema: escura no tema escuro e clara no tema claro, em vez da cor de destaque do Windows.

---

## v0.2.0

### Cleaner

- Nova página **Limpeza**, acessível pela barra lateral. A análise roda sozinha na primeira vez que a página abre e só mede, não apaga nada.
- 9 categorias com tamanho, explicação, nível de segurança (Seguro/Atenção) e caixa de seleção: temporários do usuário e do Windows, cache do Windows Update, cache de entrega de atualizações, relatórios de erro, dumps de memória, miniaturas, shaders e Prefetch.
- Categorias de nível Atenção (Windows Update, shaders e Prefetch) vêm desmarcadas.
- Confirmação antes de executar, mostrando o que será removido e o total.
- Resultado da limpeza com o que foi liberado e o que ficou (em uso ou sem permissão). Os tamanhos são atualizados automaticamente depois.
- Limpeza de pastas do sistema pede permissão de administrador somente na hora de limpar, por um processo auxiliar. Se o usuário recusar, o resto da limpeza continua normalmente.
- Segurança: a lista de categorias é fixa, nunca segue atalhos de pasta (junctions/links), nunca remove a pasta raiz do alvo e recusa raízes como `C:\` e `C:\Windows`.

### Base

- Análise e limpeza rodam em thread separada; a janela não trava.
- Novo `Logger` em `%LOCALAPPDATA%\UltraEdgeX\logs\ultraedgex.log`, com rotação em 1 MB.
- Novo `UiDispatcher` para threads de trabalho atualizarem a interface com segurança.
- Novo `CleanerController`: a tela não chama mais o Cleaner diretamente (DEC-010).
- Novo `MainShell`: barra lateral, seletor de tema e troca entre Painel e Limpeza.

### Interface

- O Painel não executa mais a limpeza. Quando a análise acha arquivos temporários, o botão agora é "Ver em Limpeza".
- Novo controle `Checkbox` e cores de sucesso/aviso nos dois temas.

### Correções

- O anel de saúde agora desenha corretamente scores abaixo de 100.

---

## v0.2.1

### Painel

- A análise de "Arquivos temporários" agora usa o Cleaner novo. Soma as mesmas categorias que a página Limpeza deixa marcadas, então os números dos dois lugares batem.
- A análise do Painel roda em segundo plano; o botão mostra "Analisando…" e a janela não trava.
- Ao voltar para o Painel depois de uma análise, ela é refeita sozinha (por exemplo, depois de uma limpeza).
- O aviso de arquivos temporários agora é identificado por uma ação (`IssueAction::OpenCleaner`) em vez de pelo título.

### Base

- Removidos `TemporaryFilesScanner` e `TemporaryFilesCleaner`, substituídos pelo `Cleaner`.
- A leitura de uso da CPU agora é protegida para ser chamada de duas threads.

---

## v0.2.2

### Cleaner

- Quando o Windows nega até a leitura de uma pasta (sem administrador), a categoria agora mostra tamanho desconhecido ("—" e "Requer administrador") em vez de "Vazio".
- O Painel e a Limpeza usam a mesma formatação de tamanho (ex.: "4,89 GB"), então os números aparecem iguais nos dois lugares.

---

## v0.2.3

### Limpeza

- Quando a medição é parcial (sem administrador), a mensagem de análise diz "pelo menos X" e explica que algumas pastas exigem administrador para medir tudo. O aviso do Painel também diz "Pelo menos X".
- Novo botão **Reabrir como admin** na página Limpeza (aparece só quando o app não está como administrador). O Windows mostra o pedido de permissão; se aceito, o app abre de novo já na Limpeza, com todas as pastas medidas, e a cópia anterior é fechada.
- "Requer administrador" só aparece em linhas que têm arquivos ou tamanho desconhecido; uma pasta realmente vazia mostra só "Vazio".
- Pastas que o Windows nega ler contam como medição parcial também na análise (`GetPathState`).

---

## v0.2.4

### Interface

- A confirmação da limpeza agora é uma caixa própria do app, no lugar da caixa padrão do Windows. Ela escurece a janela, mostra um cartão com as categorias e tamanhos, o total, avisos (nível Atenção, administrador, tamanho desconhecido) e os botões Cancelar e Limpar agora, nos dois temas.
- Enquanto a caixa está aberta, o resto da tela fica bloqueado. Esc cancela. O Enter não confirma de propósito, porque a ação não pode ser desfeita.

### ROUI

- Novo controle `ConfirmDialog` (reutilizável para outras confirmações).
- `Panel` ganhou `SetModalChild`: quando o filho modal está visível, só ele recebe o mouse.
- A janela passou a tratar o teclado (`WM_KEYDOWN`), encaminhado ao shell.

---

## v0.3.0

### Serviços

- Nova página **Serviços**, no lugar de "Desempenho" na barra lateral.
- Lista fixa de 15 serviços do Windows com interruptor, descrição do que faz e do impacto, nível de segurança (Seguro/Atenção) e estado atual (Rodando/Parado e tipo de inicialização). Serviços essenciais (Windows Update, Defender, rede, áudio etc.) não fazem parte da lista.
- Seguros (8): telemetria, mensagens push do WAP, relatório de erros, fax, demonstração de varejo, mapas baixados, Windows Insider e controle dos pais.
- Atenção (7): SysMain, política de diagnóstico, Windows Search, spooler de impressão, otimização de entrega, Bluetooth e geolocalização.
- As mudanças ficam pendentes até o usuário clicar em **Aplicar alterações** e confirmar. O botão **Marcar recomendados** marca só os de nível Seguro.
- Antes de alterar um serviço, o UeX grava a configuração original; **Restaurar padrão** volta tudo ao que era. Um serviço que outros serviços ativos usam não é alterado.
- Alterar serviços pede permissão de administrador só na hora de aplicar, pelo mesmo mecanismo da Limpeza.

### ROUI

- Novo `ScrollPanel` (lista com rolagem pela roda do mouse e barra de rolagem), `Renderer::PushClip/PopClip` e `OnMouseWheel`.
- `Checkbox` ganhou o estilo `Switch` (interruptor).
- Botão secundário desativado agora tem aparência de desativado.

### Base

- Elevação compartilhada em `ElevatedProcess` (Limpeza e Serviços usam o mesmo código e a mesma validação do arquivo de resultado).
- `SafetyLevel` compartilhado entre Limpeza e Serviços.
- Caixa de confirmação aceita até 16 itens.


---

## v0.3.1

### Painel

- "Analisar sistema" agora também detecta serviços desnecessários. O alerta mostra só a quantidade (ex.: "4 serviços desnecessários") e não promete ganho de desempenho.
- Só contam serviços de nível Seguro que existem neste Windows, não estão desativados e estão rodando ou com início automático. Serviço manual e parado não conta.
- Cada alerta tem o próprio botão ao lado do bloco: **Ver em Limpeza** nos arquivos temporários e **Ver em Serviços** nos serviços (`IssueAction::OpenServices`).
- Os alertas ficam numa área com rolagem: aparecem 3 por vez e o usuário rola com a roda do mouse para ver os outros. Todos os alertas são mostrados, ordenados por gravidade (os mais graves primeiro).
- O alerta de serviços reduz 3 pontos do Health Score.

### Serviços

- O critério do que é "desnecessário" ficou em um só lugar (`Services::IsRecommendedToDisable`), usado pelo Painel e pelo botão **Marcar recomendados**. Os números sempre batem.
- **Marcar recomendados** deixou de marcar serviço Seguro manual e parado, porque desativar não muda nada na prática.

---

## v0.4.0

Status de teste: testado no Windows.

### Inicialização

- Nova página **Inicialização**, no lugar do atalho que abria as configurações do Windows. O Painel também passou a abrir essa página.
- Lista os aplicativos que abrem com o Windows (chaves `Run` do usuário atual, de todos os usuários e de apps de 32 bits) e os apps da Microsoft Store (Terminal, Xbox, Teams etc.), com interruptor, descrição, nível de segurança (Seguro/Atenção), estado (Ativo/Desativado) e origem.
- Aplicativos que o UeX não reconhece vêm sempre como **Atenção** (nunca Seguro) e mostram o nome do programa para o usuário conferir.
- As mudanças ficam pendentes até **Aplicar alterações** e confirmar. **Marcar recomendados** marca só apps conhecidos de nível Seguro que estão ativos.
- Nada é apagado: o UeX só muda a marca de habilitado que o Windows usa (`StartupApproved`), a mesma do Gerenciador de Tarefas. Para voltar atrás, basta reativar o interruptor e aplicar de novo.
- Apps do usuário atual não pedem administrador. Apps de todos os usuários pedem permissão de administrador só na hora de aplicar, pelo mesmo mecanismo da Limpeza e de Serviços.
- O bloco de inicialização do Painel agora atualiza sozinho quando a página volta a ser exibida.

### Base

- `StartupManager` agora também altera e restaura entradas (`Execute`), lê a chave de 32 bits e trata o estado de `StartupApproved` por paridade do primeiro byte (par = habilitado, ímpar = desabilitado).
- Novo `StartupCatalog`: lista interna de apps conhecidos e critério único de "recomendado para desativar" (`IsRecommendedToDisable`).
- Novo `ElevatedStartup`: ajudante elevado que só aceita entradas de HKLM e só altera nomes que existem agora na chave `Run`. Os nomes viajam em hexadecimal para aguentar espaços e acentos.
- Novos `StartupController` e `StartupPage` (a lista é dinâmica, então as linhas são recriadas a cada leitura).
- Apps da Loja: só entram pacotes instalados para o usuário (restos de pacotes desinstalados são ignorados). Um app da Loja que ainda não ligou a própria inicialização (como o Terminal) também pode ser ligado pelo interruptor. Estados controlados por política ficam travados, com o motivo na linha.
- Nas linhas de apps da Loja, o texto à direita mostra o nome do pacote ("Loja · …").
- Catálogo: a regra do Teams vem antes da do Steam ("MSTeams" contém "steam").
- Fora desta versão: pasta Inicializar do menu Iniciar e tarefas agendadas.

---

## v0.4.1

Status de teste: aguardando teste no Windows.

### Painel

- **Informações do sistema** agora em duas colunas, com 10 itens: Sistema operacional (edição e versão), Computador, Processador, Placa de vídeo (com VRAM), Memória, Placa-mãe, BIOS, Monitor (resolução e taxa de atualização), Tempo ligado e Plano de energia. O item "Arquitetura" saiu, porque repetia o sistema operacional.
- Textos longos são encurtados com "…" conforme a largura da coluna. Tempo ligado, plano de energia e monitor são relidos toda vez que o Painel é exibido.
- O card de Saúde ficou um pouco mais estreito para dar espaço às duas colunas.
- O bloco de inicialização do Painel passou a usar os nomes amigáveis (por exemplo "TranslucentTB" no lugar do identificador do pacote da Loja).

### Base

- `SystemInfo::GetSystemDetails` lê placa de vídeo (DXGI, ignora o adaptador de software e escolhe a de maior memória dedicada), placa-mãe e BIOS (registro, sem WMI, com filtro de textos "To be filled by O.E.M."), versão do Windows (build 22000 ou maior = Windows 11), plano de energia e monitor principal.
- Dados que não mudam (hardware, Windows) são lidos uma vez; os demais, a cada chamada.
- Novas bibliotecas no CMake: `dxgi` e `powrprof`.

---

## v0.4.2

Status de teste: aguardando teste no Windows.

### Painel

- O card de disco ganhou uma **seta** no canto superior direito. Ao clicar, abre um menu pequeno com todos os HDs e SSDs: letra, nome do volume, tipo (SSD/HD), espaço livre e uma barra de uso. O disco do Windows vem marcado como "Sistema" e o escolhido com um check.
- Ao escolher, o card passa a mostrar o disco selecionado (título "DISCO D: · NOME", tamanho e espaço livre) e a atualização em tempo real acompanha esse disco. Escolher o disco do sistema volta ao padrão.
- A seta só aparece quando há mais de um disco. Se o disco escolhido for removido, o card volta sozinho para o disco do sistema.
- O menu fecha ao escolher, ao clicar fora, com Esc ou ao trocar de página. Setas do teclado e Enter também funcionam.
- A análise de saúde e a limpeza continuam usando o disco do sistema; a seleção só muda o card.
- A escolha vale até fechar o app (salvar entre sessões fica para a tela de Configurações).

### Base

- `SystemInfo::GetFixedDrives` (só discos fixos: ignora pendrive, cartão, CD/DVD e rede; tipo SSD/HD pelo driver do disco, sem administrador) e `SystemInfo::GetDiskInfo(root)`.
- Novo controle `DrivePicker` (menu ancorado, filho modal do painel) e seta de seleção no `Card`.

---

## Revisão da documentação (Outubro/2026)

Sem mudança no programa.

### Documentação

- Documentos atualizados para refletir o estado real do projeto: nome UltraEdgeX (UeX), módulos de Serviços e Inicialização, estrutura de pastas, padrão de elevação, inventário de componentes e tabela completa de cores.
- Decision Log reorganizado: DEC-011 passou a ser a cor roxa (citada no Brand Guide) e a decisão sobre o nível de segurança da Inicialização virou DEC-013. Novas decisões DEC-012 a DEC-018.
- V1 Specification passou a mostrar Feito / Pendente em vez de marcar tudo como pronto. A Lixeira saiu da V1 (DEC-018).
- Roadmap atualizado e ideias adiadas registradas em Ideas.
