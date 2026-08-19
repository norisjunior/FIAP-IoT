# Roteiro da aula: MQTT ao vivo com 10 grupos

Demonstração em dois sentidos, com o broker rodando no notebook do professor e todos
os dispositivos no roteador TP-Link da sala. **Sem dependência de internet:** se a
rede do campus cair no meio da aula, a demonstração continua.

```
        [ESP32 professor + HC-SR04]  ──publish──┐
                                                ▼
   Notebook: Mosquitto (1883) + Node-RED (1880)      <- "o servidor"
                                                ▲ │
        [10x ESP32 dos grupos]  ──publish botão─┘ └──subscribe──> LED + buzzer
```

---

## 1. Materiais

**Professor:** 1 ESP32 DevKit, 1 HC-SR04, roteador TP-Link, notebook com Docker Desktop.

**Cada grupo:** 1 ESP32 DevKit, 1 LED, 1 resistor 220 Ω, 1 buzzer **ativo**,
1 botão, protoboard, jumpers, cabo USB.

---

## 2. Rede — o ponto de falha número 1

O código quase nunca é o problema desta aula. A rede é. Resolva isto **na véspera**.

### Roteador TP-Link

| Item | O que fazer | Por quê |
|---|---|---|
| Banda | SSID exclusivo em **2,4 GHz** | O ESP32 não fala 5 GHz. Em roteadores com Smart Connect, desligue e separe os SSIDs |
| Segurança | **WPA2-PSK** (não WPA2/WPA3 misto) | O ESP32 clássico engasga com WPA3 |
| SSID e senha | curtos, sem acento e sem símbolo | Dez grupos vão digitar isso no código |
| **Isolamento de AP** | **DESLIGAR** (Wireless → Avançado → "Isolamento de AP") | Ligado, ele bloqueia cliente↔cliente: o WiFi conecta e o MQTT simplesmente não. É a falha mais traiçoeira |
| DHCP | Reserva de endereço para o MAC do notebook | Para o IP do broker não mudar entre a véspera e a aula |

### Notebook (broker)

```powershell
# 1. Subir a plataforma (Docker Desktop no Windows, NÃO docker dentro do WSL2 puro:
#    no WSL2 a porta fica presa na VM e não aparece na LAN)
cd ..\..\IoT-platform
.\start-windows.ps1

# 2. Liberar as portas no firewall do Windows (inbound é bloqueado por padrão)
New-NetFirewallRule -DisplayName "MQTT e Node-RED (aula IoT)" -Direction Inbound `
  -Protocol TCP -LocalPort 1883,1880 -Action Allow

# 3. Descobrir o IP do notebook NA REDE DO TP-LINK
ipconfig
# Se o notebook também estiver no cabo/WiFi do campus, haverá mais de um adaptador.
# Use o IP do adaptador conectado ao TP-Link (tipicamente 192.168.0.x).
```

**Escreva esse IP no quadro.** É o `BROKER_IP` que os dez grupos vão digitar.

### Teste de aceitação (véspera)

De **outro** dispositivo na rede do TP-Link:

```bash
mosquitto_sub -h <IP_DO_NOTEBOOK> -t teste -v
```

Se responder, a aula está garantida. Se não responder, é firewall ou isolamento de AP —
nesta ordem de probabilidade.

---

## 3. Comandos do professor durante a aula

Todos rodam pelo container, sem instalar nada no Windows:

```powershell
# Ver TODAS as mensagens da turma (a visão crua do protocolo)
docker exec -it mqtt-broker mosquitto_sub -t 'fiap/iot/#' -v

# Ver só os botões dos grupos (Ato 3 - sem a enxurrada de distância)
docker exec -it mqtt-broker mosquitto_sub -t 'fiap/iot/2026b/grupo/+/botao' -v

# Contador de clientes conectados ao broker (Ato 0)
docker exec -it mqtt-broker mosquitto_sub -t '$SYS/broker/clients/connected' -v

# PING: faz o LED de todos os grupos piscar uma vez
docker exec -it mqtt-broker mosquitto_pub -t 'fiap/iot/2026b/prof/ping' -m '1'

# Piscar SÓ o grupo 07 (Ato 5)
docker exec -it mqtt-broker mosquitto_pub -t 'fiap/iot/2026b/grupo/07/cmd' -m 'BLINK'

# Limpar mensagens retained ao final da aula
docker exec -it mqtt-broker mosquitto_pub -t 'fiap/iot/2026b/prof/dist' -r -n
```

Dashboard do Node-RED: `http://<IP_DO_NOTEBOOK>:1880/ui`
(os alunos também conseguem abrir no celular — eles estão na mesma rede).

