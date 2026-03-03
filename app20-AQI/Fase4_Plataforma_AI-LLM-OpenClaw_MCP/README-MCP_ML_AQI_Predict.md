README MCP Server


1) O que é seu “MCP” na prática (para Opção 1)

Para a aula, pense nele como um Tool Server HTTP com 4 funções:

aqi_atual (consulta no Postgres a última medição)

aqi_historico (consulta por data + período)

aqi_predict (proxy para o seu POST /predict)

mqtt_publish (publica um alerta no broker)

O OpenClaw vai usar isso via Skill (instruções) + ferramentas nativas dele (ex.: web_fetch/curl), sem você precisar “registrar tool schema” por plugin.


2) Inicializar o /predict (seja usando ML_AQI_Manual ou ML_AQI_stack)
Como rodar

rode seu /predict (o app que você já tem):

uvicorn app:app --host 0.0.0.0 --port 8000


3) Inicializar o MCP Server
rode o MCP:

uvicorn aqi_mcp_server:app --host 0.0.0.0 --port 8088