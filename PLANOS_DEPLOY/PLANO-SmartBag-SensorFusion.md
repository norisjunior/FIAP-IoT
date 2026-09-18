# Plano de implementação — SmartBag Sensor Fusion

Data: 2026-09-13. Estado: app19 implementado (09-14) e alinhado ao app15 (09-15); integração Wokwi/InfluxDB pendente.

## 1. Objetivo e limites

Evoluir a cópia do NexoLog em `C:\Projects\FIAP-IoT-eval\app19_SmartBag_SensorFusion` para a sequência:

```text
coleta → dataset → Random Forest → API → RF embarcada → TinyML/MLP embarcada
```

O app14 original permanece intacto. Alterar somente o app19 e os novos apps descritos abaixo. Este roteiro fica em `C:\Projects\FIAP-IoT-develop\PLANOS_DEPLOY` para poder ser retomado em outras sessões. Não executar publicação externa como parte da implementação local.

Preservar Arduino/PlatformIO/Wokwi, funções simples, comentários em português, blocos de configuração e namespaces `ESP32Sensors`. Não criar framework, classes de sensores, biblioteca compartilhada entre apps nem refatoração geral. Cada exemplo deve abrir e compilar independentemente.

## 2. Referências examinadas

Os caminhos abaixo são relativos aos repositórios indicados.

| Repositório / referência | O que reutilizar |
|---|---|
| eval: `app19_SmartBag_SensorFusion/device/src/app14-NexoLog.ino` e quatro `ESP32Sensors*.hpp` | Base efetiva da cópia: constantes, funções, namespaces, FastIMU, JSON, MQTT, reconexão por `millis()` |
| eval: `app14_CPS_e_Automation/app14_NexoLog` | Origem da organização e pinagem; somente referência |
| develop: `OLD-ML-com-IoT/app18-GerarDatasetEscalares_Fusion/app18-Fluxo-Node-RED-Influx.json` | Função simples que converte payload MQTT em campos para InfluxDB |
| develop: `OLD-ML-com-IoT/app19-GerarDatasetEscalares_VaccineSense` | Identificação de rodada, MQTT → Node-RED → InfluxDB; notebook `colab/app18_coleta_e_rotulagem.ipynb` com consulta SQL via `InfluxDBClient3`, anotação de rodadas e exportação CSV |
| develop: `OLD-ML-com-IoT/app20-Inferencia-AI-Cloud_Escalares_VaccineSense/colab/app19_treinamento_vaccinesense.ipynb` | Células numeradas, upload/download Colab, seleção explícita de features, split agrupado, relatório e `joblib.dump` |
| develop: `OLD-ML-com-IoT/app21-Inferencia-AI-API_Escalares_VaccineSense/api/service_app.py` | FastAPI, modelo Pydantic simples, carga única com joblib, DataFrame na ordem das features, `/predict` |
| eval: `app30-AIoT-EdgeAI-RF-Occupancy_micromlgen` | `ESP32SensorsLDR.hpp`, vetor de entrada, `Eloquent::ML::Port::RandomForest`, `predict` e LED |
| eval: `app31-TinyML-TensorFlowLite/app23-TinyML-TensorFlowLite` | `MicroTFLite`, header do modelo, tensor arena, `ModelInit`, `ModelSetInput`, `ModelRunInference`, `ModelGetOutput` |

Adaptações necessárias, sem corrigir as referências: o DHT atual retorna inválido quando chamado antes de 2 s; o NexoLog publica a cada 2,5 s; a referência de API ainda usa MQTT no firmware; o notebook antigo usa uma busca de modelos mais extensa do que esta aula requer. A nova API terá envio HTTP direto, conforme solicitado. Não copiar o protocolo de 24 rodadas, o GridSearchCV nem o scaler de Occupancy para a RF nova.

## 3. Menor sequência de apps

Cinco etapas didáticas, quatro apps: treinamento e exportação são notebooks, não precisam de firmware próprio.