---

## 4. A demonstração em 5 atos

### Ato 0 — Chamada MQTT (~5 min)

Todos gravam o firmware da Aplicação 07. No telão:

```powershell
docker exec -it mqtt-broker mosquitto_sub -t '$SYS/broker/clients/connected' -v
```

O número sobe conforme a turma grava: 2… 5… 9… 11. Antes de qualquer conceito, a
sala **se vê conectando**. É o gancho e, ao mesmo tempo, o diagnóstico: quem não
apareceu, você resolve agora e não no meio da demonstração.

Em seguida, publique o `ping`: todo LED pisca uma vez. Quem não piscou, levanta a mão.

### Ato 1 — O fan-out (1 → N)

Ligue o dispositivo do professor e aproxime a mão do HC-SR04. A sala acende.
Só isso. Deixe o silêncio trabalhar.

Depois pergunte: **"quantos fios existem entre o meu sensor e o LED de vocês?"**
Nenhum. Meu ESP32 não sabe que vocês existem; o de vocês não sabe que eu existo.
Isso é desacoplamento no espaço, e é a razão de o MQTT existir.

### Ato 2 — A onda

Cada grupo já configurou o próprio `MEU_LIMIAR`:

| Grupo | 01 | 02 | 03 | 04 | 05 | 06 | 07 | 08 | 09 | 10 |
|---|---|---|---|---|---|---|---|---|---|---|
| cm | 120 | 100 | 85 | 70 | 60 | 50 | 40 | 30 | 20 | 12 |

Caminhe **devagar** em direção ao sensor: a sala acende em ondas, grupo por grupo,
e apaga na ordem inversa quando você recua.

Uma publicação, dez assinantes, dez regras diferentes — e o broker não sabe nada
sobre nenhuma delas. O professor publicou **um número**, não um comando.

Segundo round: "descomentem o `digitalWrite(BUZZER, ligado)` e regravem". Mesmo firmware, outro
atuador — e agora a onda tem som.

### Ato 3 — O fan-in (N → 1)

Inverta o sentido. Cada grupo publica em `fiap/iot/2026b/grupo/<NN>/botao`;
você assina `fiap/iot/2026b/grupo/+/botao`. Duas telas lado a lado:

- terminal com `mosquitto_sub -v` — a visão crua, uma linha por mensagem;
- dashboard do Node-RED — a visão de produto.

Rode como **buzzer de quiz**: faça uma pergunta, e o primeiro grupo que aparecer na
tabela responde. O coringa `+` se explica sozinho quando alguém perguntar *"como o
senhor pegou o número de todos os grupos com uma linha só?"*.

### Ato 4 — Retained

Peça a um grupo para desligar e religar o ESP32 enquanto você segura a mão perto do
sensor. O LED deles acerta o estado **na hora**, sem esperar a próxima publicação.

Desacoplamento no tempo, em um `true` a mais no `publish`. É por isso que um app de
casa inteligente já abre mostrando o estado das lâmpadas.

### Ato 5 — Unicast de volta

Publique em `fiap/iot/2026b/grupo/07/cmd`: só o grupo 07 pisca.
Seu sensor → todos piscam. Este comando → um só. Mesmo código, mesma conexão,
tópico diferente. É a deixa para hierarquia de tópicos, `+` versus `#` e endereçamento.

---

## 5. Plano B

| Problema | Contorno |
|---|---|
| O roteador não sobe | Ponto de acesso móvel do Windows — **mas o teto é de 8 dispositivos**: rode em duas rodadas de 5 grupos |
| Um grupo não conecta ao broker | Empreste um ESP32 já gravado; o problema é quase sempre `BROKER_IP` ou SSID digitado errado |
| Docker não sobe na hora | `mosquitto` instalado nativo no Windows com `listener 1883` + `allow_anonymous true` já sustenta os Atos 0 a 5 (só o dashboard fica de fora) |
| Nada funciona na rede da sala | Broker público (`broker.emqx.io:1883`) com o prefixo de tópico da turma — exige que a rede libere a porta 1883 de saída |

---

## 6. Depois da demonstração

O antigo semáforo inteligente vira o **exercício**: "agora usem MQTT para construir
um semáforo que fecha quando N pedestres forem detectados em 60 s". Lógica de negócio
é ótima — só não na aula em que o protocolo está sendo apresentado.
