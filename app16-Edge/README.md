# App16 — Plataforma no Raspberry Pi

Use o mesmo firmware preparado no [app15](../app15-Cloud/README.md). A decisão continua nos fluxos da plataforma; o Raspberry Pi próximo dos dispositivos representa o Near Edge/Fog.

## Preparação

1. Conecte Raspberry Pi e ESP32 à mesma rede.
2. Execute Mosquitto, Node-RED e n8n no Raspberry Pi. Se usar Docker, escolha imagens compatíveis com a arquitetura e o sistema do Raspberry. A `IoT-platform` pode servir como referência de configuração.
3. Descubra o IP do Raspberry com `hostname -I`.
4. No firmware, configure a rede Wi-Fi e altere somente `MQTT_SERVER` para esse IP. Exemplo: `"192.168.1.50"`. Recompile e grave quando alterar essas configurações.
5. Acesse `http://IP_DO_RASPBERRY:1880` para importar o dashboard e `http://IP_DO_RASPBERRY:5678` para configurar o n8n.

## Mesmos fluxos do app15

- [Node-RED: dashboard e comandos](../app15-Cloud/Plataformas_config/NodeRED/Fluxo_1_dashboard_graphs_e_cmd.json)
- [Node-RED: InfluxDB opcional](../app15-Cloud/Plataformas_config/NodeRED/Fluxo_2_envio_InfluxDB.json)
- [n8n: notificações](../app15-Cloud/Plataformas_config/n8n/fluxo_mqtt.json)

Configure as conexões no Raspberry: na rede Docker, o broker pode ser `mosquitto`; em uma instalação nativa, `localhost`. O ESP32 usa o IP do Raspberry. Preserve tópicos, payload e regras do app15.

Reconfigure as credenciais do n8n e do InfluxDB no novo ambiente. O token não acompanha o JSON exportado. Se também usar Grafana/InfluxDB localmente, configure a fonte de dados para o banco do Raspberry.

Desative a plataforma de comando anterior para evitar duas instâncias enviando comandos à mesma caixa.

## Experimento

Desconecte o acesso à internet mantendo a rede local. Broker, Node-RED e ESP32 continuam trocando dados e comandos localmente. Telegram e outros serviços externos ficam indisponíveis.

Se o Raspberry, o broker ou o Wi-Fi local parar, o ESP32 mantém o último estado do LED. Não há regra autônoma de alerta no ESP32, armazenamento offline ou reenvio de leituras perdidas.

O que mudou foi a localização da plataforma, não a lógica do firmware. Esta pasta contém somente esta orientação.
