# Fase 2 — O ciclo se fecha

Aqui a decisão vira automática. O n8n escuta a caixa, chama a API e devolve o
resultado pelo MQTT. O ESP32 recebe o comando e **toca o buzzer**.

Ninguém digita nada.

## O que sobe junto

```bash
cd VaccineSense_service
docker compose up --build
```

| Serviço | Porta | Papel |
|---|---:|---|
| `mosquitto` | 1883 | broker MQTT |
| `ml-service` | 8000 | FastAPI com o modelo |
| `n8n` | 5678 | orquestra os dois |

O modelo entra por volume (`ml-service/model/`), então trocar o `.pkl` e
reiniciar já muda o comportamento — sem rebuild da imagem.

## Importar o fluxo

Abra `http://localhost:5678` e importe `n8n/fluxo-vaccinesense-predict.json`.

| Nó | O que faz |
|---|---|
| `mqttTrigger` | assina `fiap/iot/vaccinesense` |
| `code` | monta o corpo com as seis medições |
| `httpRequest` | `POST` para `http://ml-service:8000/predict` |
| `if` | a resposta foi `CARGA_EM_PERIGO`? |
| `mqtt` | publica o resultado em `.../cmd` |

Configure a credencial MQTT nos três nós apontando para `mosquitto:1883`.

> Dentro do compose os serviços se enxergam pelo **nome**: `ml-service`,
> `mosquitto`. De fora, é `localhost`.

## Gravar o firmware

O `.ino` em `../device/` publica as medições **e assina o tópico de comando**.
Aponte o `BROKER_IP` para a máquina que roda o compose.

## A demonstração

Aperte o botão da caixa e mexa nos sliders do Wokwi:

| O que fazer | O que acontece |
|---|---|
| tudo normal | buzzer calado |
| abrir a tampa | **buzzer toca** em segundos |
| 7 °C com criticidade 20 | buzzer calado |
| 7 °C com criticidade 75 | **buzzer toca** |

O firmware não tem nenhum `if` sobre sensor. Ele só executa o que a nuvem
mandou.

## A pergunta que fecha a fase

Pare o `ml-service` e repita o teste da tampa aberta.

O buzzer **não toca**. A carga está em risco e o alarme está mudo, porque a
decisão mora do outro lado da rede — e uma caixa de vacina viaja dentro de um
caminhão.

É o problema que a Aplicação 21 resolve, levando o modelo para dentro do ESP32.
