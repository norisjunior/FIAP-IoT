# A caixa de fumaça em inferência (demonstração do professor)

O mesmo esqueleto do motor: a caixa publica a leitura no MQTT, o n8n chama a API e a API
devolve `fire` ou `not fire`. **O firmware da caixa não muda**: é o coletor com Wi-Fi desta
pasta, publicando em `FIAPIoT/fumaca/caixa` uma vez por segundo.

```text
caixa → MQTT → n8n → API FastAPI (modelo_fumaca_incendio.pkl) → Telegram (só em fire)
```

## 1) A API

```bash
cd inferencia
python -m venv venv
venv\Scripts\activate
pip install -r requirements.txt
uvicorn service_fumaca:app --host 0.0.0.0 --port 8001
```

Porta **8001**, para rodar ao lado da API do motor (8000). Teste em
`http://localhost:8001/docs`, com uma leitura de ar parado:

```bash
curl -X POST http://localhost:8001/predict -H "Content-Type: application/json" \
  -d '{"temp":29.5,"umid":44.4,"raw_h2":14288,"raw_ethanol":19230,"pm2_5":7.6,"pm10":8.96,"nc0_5":40}'
```

Resposta: `{"class":"not fire","probabilities":{"fire":0.02,"not fire":0.98}}`.

## 2) O fluxo n8n

Importe `Fluxo-n8n-fumaca.json` e selecione as credenciais (MQTT e Telegram) e o Chat ID.

| # | Nó | Configuração |
|---|---|---|
| 1 | `MQTT Trigger` | topic `FIAPIoT/fumaca/caixa`, **JSON Parse Body** |
| 2 | `HTTP Request` | `POST http://host.docker.internal:8001/predict`, body `{{ $json.message }}` |
| 3 | `IF` | `{{ $json.class }}` é igual a `fire` |
| 4 | `Telegram` | na saída **true** do IF |

A API ignora os campos que não são features, então o fluxo manda a mensagem inteira.

## 3) O modelo

`modelo_fumaca_incendio.pkl` é a árvore de 3 níveis do `colab/treinamento_fumaca.ipynb`,
treinada nas coletas de 11/11 (as 7 features, `fire` / `not fire`):

```text
partícula (nc0_5) <= 292,55 ?  sim → temp > 32,05 ? fire : not fire
                               não → umid <= 47,30 ? fire : not fire
```

**Atenção ao calor.** A regra `temp > 32,05` veio da chama forte de 11/11. Com a caixa ligada
há muito tempo ela passa de 32 °C sozinha, e o modelo diz `fire` no ar parado, uma vez por
segundo (foi o alarme falso de 47% do dia 13/11). Ligue a caixa uns 10 minutos antes, não
horas; se alarmar, é o data drift em produção. Para não receber um Telegram por segundo,
desative o nó do Telegram e mostre a classe na saída do nó `Predict Fumaca`.
