# Servidor da demonstração de MQTT

Três formas de "receber" as mensagens dos grupos, da mais crua para a mais acabada.
Na aula vale usar mais de uma ao mesmo tempo: a visão crua prova que não há mágica,
a dashboard mostra o que vira produto.

---

## 1. Terminal — a visão crua do protocolo

Roda pelo próprio container, sem instalar nada no Windows:

```powershell
# tudo que trafega na turma
docker exec -it mqtt-broker mosquitto_sub -t 'fiap/iot/#' -v

# só os botões dos grupos (Ato 3, sem a enxurrada de distância)
docker exec -it mqtt-broker mosquitto_sub -t 'fiap/iot/2026b/grupo/+/botao' -v

# quantos dispositivos estão conectados ao broker agora (Ato 0)
docker exec -it mqtt-broker mosquitto_sub -t '$SYS/broker/clients/connected' -v
```

Publicar:

```powershell
# PING: todos os grupos piscam uma vez
docker exec -it mqtt-broker mosquitto_pub -t 'fiap/iot/2026b/prof/ping' -m '1'

# só o grupo 07 pisca (Ato 5 - unicast)
docker exec -it mqtt-broker mosquitto_pub -t 'fiap/iot/2026b/grupo/07/cmd' -m 'BLINK'

# limpar as mensagens retained ao final da aula
docker exec -it mqtt-broker mosquitto_pub -t 'fiap/iot/2026b/prof/dist' -r -n
```

---

## 2. `recebe_botoes.py` — o servidor em código

```bash
pip install paho-mqtt
python recebe_botoes.py 192.168.0.100
```

Painel de texto que se redesenha a cada mensagem: distância atual, ranking de quem
apertou primeiro. Serve para mostrar que **o servidor não tem nada
de especial — é só mais um cliente MQTT**, com o mesmo `connect`/`subscribe`/callback
que os ESP32 usam.

Argumentos: `python recebe_botoes.py [IP_DO_BROKER] [TURMA]`
(padrões: `localhost` e `2026b`).

---

## 3. `flow-nodered.json` — a dashboard

**Importar:** Node-RED (`http://<IP>:1880`, admin / FIAPIoT) → menu ☰ → *Import* →
*select a file to import* → `flow-nodered.json` → *Deploy*.

**Abrir:** `http://<IP_DO_NOTEBOOK>:1880/ui`
Os alunos também conseguem abrir no celular — estão na mesma rede.

| Grupo na dashboard | O que mostra | Ato |
|---|---|---|
| 1. Sensor do professor | gauge com a distância em tempo real | 1 e 2 |
| 2. Botões dos grupos | ranking de quem apertou primeiro, com botão de zerar | 3 |
| 3. Controles do professor | botão PING e contador de clientes conectados (`$SYS`) | 0 |

O nó de broker já vem configurado para `mqtt-broker:1883` — o nome do container na
rede Docker, conforme o [IoT-platform](../../IoT-platform/README.md). Se o Node-RED
estiver fora do Docker, troque para o IP do notebook.

### Se algum nó aparecer como "unknown"

A dashboard usa `node-red-dashboard` (nós `ui_gauge`, `ui_table`, `ui_button`,
`ui_text`). Instale por *Manage palette → Install → node-red-dashboard* e faça o
import de novo.

---

## Tópicos da turma

```
fiap/iot/2026b/prof/dist            professor publica (retained)
fiap/iot/2026b/prof/ping            professor publica: todos piscam
fiap/iot/2026b/grupo/<NN>/botao     grupo publica
fiap/iot/2026b/grupo/<NN>/cmd       professor publica só para aquele grupo
fiap/iot/2026b/grupo/+/botao        o servidor assina (coringa de um nível)
fiap/iot/#                          tudo (coringa de subárvore)
```

Para trocar de turma, altere `TURMA` nos dois `.ino`, no `flow-nodered.json`
(campos `topic` dos nós MQTT) e passe o novo valor como 2º argumento do script Python.