| App em eval | Etapas | Organização proposta |
|---|---|---|
| `app19_SmartBag_SensorFusion` (já existe) | 1, 2 e treinamento da 5: coleta, CSV, RF e MLP | Preservar `device/` e `fluxo/`; acrescentar `colab/`, `dataset_gerado/`, README e guia |
| `app20_Inferencia_API_SmartBag` | 3: ESP32 envia seis features à API | `device/`, `api/`, README |
| `app21_EdgeAI_RF_SmartBag` | 4: RF local, sem API | `device/`, README; exportação no notebook RF do app19 |
| `app22_TinyML_SmartBag` | 5: MLP local | `device/`, README; notebook no app19 |

Esses nomes são a proposta deste plano. Conferir novamente a disponibilidade antes de criar diretórios em uma sessão futura. Não renomear apps anteriores.

## 4. Contrato único dos dados

Ordem obrigatória em CSV, notebook, API e vetores C++:

```python
FEATURES = ["temperatura", "umidade", "delta_distancia", "luz", "mov_max", "incl_max"]
CLASSES = ["ENTREGA_OK", "REVISAR_ENTREGA"]
```

| Feature | Definição |
|---|---|
| temperatura | °C, última leitura válida do DHT22 em cache |
| umidade | %, mesmo ciclo de leitura da temperatura |
| delta_distancia | cm, `distanciaAtual - distBase`; preservar sinal |
| luz | RAW de 0 a 4095, analogRead com resolução de 12 bits; mesma leitura nas quatro versões |
| mov_max | m/s², maior `fabs(sqrt(x*x+y*y+z*z)-9.80665)` no intervalo |
| incl_max | graus, maior ângulo do vetor de aceleração em relação ao eixo vertical de montagem no intervalo |

CSV de trabalho: `timestamp,device,rodada,situacao,temperatura,umidade,delta_distancia,luz,mov_max,incl_max,target`.

Preservar metadados no CSV para rastrear a coleta e separar rodadas. Somente as seis features entram em X. `target` contém os nomes das duas classes; no treinamento, mapear explicitamente 0=ENTREGA_OK e 1=REVISAR_ENTREGA para facilitar os dois modelos embarcados. Rejeitar rótulos desconhecidos, sem convertê-los silenciosamente em zero.

## 5. Etapa 1 — coleta

### Firmware e hardware

1. Adaptar o `.ino` da cópia, renomeando para `app19-SmartBag.ino`; preservar cabeçalhos atuais.
2. Usar `ESP32SensorsLDR.hpp` com analogRead de 12 bits no GPIO 35. Sem conversão para lux; os extremos 0 e 4095 também são publicados. Atualizar diagrama e notebooks para RAW.
3. Manter temperatura/umidade em cache e chamar o DHT a cada 2,5 s. Publicar a última leitura válida; sem leitura inicial, null. Não adicionar expiração ou indicadores de qualidade à versão didática.
4. Ao iniciar a primeira coleta da rodada, obter cinco distâncias válidas, espaçadas em pelo menos 60 ms, com a bag fechada/parada. Guardar a média em distBase. Limitar a tentativa a 5 s; se falhar, publicar delta null e tentar novamente ao parar/iniciar a coleta. Nunca substituir ausência de eco por 400.
5. Manter baseline durante a rodada. Mudança de carga que exija nova referência inicia outra rodada e calibração; não recalibrar silenciosamente ao abrir a tampa.
6. Amostrar MPU aproximadamente a cada 50 ms por `millis()`. Reutilizar a conversão FastIMU g → m/s² e `medirMovimentacao`. Acrescentar somente uma função de inclinação ao header existente.
7. Montar o MPU no corpo da bag, com +Z para cima quando vertical. Calcular `acos(clamp(z/norma, -1, 1))` em graus, rejeitando norma quase zero e dados não finitos. Se a montagem tiver outra orientação, ajustar explicitamente o eixo antes de coletar, mantendo-o nas quatro versões.
8. Atualizar dois acumuladores máximos; a cada 1000 ms ler distância/LDR, montar amostra com cache DHT e máximos, e zerar ambos para o próximo intervalo. Nunca carregar máximos anteriores para uma nova janela.
9. Publicar uma amostra por segundo somente durante coleta, com seis features e device/rodada/situacao. Sem id, segundo_coleta, coletando, valido, falhas_distancia, leituras_mpu, janela_ms, distancia ou dist_base no JSON. O bool coletando existe apenas internamente para os botões/LED/publicação. Sem eco, delta=null; máximos começam em NAN e usam fmaxf, resultando em null se não houve leitura. Remover contadores de falha/amostragem, flags de janela e validação por duração. Reset dos máximos diretamente no loop, depois de publicar e ao iniciar coleta.
10. LED do app19 exclusivamente aceso durante coleta e apagado quando parada. Ausência de medida aparece como null, sem alterar o LED ou definir target. O indicador das futuras versões de inferência será tratado na respectiva etapa.

