# Ajustes nos slides

Referência: texto dos dois PPTX e inspeção das imagens incorporadas de dashboard, fluxos e notificações. Os PowerPoints permanecem intactos. Números correspondem à ordem dos slides no arquivo, incluindo capas e apêndices.

## Aula 07 — Node-RED

Cinco alterações, sem acrescentar slides.

### 1. Slide 4 — Objetivo

Substituir a lista por:

- Entender o percurso de uma mensagem no Node-RED.
- Receber dados do ESP32 via MQTT e JSON.
- Construir gráficos e gauges para acompanhar uma entrega.
- Integrar DHT22, HC-SR04 e MPU6050/MPU6500 (FastIMU).

### 2. Slide 9 — Arquitetura de um CPS para IoT

Atualizar a camada física para “DHT22, HC-SR04, MPU6050/MPU6500 (FastIMU) e LED”.

Texto de apoio: “O ESP32 mede as condições da caixa. O Node-RED recebe os dados e apresenta o estado da entrega.”

Nota de fala: no app14 o LED fica reservado. No app15 ele recebe ON/OFF por MQTT. No app16 o mesmo firmware recebe comandos da plataforma no Raspberry Pi. Essa distinção prepara a evolução sem antecipar código.

### 3. Slide 21 — Aplicação IoT: NexoLog

Substituir o infográfico do Sentinela por uma caixa de carga com os sensores identificados. Manter o fundo e a tipografia do deck.

Texto:

“A equipe precisa acompanhar uma entrega de carga sensível.”

- DHT22: temperatura e umidade dentro da caixa.
- Ultrassônico: distância até a tampa.
- MPU6050/MPU6500 (FastIMU): inclinação da caixa.

Pergunta: “A caixa chegou nas mesmas condições em que saiu?”

Rodapé: “Limites e incidentes simulados para a aula.”

### 4. Slide 25 — Aplicação: app14

Trocar a referência da aplicação por `app14_CPS_e_Automation_v2/app14_NexoLog`.

Usar este payload para acompanhar o percurso no editor:

```json
{"device":"NexoLogEquipe01","temp":24,"umid":55,"dist":10,"inclinacao":0}
```

É um recorte dos campos usados na aula; o firmware também publica `accel_x`, `accel_y` e `accel_z`.

Roteiro de demonstração: MQTT-in, JSON, Debug, Function e Gauge. Abrir `separarDadosSensores` e observar que cada saída contém uma mensagem com um valor em `payload`.

### 5. Slide 28 — Dashboard da entrega

Substituir a captura com “Índice de Calor / Presença” por uma captura do novo dashboard.

Mostrar temperatura, umidade, distância, inclinação e estado da entrega. Ao lado, apenas a pergunta: “O que muda quando levantamos a tampa?”

Nota de fala: usar `Simular: Normal` e `Simular: Tampa aberta`. Depois demonstrar no Wokwi. O gráfico na tela não substitui um banco de histórico.

Para respeitar o limite de cinco alterações, as capturas antigas dos slides 29 e 32 ficam como exemplos anteriores de notificações. Identificar isso oralmente; suas mensagens de presença não descrevem a NexoLog. No slide 32, aproveitar a repetição das mensagens para discutir por que a versão nova avisa apenas na mudança de estado.

## Aula 08 — Node-RED e n8n

Cinco alterações, sem acrescentar slides.

### 1. Slide 4 — Objetivo

Texto:

“Notificar o responsável quando as condições da entrega mudarem.”

- Receber eventos da NexoLog via MQTT.
- Preparar uma mensagem no n8n.
- Enviar o aviso ao Telegram.

### 2. Slide 6 — Automação com n8n

Substituir o texto informal e a afirmação de que n8n local não permite webhooks por:

“O Node-RED acompanha os sensores e identifica mudanças. O n8n recebe o evento e comunica o responsável.”

“Nesta prática, o gatilho é MQTT. O n8n local conecta-se ao broker e envia a mensagem ao Telegram. Não precisamos expor um webhook público.”

Nota de fala: um webhook local atende clientes que conseguem alcançá-lo na rede. Receber chamadas externas exige um endereço acessível a esses clientes.

### 3. Slide 11 — Avisos da entrega

Substituir a imagem com alertas de sala/PIR por dois exemplos de mensagem:

```text
NexoLog | NexoLogEquipe01
Tampa aberta
Distância: 40 cm
```

```text
NexoLog | NexoLogEquipe01
Entrega em condição normal
Distância: 10 cm
```

Nota de fala: o workflow inclui também temperatura, umidade, inclinação e horário. Nenhuma mensagem precisa ser enviada repetidamente a cada leitura.

### 4. Slide 12 — Workflow da entrega

Substituir a captura da lista de workflows por uma captura do fluxo importado de `Plataformas_config/n8n/fluxo_mqtt.json`.

Destacar os três nós: `MQTT Trigger`, `Preparar mensagem`, `Avisar responsável`.

Texto: “Tópico de entrada: FIAPIoT/nexolog/equipe01/eventos”.

Nota de fala: comparar `/dados` com `/eventos`. A regra e a detecção de mudança já aconteceram no Node-RED. No n8n, configurar as credenciais e o Chat ID antes de ativar o workflow.

### 5. Slide 14 — Hands on: a mesma caixa nas próximas etapas

Texto:

1. Caixa normal: observar o dashboard.
2. Levantar a tampa: verificar um aviso.
3. Manter aberta: observar que o aviso não se repete.
4. Fechar a tampa: verificar a recuperação.

Fechamento oral: “No app15 a central enviará o comando para acender o LED. No app16 essa mesma plataforma ficará no Raspberry Pi. O que continua funcionando quando a internet cai, mas a rede local permanece?”

O slide 13 mantém sua função de mostrar o histórico de execuções. O nome antigo do workflow na captura é apenas referência visual do editor.
