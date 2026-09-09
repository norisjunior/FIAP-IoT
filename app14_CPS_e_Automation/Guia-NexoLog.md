# NexoLog — monitoramento de entregas

Uma empresa fictícia acompanha cargas sensíveis. A mesma caixa e os mesmos dados acompanham as aulas.

| Etapa | Firmware | Plataforma |
|---|---|---|
| [App14](README.md) | Projeto-base: coleta e publicação MQTT | Fluxos, Debug, gauges, gráficos e eventos |
| [App15](../app15-Cloud/README.md) | Acrescentar somente callback e assinatura MQTT ao app14 | Regras enviam ON/OFF ao LED |
| [App16](../app16-Edge/README.md) | Mesmo firmware preparado no app15 | Mesmos fluxos executados no Raspberry Pi |

O projeto embarcado está em `app14_CPS_e_Automation/app14_NexoLog`. O app15 contém o roteiro das alterações e as configurações da plataforma. O app16 contém somente a orientação de implantação no Raspberry.

## Montagem

| Componente | GPIO | Função |
|---|---|---|
| DHT22 | 26 | Temperatura e umidade do ar |
| HC-SR04 TRIG / ECHO | 17 / 16 | Distância até a tampa |
| MPU6050 ou MPU6500 SDA / SCL | 18 / 19 | Inclinação da caixa |
| LED vermelho | 27 | Reservado no app14; comandos ON/OFF a partir do app15 |

Fixe o ultrassônico dentro da caixa, apontado para a tampa, a aproximadamente 10 cm quando fechada. Ao levantar a tampa, a distância aumenta. Sem eco, a leitura é inválida; isso não confirma abertura.

O diagrama inclui resistor de 220 Ω no LED e divisor no ECHO: ECHO ligado a 1 kΩ, GPIO16 na junção, 2 kΩ da junção ao GND. HC-SR04 em 5 V; DHT22 e MPU em 3,3 V. GND comum. No DHT sem módulo, acrescente pull-up de 10 kΩ entre DATA e 3,3 V.

## FastIMU

Usamos FastIMU 1.3.0, como nos projetos de IMU da disciplina. Em `ESP32SensorsAccel.hpp`, selecione `MPU_TYPE`: `MPU6050` para o diagrama Wokwi ou `MPU6500` para essa placa física. O endereço I2C é `0x68`, com AD0 em GND.

A biblioteca retorna aceleração em g; o módulo converte para m/s² antes da publicação. Fixe o MPU com Z para cima e faça movimentos lentos. A inclinação é uma estimativa pela gravidade, em graus, não um detector de impactos. Não há calibração automática nesta aula.

## Dados e comandos

Os tópicos são iguais nas três etapas:

- `FIAPIoT/nexolog/equipe01/dados`: leituras JSON, a cada 2,5 segundos.
- `FIAPIoT/nexolog/equipe01/eventos`: mudanças de estado publicadas pelo Node-RED.
- `FIAPIoT/nexolog/equipe01/cmd`: comandos ON/OFF, recebidos a partir do app15.

O dispositivo usa `NexoLogEquipe01`. Troque a identificação e `equipe01` no firmware e nos fluxos para sua equipe. Não execute dois ESP32 com o mesmo Client ID.

```json
{"device":"NexoLogEquipe01","temp":24,"umid":55,"dist":10,"accel_x":0,"accel_y":0,"accel_z":9.81,"inclinacao":0}
```

Falhas de leitura podem aparecer como `null`. O payload não muda no app15 e não contém confirmação do LED. Não usamos índice de calor para avaliar a carga.

## Regras na plataforma

| Leitura | Alerta didático |
|---|---|
| Temperatura | > 30 °C |
| Umidade | > 70% |
| Distância | > 25 cm |
| Inclinação | > 45° |
| Leitura inválida | Falha de sensor |

São limites de aula, não especificações de conservação de uma carga real. O LED apenas sinaliza o alerta. No app14, a plataforma apresenta a condição; no app15 e app16, também envia o comando.

## Plataforma

Inicie a `IoT-platform` do repositório de avaliação. Importe somente os arquivos indicados no README da etapa. Os dashboards usam `node-red-dashboard` (`ui_gauge`, `ui_chart`, `ui_text`), como no material original. O histórico usa `node-red-contrib-influxdb`.

| Origem da conexão | Broker MQTT |
|---|---|
| ESP32 no Wokwi com gateway | `host.wokwi.internal:1883` |
| ESP32 físico | IP do computador ou Raspberry Pi:1883 |
| Node-RED/n8n na rede Docker da plataforma | `mosquitto:1883` |
| Node-RED/n8n instalados diretamente na máquina do broker | `localhost:1883` |

Editor: `http://localhost:1880`. Dashboard: `http://localhost:1880/ui/`. No Raspberry, substitua `localhost` pelo IP dele para acessar pelo computador.

Como as etapas compartilham tópicos, desative o dashboard anterior ao ativar o seguinte. Mantenha um fluxo de gravação e um workflow n8n ativos por equipe para evitar duplicação de dados e notificações.

## InfluxDB, n8n e Grafana

Depois do dashboard, importe `Fluxo_2_envio_InfluxDB.json`. Configure URL, organização, bucket e token. Os valores iniciais `fiapiot` e `sensores` seguem o `.env.exemplo` da plataforma. O measurement é `nexolog` e a tag é `device`. O horário registrado é o de recebimento no banco.

No n8n, importe `fluxo_mqtt.json`, configure MQTT, Telegram e `SEU_CHAT_ID`, e ative o workflow. O Node-RED publica eventos na entrada em alerta, mudança do motivo e recuperação. A primeira leitura normal não gera aviso. Reiniciar o Node-RED reinicia essa memória de estado.

Grafana pode consultar o mesmo banco e measurement, sem modificar o firmware. Não há dashboard Grafana exportado nesta versão.

Os nós `Simular` alimentam o dashboard diretamente. No app15 também enviam comandos reais ao LED, mas não alimentam o fluxo separado de InfluxDB.

## Limites

O ESP32 não decide sobre alertas. Sem comunicação, mantém o último estado do LED. Não há armazenamento offline nem reenvio de leituras perdidas. No app16, perder a internet mantendo a rede local permite continuar os comandos locais; notificações externas dependem da internet.