O ângulo do acelerômetro é uma aproximação afetada por aceleração dinâmica. Um buraco pode elevar movimento e inclinação estimada. Os máximos também perdem a ordem dos acontecimentos: abertura no fim da janela e impacto no começo podem parecer simultâneos. Documentar e incluir esses casos na coleta, sem acrescentar FFT, RMS, séries de entrada ou fusão complexa. A MLP recebe exatamente as mesmas limitações da RF.

### Identificação e protocolo

Usar dois botões com INPUT_PULLUP, no padrão do app17-7: COLETA (27) inicia/para a situação selecionada; SITUAÇÃO (26) avança apenas com coleta parada. Tratar aperto e debounce de 300 ms diretamente no loop. Rodada começa em 1 e só aumenta ao passar da última situação à primeira. Essa passagem marca baselinePendente; a próxima coleta recalibra com a bag fechada/parada. Pausar/retomar mantém rodada e situação; zera tempo da coleta e máximos. Remover novaViagem(), viagem, sessao e numeroViagem. Serial apenas exibe dados.

O Colab seleciona um device e início/fim de uma execução contínua sem reset, mantendo SELECT SQL. Cada rodada reúne as sete situações e forma o grupo do split. Não juntar reinicializações diferentes: rodada volta a 1. Não há contagem de tempo de coleta no payload nem descarte automático dos primeiros segundos. Preparar a situação antes de iniciar e parar durante as transições.

Executar as sete situações em duas rodadas completas inicialmente: 14 coletas de situações, não 24. Propor 45–60 s úteis por situação, sem prolongar impacto isolado artificialmente. Durante a coleta de buraco, intercalar transporte normal e poucos impactos. Variar carga entre rodadas, iluminação e intensidade; não reproduzir exatamente os mesmos controles.

| Situação | Rótulo anotado | Contraponto que precisa aparecer nos dados |
|---|---|---|
| Parada, fechada | ENTREGA_OK | Luz residual, diferentes temperaturas/umidades |
| Transporte normal | ENTREGA_OK | Movimento e inclinação moderada variáveis |
| Buraco / impacto pontual | ENTREGA_OK | Movimento alto também ocorre sem problema |
| Abertura na entrega, pouco movimento | ENTREGA_OK | Valores de luz e delta semelhantes também ocorrem sem problema |
| Abertura em movimento | REVISAR_ENTREGA | Sobrepor luz/delta com entrega e movimento com transporte/buraco |
| Tombamento | REVISAR_ENTREGA | Variar movimento e aproximar inclinações das observadas em casos OK |
| Problema térmico | REVISAR_ENTREGA | Registrar condição conhecida do ensaio e variar as demais features; não criar target por um if de temperatura |

Os rótulos vêm da situação observada e anotada, não de limiares calculados no notebook ou firmware. Caso térmico deve ter contexto didático definido no guia; não alegar que essas seis features detectam duração de exposição ou adequação específica de qualquer produto. Se os rótulos dependerem de informação ausente das seis entradas, haverá ambiguidade que o modelo não consegue eliminar. Não fabricar sobreposição alterando medições nem inverter rótulos para forçar dificuldade.

### MQTT, InfluxDB e CSV

