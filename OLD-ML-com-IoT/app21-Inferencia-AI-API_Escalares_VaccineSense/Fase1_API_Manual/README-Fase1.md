# Fase 1 — A API responde

O modelo sai do notebook e vira **serviço**. Nada é automático ainda: você
pergunta na mão e olha a resposta.

## Rodar

```bash
pip install -r requirements.txt
uvicorn service_app:app --host 0.0.0.0 --port 8000
```

## Testar sem escrever código

Abra `http://localhost:8000/docs`.

O FastAPI gera a documentação sozinho, com um botão **Try it out** em cada
rota. Dá para testar o modelo inteiro pelo navegador, antes de ligar qualquer
dispositivo.

## Testar pela linha de comando

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{"tempInterna":5.0,"tempExterna":25.0,"umidade":70,"luz":3100,"criticidade":30,"distancia":58}'
```

```json
{"class":"CARGA_EM_PERIGO","probabilities":{"TRANSPORTE_OK":0.0,"CARGA_EM_PERIGO":1.0}}
```

## Os cinco testes da aula

| tempInterna | criticidade | luz | Resposta esperada |
|---:|---:|---:|---|
| 5,0 | 20 | 55 | `TRANSPORTE_OK` |
| 5,0 | 20 | **3100** | `CARGA_EM_PERIGO` — tampa aberta |
| **9,0** | 20 | 55 | `CARGA_EM_PERIGO` — carga quente |
| 7,0 | **20** | 55 | `TRANSPORTE_OK` |
| 7,0 | **75** | 55 | **`CARGA_EM_PERIGO`** |

As duas últimas linhas são o ponto da fase: **a mesma temperatura**, respostas
opostas. Quem decide é a criticidade, e o modelo aprendeu isso sozinho no
notebook da Aplicação 19.

## O que ficou faltando

Alguém precisa perguntar. A caixa publica no MQTT, mas ninguém escuta — e o
buzzer continua mudo.

É o que a Fase 2 resolve.
