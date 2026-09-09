# Memória — NexoLog

## Decisão vigente — 09/09/2026

Empresa fictícia de entregas de cargas sensíveis. Simples, didático, direto; manter os módulos ESP32Sensors e o estilo de código do professor.

- App14: único projeto-base embarcado, em `app14_CPS_e_Automation_v2/app14_NexoLog`. Publica leituras; LED reservado e apagado.
- App15: README ensina a acrescentar somente recepção MQTT ao app14. Inclui tópico de comandos, protótipo, callback, setCallback e subscribe após conexão. Mantém os fluxos de Node-RED e n8n.
- App16: somente README para executar a plataforma no Raspberry Pi. Reutiliza firmware e fluxos do app15, alterando configurações de rede. Sem decisão de alerta no ESP32.

Essa decisão substitui a proposta anterior de alerta autônomo no app16. Near Edge/Fog é a plataforma no Raspberry, próxima da caixa.

## Hardware e biblioteca

DHT22 no GPIO26; TRIG17/ECHO16; MPU SDA18/SCL19; LED27. FastIMU 1.3.0 substitui Adafruit_MPU6050. `MPU_TYPE` seleciona MPU6050 (Wokwi) ou MPU6500 (placa física). Aceleração convertida de g para m/s², inclinação em graus. Sem calibração automática.

A callback segue o app08: `String mensagem(conteudo, tamanho);`, comparações com ON e OFF e atuação no LED. Nenhuma alteração de telemetria entre app14 e app15.

## Contrato e regras

Tópicos comuns: `FIAPIoT/nexolog/equipe01/dados`, `/eventos` e `/cmd`. Client ID `NexoLogEquipe01`. Apenas uma plataforma de comando ativa por equipe.

Payload: device, temp, umid, dist, accel_x, accel_y, accel_z, inclinacao. Não transmite alerta nem confirmação do LED. Limites no Node-RED: temperatura >30 °C, umidade >70%, distância >25 cm, inclinação >45° ou leitura inválida. São valores didáticos.

O n8n recebe eventos de mudança do Node-RED e prepara notificações. O InfluxDB armazena no measurement `nexolog`, tag `device`. Grafana pode consultar o mesmo histórico; nenhum painel Grafana foi criado.

## Arquivos e preservação

Originais de FIAP-IoT-eval e PPTX preservados. SugestoesSlides.md mantém cinco intervenções por deck, atualizadas para FastIMU e Raspberry.

Revisão automática bloqueou a tentativa de exclusão sem detalhar o motivo. As cópias antigas foram movidas de forma recuperável para `C:/Users/noris/AppData/Local/Temp/NexoLog-arquivo-20260909-164302`. Snapshot anterior à reorganização: `C:/Users/noris/AppData/Local/Temp/nexolog-antes-reorganizacao.zip`.

## Limites

Sem decisão local de alerta, fila offline ou reenvio. Se a rede local cair, o LED mantém o último comando. Serviços externos como Telegram dependem da internet, mesmo com plataforma no Raspberry.

## Validação da reorganização

- App14 compilado para MPU6050 e MPU6500 com FastIMU 1.3.0.
- Código app15 montado em pasta temporária extraindo os cinco blocos C++ do README entregue. Compilado para MPU6050 e MPU6500.
- Diff conferido: somente tópico de comandos, protótipo, setCallback, subscribe e callback com String. Coleta, payload e publicação iguais.
- Fluxos JSON, referências entre nós, tópicos comuns, comandos ON/OFF, condições de alerta, supressão de eventos repetidos e links Markdown verificados.
- App15 contém apenas README e Plataformas_config. App16 contém apenas README.
- Não realizado teste em placa/Wokwi ou envio de Telegram nesta reorganização. Não houve deploy na plataforma existente.

## Publicação no repositório de avaliação

Após o usuário remover os apps antigos, as versões de desenvolvimento foram copiadas para `FIAP-IoT-eval/app14_CPS_e_Automation`, `app15-Cloud` e `app16-Edge`, sem sufixo `_v2` e sem caches de compilação. Links ajustados; guia comum em `app14_CPS_e_Automation/Guia-NexoLog.md`. Código e configurações conferidos byte a byte contra a origem. Keepalive atual: 120 segundos, sem overrides dos timeouts.