- Node-RED em `NodeRED/`, regerado a partir do `Fluxo_2` do app15 para manter a mesma estrutura de nós. Tópico e measurement próprios, sem afetar NexoLog; sem dashboard e sem n8n neste app — aqui o fluxo é só transporte.
- Enviar seis features e apenas device/rodada/situacao. Node-RED grava os valores disponíveis; não há filtro por coletando/valido, pois o ESP32 publica somente durante a coleta. Null fica como campo ausente no InfluxDB.
- Manter measurement smartbag_raw_adc_2026. Colab consulta SELECT * com início/fim, seleciona a equipe, associa situacao ao target, remove incompletas/não finitas e salva CSV. Sem camada de validação de produção.
- Rotular pela situação efetivamente executada. Mostrar contagens por rodada/situacao/target; permitir descartar manualmente um par incorreto. Não rotular por limiar de sensores.
- Guardar CSV completo com grupos em `dataset_gerado/smartbag_dataset.csv` somente após coleta. Não entregar dados sintéticos como medições reais.

## 6. Etapa 2 — Random Forest no Colab

Criar `colab/app19_treinamento_smartbag_rf.ipynb`, curto e executável célula por célula:

1. Instalar/importar pandas, matplotlib, scikit-learn e joblib; upload do CSV.
2. Conferir contrato, não finitos, classes e pelo menos duas rodadas; selecionar FEATURES e mapear target.
3. Separar por rodada como no app17-7: a última rodada fica no teste e todas as anteriores no treino. Conferir presença das duas classes em ambos; repetir a mesma divisão na MLP.
4. Treinar `Pipeline(StandardScaler, RandomForestClassifier)` com 21 árvores, profundidade livre e `random_state=42`, sem busca de hiperparâmetros. Registrar configuração efetiva.
5. Avaliar acurácia, matriz de confusão, `classification_report` e `feature_importances_`.
6. Como verificação didática curta, comparar com classe majoritária e árvores de um único corte treinadas separadamente em cada feature, sempre com o mesmo split. Inspecionar também distribuições e confusões por situação. Importância sozinha não demonstra combinação de features.
7. Salvar o Pipeline avaliado como `modelo_smartbag.pkl` com joblib, recarregar e verificar predição. Registrar versões e rodadas usadas em metadados JSON, com ordem, unidades, classes e o mapa 0/1. Pinar `numpy==2.1.3`, `pandas==2.2.3`, `scikit-learn==1.6.1`, `joblib==1.5.3` e `micromlgen==1.1.28`, as mesmas do app17-7/app18: quem exige o pin é o `joblib.load` da API, não o micromlgen, que roda até com sklearn 1.9.
8. Acrescentar células finais para micromlgen na etapa 4; não treinar uma floresta diferente sem reavaliar.

Os ~89% e ~67% informados são resultados prévios do usuário em dataset sintético, não metas garantidas nem resultados desta coleta. Se uma única feature resolver quase tudo, revisar o protocolo e coletar contrapontos reais. Não ajustar rótulos ou hiperparâmetros olhando repetidamente o teste. Duas rodadas servem à demonstração; para ajuste e avaliação mais confiável, reservar rodadas adicionais inteiras.

## 7. Etapa 3 — API

Criar `app20_Inferencia_API_SmartBag/api/service_app.py`, `requirements.txt` e documentação local. Copiar o `.pkl` produzido, com suas versões registradas, para a pasta da API. Carregar uma vez; se o arquivo faltar, explicar como gerá-lo, sem substituir por modelo fictício.

Adaptar FastAPI/Pydantic da referência. `POST /predict` recebe somente os seis campos numéricos finitos, constrói DataFrame com FEATURES, chama `predict` e devolve `{"classe":"ENTREGA_OK"}` ou `{"classe":"REVISAR_ENTREGA"}`. Remover dependência de `named_steps`, pois aqui se salva o estimador RF diretamente.

**Decisão de 2026-09-15: o transporte é n8n, não HTTP direto no dispositivo.** O ESP32 publica as seis features em MQTT; o n8n assina, chama a API e devolve a classe por MQTT; o firmware acende a saída correspondente. É exatamente o caminho do app18 e do app28, que os alunos já viram, e evita HTTP síncrono dentro do ciclo de 50 ms do MPU. Em todo o eval só o app13 usa HTTPClient — introduzir esse padrão aqui seria ensinar rede numa aula de ML.

