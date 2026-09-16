# NexoLog — monitoramento de entregas

Uma empresa fictícia acompanha cargas sensíveis. A mesma caixa e os mesmos dados acompanham as aulas.

| Etapa | Firmware | Plataforma |
|---|---|---|
| [App14](README.md) | Projeto-base: coleta e publicação MQTT | Node-RED mostra; n8n decide e avisa |
| [App15](../app15-Cloud/README.md) | Acrescentar somente callback e assinatura MQTT ao app14 | Regras enviam ON/OFF ao LED |
| [App16](../app16-Edge/README.md) | Mesmo firmware preparado no app15 | Mesmos fluxos executados no Raspberry Pi |

O projeto embarcado está em `app14_CPS_e_Automation/app14_NexoLog_PUB_only`. Para montar do zero, em duas iterações que rodam: [firmware](app14_NexoLog_PUB_only/CONSTRUIR-O-FIRMWARE.md), [Node-RED](Plataformas_config/NodeRED/CONSTRUIR-O-FLUXO.md) e [n8n](Plataformas_config/n8n/CONSTRUIR-O-FLUXO.md). O app15 contém o roteiro das alterações e as configurações da plataforma. O app16 contém somente a orientação de implantação no Raspberry.

## Montagem

| Componente | GPIO | Função |
|---|---|---|
| DHT22 | 4 | Temperatura e umidade do ar |
| HC-SR04 TRIG / ECHO | 19 / 18 | Distância até a tampa |
| MPU6050 ou MPU6500 SDA / SCL | 22 / 23 | Movimentação da caixa |
| LED vermelho | 21 | Reservado no app14; comandos ON/OFF a partir do app15 |

Fixe o ultrassônico dentro da caixa, apontado para a tampa, a aproximadamente 10 cm quando fechada. Ao levantar a tampa, a distância aumenta. Sem eco, a leitura é inválida; isso não confirma abertura.

Os pinos ficam todos do lado direito da placa, para facilitar a montagem física. O diagrama inclui resistor de 220 Ω no LED. No Wokwi o HC-SR04 está em 3,3 V, com ECHO direto no GPIO18. Na montagem física com HC-SR04 alimentado em 5 V, use divisor no ECHO: ECHO ligado a 1 kΩ, GPIO18 na junção, 2 kΩ da junção ao GND. DHT22 e MPU em 3,3 V. GND comum. No DHT sem módulo, acrescente pull-up de 10 kΩ entre DATA e 3,3 V.

## FastIMU

Usamos FastIMU 1.3.0, como nos projetos de IMU da disciplina. Em `ESP32SensorsAccel.hpp`, selecione `MPU_TYPE`: `MPU6050` para o diagrama Wokwi ou `MPU6500` para essa placa física. O endereço I2C é `0x68`, com AD0 em GND.

A biblioteca retorna aceleração em g; o módulo converte para m/s² antes da publicação. A movimentação é a magnitude da aceleração descontando a gravidade, em m/s²: caixa parada fica próxima de 0 e sobe conforme o chacoalho. O firmware amostra o MPU a cada 50 ms e publica o **maior** valor do último segundo — uma leitura por envio deixaria o pico do sacolejo passar despercebido. Não há calibração automática nesta aula.

## Dados e comandos

Os tópicos são iguais nas três etapas:

- `FIAPIoT/nexolog/equipe01/dados`: leituras JSON, a cada 1 segundo. Node-RED e n8n assinam **os dois** este tópico.
- `FIAPIoT/nexolog/equipe01/cmd`: comandos em JSON para os LEDs, recebidos a partir do app15.

O dispositivo usa `NexoLogEquipe01`. Troque a identificação e `equipe01` no firmware e nos fluxos para sua equipe. Não execute dois ESP32 com o mesmo Client ID.

```json
{"device":"NexoLogEquipe01","temp":24,"umid":55,"dist":10,"accel_x":0,"accel_y":0,"accel_z":9.81,"movimentacao":0}
```

Falhas de leitura podem aparecer como `null`. O payload não muda no app15 e não contém confirmação do LED. Não usamos índice de calor para avaliar a carga.

## Regras na plataforma

| Leitura | Alerta didático |
|---|---|
| Temperatura | > 30 °C |
| Umidade | > 70% |
| Distância | > 25 cm |
| Movimentação | > 3 m/s² |
| Leitura inválida | Falha de sensor |

São limites de aula, não especificações de conservação de uma carga real. O LED apenas sinaliza o alerta. No app14, a plataforma apresenta a condição; no app15 e app16, também envia o comando.

## Plataforma

Entre no diretório `IoT-platform` que você recebeu e suba a plataforma:

```bash
docker compose up -d
```

Sobem juntos Mosquitto, Node-RED, n8n, InfluxDB e Grafana. Para parar, `docker compose down` na mesma pasta — sem opção de remover volumes, senão você perde fluxos e dashboards. Importe somente os arquivos indicados no README da etapa. Os dashboards usam `node-red-dashboard` (`ui_gauge`, `ui_chart`, `ui_text`), como no material original. O histórico usa `node-red-contrib-influxdb`.

| Origem da conexão | Broker MQTT |
|---|---|
| ESP32 no Wokwi com gateway | `host.wokwi.internal:1883` |
| ESP32 físico | IP do computador ou Raspberry Pi:1883 |
| Node-RED/n8n na rede Docker da plataforma | `mosquitto:1883` |
| Node-RED/n8n instalados diretamente na máquina do broker | `localhost:1883` |

Editor: `http://localhost:1880`. Dashboard: `http://localhost:1880/ui/`. No Raspberry, substitua `localhost` pelo IP dele para acessar pelo computador.

Como as etapas compartilham tópicos, desative o dashboard anterior ao ativar o seguinte. Mantenha um fluxo de gravação e um workflow n8n ativos por equipe para evitar duplicação de dados e notificações.

## InfluxDB, n8n e Grafana

Depois do dashboard, importe `Fluxo_2_envio_InfluxDB.json`. Ele assina o mesmo `dados` e grava as sete medições, sem regra nenhuma: o measurement é `nexolog`, `device` é a tag e o horário registrado é o de recebimento no banco.

Usamos o **InfluxDB Cloud**, não o da plataforma. Cada um preenche quatro campos: URL da sua região e token no nó de configuração, organização e bucket no nó `Gravar leituras`. O token não vem no arquivo importado — o Node-RED guarda token como credencial e a exportação sempre remove.

Dashboard e InfluxDB são dois fluxos independentes assinando o mesmo tópico. Rode os dois ao mesmo tempo: um mostra agora, o outro guarda para depois.

No n8n, importe `fluxo_mqtt.json`, configure MQTT, Telegram e `SEU_CHAT_ID`, e ative o workflow. Ele assina `dados` direto do ESP32: um Switch compara as quatro variáveis com os limiares e cada saída monta a sua mensagem. O aviso sai a cada leitura que passar do limite, não só na mudança — veja a nota sobre a taxa do Telegram no guia do fluxo.

Grafana pode consultar o mesmo banco e measurement, sem modificar o firmware. Não há dashboard Grafana exportado nesta versão.

Os nós `Simular` alimentam o dashboard diretamente. No app15 também enviam comandos reais ao LED, mas não alimentam o fluxo separado de InfluxDB.

## Limites

O ESP32 não decide sobre alertas. Sem comunicação, mantém o último estado do LED. Não há armazenamento offline nem reenvio de leituras perdidas. No app16, perder a internet mantendo a rede local permite continuar os comandos locais; notificações externas dependem da internet.
