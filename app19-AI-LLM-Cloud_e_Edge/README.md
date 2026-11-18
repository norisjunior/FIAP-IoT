# O dado deixa de ser descartado

Até aqui, cada janela do motor era classificada e esquecida. A predição acendia
um LED e acabava ali: não havia como perguntar *"como o motor estava ontem à
noite?"*, porque ninguém guardou.

Este app coloca um **PostgreSQL** no caminho e usa isso de duas formas
diferentes:

| Pasta | O que guarda | Para quê |
|---|---|---|
| [`Chatbot-CloudAI/`](Chatbot-CloudAI/README.md) | a predição da nuvem, a cada segundo | perguntar em linguagem natural como o motor está, ou como estava |
| [`Chatbot-EdgeAI/`](Chatbot-EdgeAI/README.md) | a predição da **borda**, a cada segundo; depois, a da borda e a da nuvem lado a lado | levar o histórico do motor para fora, num `.csv`; depois, medir se o modelo pequeno do dispositivo continua concordando com o modelo da nuvem |

As duas aplicações usam a mesma API, que está em [`api/`](api/) e serve a rede
neural treinada com o dataset do motor. Na `Chatbot-EdgeAI/`, só a etapa de
conferir precisa dela: guardar a predição da borda dispensa a API.

## O que é novo aqui

Nada do dispositivo. As duas aplicações medem a mesma janela de 1 segundo, com
as mesmas 8 features, e classificam nas mesmas 4 classes: `operando`,
`inclinado_frente`, `inclinado_tras` e `anomalia`.

O que muda é o **destino da predição**:

```text
antes    ESP32 → MQTT → n8n → API → MQTT → ESP32          (a resposta acende um LED e some)
Chatbot  ESP32 → MQTT → n8n → API → MQTT → ESP32
                          └──────────────→ PostgreSQL → chat com LLM
Borda    ESP32 (decide sozinho) → MQTT → n8n → PostgreSQL → .csv
Validacao ESP32 (decide sozinho) → MQTT → n8n → API → PostgreSQL
                                                  └→ as duas respostas, comparadas
```

## Antes de começar

Este app roda na **mesma plataforma das aulas anteriores**, a `IoT-platform`.
Ela já sobe o PostgreSQL, que até aqui só guardava os dados do próprio n8n, e
as tabelas deste app ficam nele. O modelo de linguagem não roda no seu
computador: é o **Gemini**, do Google, chamado pela internet com a chave do
seu grupo.

Serviços que este app usa:

| Serviço | Onde | Para quê |
|---|---|---|
| MQTT Broker | `localhost:1883` | o ESP32 publica a janela |
| n8n | `http://localhost:5678` | os fluxos das duas aplicações |
| PostgreSQL | `n8n-postgres:5432`, dentro do Docker | as duas tabelas |
| Gemini | na internet, com a chave do grupo | o modelo de linguagem do chat |

### A chave do Gemini

1. Entre em [aistudio.google.com](https://aistudio.google.com) com uma conta
   Google e clique em **Get API key** → **Create API key**. Não pede cartão.
2. No n8n, crie a credencial **Google Gemini(PaLM) Api**: cole a chave em
   **API Key** e deixe o **Host** como está.

A chave é uma senha: não vai para o GitHub nem para o grupo da turma.

O plano gratuito tem dois limites que aparecem na prática:

- **Pedidos por dia.** O agente gasta dois ou três pedidos a cada pergunta: um
  para escolher a ferramenta, outro para escrever a resposta. Acabou o limite,
  o chat devolve erro até o dia virar; use a chave de outro integrante do grupo.
- **O Google pode usar o que você manda** para melhorar os produtos dele. Dados
  do motor, tudo bem; dado pessoal, nunca.

Os fluxos já vêm com o modelo `gemini-3.5-flash-lite`. Se ele não aparecer na
lista do nó, escolha outro **Flash-Lite** ou **Flash** da lista.

## A API

```bash
cd api
python -m venv venv

venv\Scripts\activate        # Windows
source venv/bin/activate     # Linux / macOS

pip install -r requirements.txt
uvicorn service_app:app --host 0.0.0.0 --port 8000
```

É a mesma API do app anterior, servindo a mesma rede neural. As duas aplicações
apontam para `http://host.docker.internal:8000/predict` — o n8n roda em
contêiner e a API roda no seu computador, então `localhost` de dentro do n8n não
acharia ninguém.

Confira que subiu:

```bash
curl -X POST http://localhost:8000/predict \
  -H "Content-Type: application/json" \
  -d '{"mean_ax":-0.413,"mean_ay":0.811,"mean_az":0.965,"std_ax":0.017,"std_ay":0.011,"std_az":0.005,"std_mag":0.01,"p2p_mag":0.04}'
```

## Estrutura

```text
api/               service_app.py · modelo_motor_multiclasse.pkl · requirements.txt
Chatbot-CloudAI/   device/ (publica a janela) · n8n/ (ingestão + chat)
Chatbot-EdgeAI/    device/ (decide e publica) · n8n/ (ingestão da borda · comparação + chat)
```

## Por onde começar

A `Chatbot-CloudAI/` primeiro: ela reaproveita o firmware que você já conhece e
o que muda está todo no n8n. A `Chatbot-EdgeAI/` traz um firmware novo — o
primeiro deste curso que **decide sozinho e ainda assim fala com a nuvem**.

As duas têm a mesma forma: um fluxo que ingere sem parar e um fluxo de chat com
um agente LLM. O que muda é a pergunta que o agente responde — *como o motor
está* de um lado, *o modelo da borda ainda é confiável* do outro. A
`Chatbot-EdgeAI/` começa por um fluxo a mais: a predição da borda no banco,
antes da comparação com a nuvem.