Derivar `device/` do app19 validado, com o mesmo cálculo e unidades, removendo os botões: como no app18, os pinos que eram entrada do rótulo humano viram saída do rótulo do modelo. Tratar ausência de resposta, JSON inválido e classe desconhecida como falha de comunicação, sem converter em ENTREGA_OK. A saída mantém a última decisão recebida até a próxima chegar.

O `.pkl` é um Pipeline com o StandardScaler dentro, então a API chama `predict` sobre o DataFrame em valores originais — o padrão do app18 vale sem adaptação, inclusive `type(modelo[-1]).__name__`. Como o treino mapeia 0/1, a API traduz o inteiro de volta para o nome da classe, usando `mapa_classes` dos metadados.

## 8. Etapa 4 — RF embarcada

No notebook RF, usar `micromlgen.port` para converter o estimador salvo, com classes numéricas 0 e 1, para `AIoTRandomForest_micromlgen.hpp`. Conferir a API e compatibilidade das versões efetivamente instaladas quando executar. Não escrever árvores manualmente.

Criar `app21_EdgeAI_RF_SmartBag/device/` a partir da aquisição validada. Instanciar `Eloquent::ML::Port::RandomForest`, montar `float dados[6]` na ordem contratada, aplicar `Scaler::std()` e chamar `predict`.

**Decisão de 2026-09-15: a RF é treinada dentro de `Pipeline(StandardScaler, RandomForestClassifier)` e o scaler vai embarcado**, espelhando o app30. A árvore não precisa de normalização, mas o objetivo didático é o aluno aplicar o mesmo `Scaler::std()` nos apps 21 e 22. O micromlgen porta só a floresta: o scaler sai em `AIoTSmartBagScaler.hpp` à parte, e a conferência Python/C++ usa as entradas já normalizadas. Não transportar scaler, modelo ou features de Occupancy — só o formato do header.

**21 árvores, não 20.** O `vote.jinja` desempata com `>` estrito, então empate em número par vence sempre `votes[0]` = ENTREGA_OK, silenciosamente.

Comparar Python e C++ gerado sobre as mesmas amostras de teste, incluindo fronteiras. Não presumir equivalência perfeita: conferir semântica de votação e precisão numérica do exportador. Se houver divergências, investigar e registrar antes de afirmar equivalência. Medir flash/RAM e tempo de execução; se reduzir a floresta for necessário, reavaliar e atualizar API/header a partir do mesmo artefato.

## 9. Etapa 5 — MLP TinyML

Criar `app19_SmartBag_SensorFusion/colab/app19_treinamento_smartbag_mlp.ipynb` usando o mesmo CSV, FEATURES, classes e divisão por rodadas da RF.

- Começar com `Input(6) → Dense(8, relu) → Dense(1, sigmoid)`, saída 1=REVISAR_ENTREGA, decisão em 0,5. Só acrescentar camada se houver evidência em validação independente.
- Ajustar StandardScaler somente no treino. Exportar média e escala das seis features em header simples `AIoTSmartBagScaler.hpp`; aplicar `(x-media)/escala` exatamente uma vez no ESP32.
- Com duas rodadas, usar número fixo de épocas e não usar a rodada de teste para early stopping. Se precisar selecionar configuração, coletar terceira rodada para validação agrupada.
- Avaliar relatório e matriz de confusão no mesmo teste da RF. Converter inicialmente para `.tflite` float32, evitando complexidade de quantização desnecessária para seis entradas.
- Gerar `modelo_smartbag.h` com bytes reais do `.tflite`. Salvar também parâmetros de normalização e metadados. Não preencher header com modelo de irrigação ou bytes fictícios.
- Seguir MicroTFLite do app31, inicialmente sua arena de 8 KiB como ponto de partida, ajustando apenas se a alocação real exigir. Manter dependências necessárias; não copiar servo e sensores de irrigação.
- Comparar saídas Keras, interpretador TFLite e ESP32 com as mesmas entradas normalizadas, registrando tolerância numérica e classe. Conferir entrada 6, saída 1 e falhas de inicialização/inferência.

