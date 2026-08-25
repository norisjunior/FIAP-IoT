# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 20 - Vaccine Sense: a decisão acontece na nuvem

Evolução da Aplicação 19. Lá o modelo rodava num script e o resultado aparecia na
tela. Aqui ele vira **serviço**, e o resultado **volta para a caixa**: o n8n
recebe cada medição, chama a API que executa o `.pkl` e publica a resposta no
tópico de comando. O ESP32 escuta e toca o buzzer.

A caixa não decide nada. Ela mede, pergunta e obedece.

## O pipeline

```
ESP32  →  MQTT  →  n8n  →  API FastAPI (.pkl)  →  n8n  →  MQTT  →  ESP32
  │                                                                  │
  └── publica as 6 medições                        aciona o buzzer ──┘
```

## Estrutura

```
device/     firmware: publica as medições E assina o tópico de comando
api/        FastAPI que carrega o modelo e responde /predict
n8n/        fluxo que liga os dois
```

## 1. Subir a API

```bash
cd api
pip install -r requirements.txt
uvicorn service_app:app --host 0.0.0.0 --port 8000
```

Confira em `http://localhost:8000/docs` — o FastAPI gera a documentação sozinho,
e dá para testar o `/predict` pelo navegador antes de ligar qualquer dispositivo.

Um teste rápido pela linha de comando:

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{"tempInterna":5.0,"tempExterna":25.0,"umidade":70,"luz":3100,"criticidade":30,"distancia":58}'
```

Resposta:

```json
{"class":"CARGA_EM_PERIGO","probabilities":{"TRANSPORTE_OK":0.0,"CARGA_EM_PERIGO":1.0}}
```

## 2. Importar o fluxo no n8n

Importe `n8n/fluxo-vaccinesense-predict.json`. São seis nós:

| Nó | O que faz |
|---|---|
| `mqttTrigger` | assina `fiap/iot/vaccinesense` |
| `code` | monta o corpo da requisição com as seis medições |
| `httpRequest` | `POST` para `http://host.docker.internal:8000/predict` |
| `if` | a resposta foi `CARGA_EM_PERIGO`? |
| `mqtt` (liga) | publica `CARGA_EM_PERIGO` em `.../cmd` |
| `mqtt` (desliga) | publica `TRANSPORTE_OK` em `.../cmd` |

Configure as credenciais MQTT nos três nós que falam com o broker.

> `host.docker.internal` é como o contêiner do n8n enxerga a sua máquina. Se a
> API rodar em outro lugar, troque a URL do nó `httpRequest`.

## 3. Gravar o firmware

```bash
cd device
pio run
```

O `.ino` é o da Aplicação 19 com três acréscimos:

```cpp
const char* TOPICO_CMD = "fiap/iot/vaccinesense/cmd";

void receberComando(char* topico, byte* conteudo, unsigned int tamanho) {
  String comando(conteudo, tamanho);
  comando.trim();
  comando.toUpperCase();

  if (comando == "CARGA_EM_PERIGO") {
    tone(PIN_BUZZER, 2000);
  } else {
    noTone(PIN_BUZZER);
  }
}
```

Mais `mqtt.setCallback(receberComando)` no `setup()` e `mqtt.subscribe(TOPICO_CMD)`
ao conectar.

**Repare no que não existe aqui:** nenhum `if` sobre temperatura, luz ou
distância. O firmware não sabe o que é perigo — ele só executa o que a nuvem
mandou.

## O que demonstrar em aula

Aperte o botão para iniciar a coleta e mexa nos sliders do Wokwi:

| O que fazer | O que acontece |
|---|---|
| tudo normal | buzzer calado |
| abrir a tampa (luz alta) | buzzer toca em segundos |
| temperatura interna em 9 °C | buzzer toca |
| temperatura em 7 °C com criticidade **20** | buzzer calado |
| temperatura em 7 °C com criticidade **75** | **buzzer toca** |

As duas últimas linhas são a demonstração que vale a aula: **a mesma
temperatura**, respostas opostas. O modelo aprendeu sozinho que a faixa aceita
aperta quando a carga é crítica — ninguém escreveu esse `if` no firmware.

## A pergunta que fecha a aula

Desligue o Wi-Fi, ou pare a API, e repita o teste da tampa aberta.

O buzzer **não toca**. A carga está em risco, e o alarme está mudo — porque a
decisão mora do outro lado da rede.

Uma caixa de vacina viaja dentro de um caminhão. É esse o problema que a
Aplicação 21 resolve, levando o modelo para dentro do ESP32 com o m2cgen.

## Sobre o modelo

O `modelo_vaccinesense.pkl` vem do notebook da Aplicação 19. Para trocá-lo, basta
substituir o arquivo em `api/` e reiniciar o `uvicorn` — a API carrega o modelo
uma vez, na inicialização.

O modelo atual é uma árvore de decisão com estas importâncias:

| Feature | Importância |
|---|---:|
| `tempInterna` | 0,617 |
| `luz` | 0,195 |
| `criticidade` | 0,187 |
| `tempExterna`, `umidade`, `distancia` | 0,000 |

Três features carregam a decisão e três não participam. Vale comentar: a
`distancia` ficou em zero porque a `luz` já resolve a tampa aberta neste
conjunto, e a `tempExterna` porque ambiente quente aparece dos dois lados do
alvo — exatamente o que o protocolo de coleta pretendia.