## 10. Inventário previsto de arquivos

Lista abaixo é planejamento, não arquivos já criados.

| Local | Criar/adaptar |
|---|---|
| app19 | `README.md`, `Guia-NexoLog.md`, `device/src/app19-SmartBag.ino`, headers de sensores necessários, `platformio.ini`, `diagram.json`, documentação do firmware, fluxo Node-RED e instruções, três notebooks em `colab/` |
| app19, após executar coleta/treino | `dataset_gerado/smartbag_dataset.csv`, `.pkl`, metadados e header micromlgen baixados do Colab |
| app20 | README, `device/` independente com `.ino`, headers, PlatformIO/Wokwi; `api/service_app.py`, `api/requirements.txt`, modelo gerado |
| app21 | README, `device/` independente com aquisição, inferência e header RF gerado |
| app22 | README, `device/` independente, `.tflite`, header do modelo e scaler gerados |

Preservar `wokwi.toml` em cada device e conferir caminhos para `.pio/build/esp32/firmware.bin` e `.elf`. Não adicionar arquivos vazios de dataset/modelo apenas para completar a árvore. Evitar multiplicar guias equivalentes; README explica como executar e o guia concentra o protocolo.

## 11. Ordem de execução e critérios de conclusão

- [x] Examinar padrões locais e registrar este roteiro.
- [ ] Etapa 1: implementar aquisição/fluxo/notebook; compilar com PlatformIO; abrir Wokwi; conferir baseline, sinal do delta, cache DHT, RAW, máximos e reset por intervalo, cadência e identificação. Simular perda de eco, ausência de MPU e falha de rede.
- [ ] Coletar duas rodadas completas e executar notebook de rotulagem; verificar exportação e grupos. Hardware/coleta humana não disponíveis devem ser registrados como pendência, nunca presumidos executados.
- [ ] Etapa 2: executar notebook do início ao fim com CSV real, verificar split sem rodadas compartilhadas, relatório e `.pkl` recarregado.
- [x] Etapa 3 implementada em 2026-09-16 (`3e5c647`), como `app20_Inferencia_API_SmartBag`: `device/` sem botões, `api/` com o Pipeline e `n8n/` fazendo a ponte. API validada com TestClient e Pipeline sintético temporário (duas classes, campo extra ignorado, campo faltando e nulo em 422, caminho sem modelo); firmware compila (RAM 45.572, flash 791.549). **Falta** rodar com o `.pkl` real, ensaiar no Wokwi com n8n e API no ar, e comparar a classe da API com a predição direta do notebook.
- [ ] Etapa 4: gerar RF, comparar Python/C++, compilar e testar no Wokwi sem API, registrar flash/RAM e latência.
- [ ] Etapa 5: treinar/exportar MLP, conferir scaler e Keras/TFLite/ESP32, compilar e executar no Wokwi, comparar métricas com RF no mesmo teste.
- [ ] Revisar diff: nenhum app anterior alterado; nenhum token ou artefato fictício; documentar arquivos criados, versões e verificações realmente executadas.

Não é necessário criar suíte extensa: usar validações de contrato nos notebooks/API, comparação dos artefatos exportados e ensaios dos sensores/cadência, que são os riscos centrais.

## 12. Retomada em outra sessão

Ler este plano e conferir alterações do usuário antes de editar. A versão atual do app19 é didática; não reintroduzir controles de qualidade ou telemetria de produção sem solicitação.

Estado vigente em 2026-09-15 (alinhamento com o app15):

- app19 alinhado ao app15 e comitado em dois passos: `41feb8c` (estado do dia 14, que so existia na working tree) e `4913195` (alinhamento).
- Heranças da cópia antiga removidas: `fluxo/n8n/` inteiro, `dashboard.json` e `NodeRED-Info.md`. `Guia-NexoLog.md` virou `Guia-SmartBag.md`; `instalar_mqtt.md` restaurado do app15; `fluxo/NodeRED/` virou `NodeRED/`, no layout do app18.
- Firmware nas convenções do app15: Serial em CSV com CRLF e linha de título no `setup()`, `setKeepAlive(120)`, `WiFiClient.h`. Removida a guarda sem explicação no botão SITUAÇÃO. Build: RAM 45.644, flash 790.685.
- `Fluxo_2` regerado a partir do arquivo do app15: mesma estrutura de nós, `mqtt in` utf8 + nó `json`, broker `mosquitto`, função `[campos, tags]`. Campo nulo é pulado na escrita, como confirmado no app15.
- `CONSTRUIR-O-FIRMWARE.md` (quatro iterações, com tabela de paridade com o app15) e `CONSTRUIR-O-FLUXO.md` (duas iterações) reescritos no padrão "do zero, não diff".
- Notebooks: versões pinadas como no app17-7/app18; RF virou Pipeline com StandardScaler e 21 árvores; scaler exportado em `AIoTSmartBagScaler.hpp` no formato `Scaler::std()` do app30, com os mesmos números nos dois notebooks; mapa 0/1 nos metadados; `.loc` no lugar de `.iloc`; situação fora do mapa falha em vez de sumir no dropna.
- Caminho da RF conferido de ponta a ponta com CSV sintético nas versões pinadas: Pipeline, `.pkl` recarregado, folhas puras, `port()` e semântica de votação do header batendo com o scikit-learn em 350 amostras, sem empates. Os artefatos do teste não foram colocados no app.
- Wokwi interativo, InfluxDB real e coleta humana continuam pendentes. Apps 20–22 ainda não implementados. Próximo passo: duas rodadas completas e execução dos Colabs com o CSV real.

Decisões do usuário em 2026-09-15: Node-RED e não n8n no app19; sem Grafana no app19; sem NTP, mas com rodadas; SQL no Colab; app20 serve a RF (`.pkl`) via n8n; scaler embarcado nos apps 21 e 22.

Decisões de 2026-09-16: a plataforma sobe pela `IoT-platform` (Compose único), com `docker compose up -d` dentro do diretório — o aluno recebe essa pasta, não o repositório, então o roteiro é textual, sem link relativo para fora do app — `instalar_mqtt.md` está aposentado e foi removido do app19. A coleta grava no **InfluxDB Cloud**, não no `influxdb:2.7` local da plataforma, porque o Colab consulta com SQL (API v3); da plataforma o app19 usa Mosquitto e Node-RED. Atenção ao duplo sentido de "Edge": app16 é Near Edge/Fog (Raspberry), apps 21/22 são modelo no microcontrolador.

Estado anterior, em 2026-09-14:

- app19 implementado com dois botões, LED de coleta, rodada igual ao app17-7, baseline, seis features escalares e LDR RAW. Publica apenas durante coleta.
- Payload: temperatura, umidade, delta_distancia, luz, mov_max, incl_max, device, rodada, situacao. Sem controles/diagnósticos extras.
- Coleta Colab simplificada: SQL → rótulo pela situação → dropna → CSV. RF e MLP usam última rodada como teste. Três notebooks continuam no app19.
- Build aprovado (RAM 45.644 bytes, flash 790.725 bytes). Testes locais do controle/JSON verificaram publicação somente durante coleta, máximos, null e payload de nove campos; função Node-RED exercitada sem diagnósticos.
- Três notebooks reexecutados com dados temporários; divisão rodada 1/rodada 2, RF Python/C++ e MLP Keras/TFLite conferidas. Os modelos da fixture não foram colocados nos apps.
- README, Guia-NexoLog, CONSTRUIR-O-FIRMWARE e CONSTRUIR-O-FLUXO atualizados.
- Wokwi interativo, InfluxDB real e coleta humana ainda pendentes. Apps20–22 ainda não implementados. Próximo passo: duas rodadas completas e execução dos Colabs com o CSV real.

As decisões anteriores sobre sessão aleatória, função novaViagem, toque longo, lux, LED de falha, validade, contadores e descarte de transições foram substituídas pelos ajustes acima.
Simplificação do HC-SR04 no app19: medirDistancia() retorna float em cm, ou NAN sem eco. Removida a struct DISTANCIA e seu bool valido. O baseline usa isfinite(dist); o JSON mantém delta null na ausência de eco.
