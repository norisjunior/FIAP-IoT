# Forma 2 — Coletor Serial → CSV

Script Python que lê o stream `ax,ay,az` do `app17-10-RawSerialCSV` e grava o CSV bruto
rotulado pedido na **Sprint 3**: `timestamp,ax,ay,az,label`.

Estilo linear (igual ao `app17-0-Plot`): sem classes, configuração no topo do arquivo.

## Instalar

```bash
python -m venv .venv
.\.venv\Scripts\activate     # Windows
python -m pip install -r requirements.txt
```

## Configurar a conexão (topo do `coletor_raw.py`)

Escolha **uma**:

- **(A) Wokwi** (padrão): `PORTA = "rfc2217://localhost:4000"`.
  Exige que o `wokwi.toml` do app17-10 tenha `rfc2217ServerPort = 4000` e que a **aba do
  simulador fique visível** no VS Code (senão a simulação pausa).
- **(B) ESP32 físico**: `PORTA = "COM6"` (ajuste à sua porta). **Feche o Serial Monitor** antes
  de rodar — só um programa pode abrir a porta COM por vez.

## Coletar

```bash
python coletor_raw.py
```

1. O script pergunta a **label** da rodada: `normal` (MPU parado) ou `anomalo` (chacoalhando).
2. Gera um arquivo `coleta_<label>_<data>.csv` e grava enquanto recebe dados.
3. **Ctrl+C** para parar. Faça **uma rodada por classe** (ex.: 1 min cada → ~6000 amostras).

> **Protocolo (Aula 14, slide 22):** mantenha a condição estável e **descarte os primeiros
> segundos** (transição). Para anomalia, chacoalhe o ESP32/MPU (ou sacuda o MPU no Wokwi)
> durante toda a coleta.

## Saída (formato da Sprint 3)

```csv
timestamp,ax,ay,az,label
2026-06-26 20:30:01.123,0.120,-0.040,9.810,normal
2026-06-26 20:30:01.133,0.180,-0.020,9.790,normal
```

## Mapeamento Sprint

- **Sprint 3 – item 2:** captura com registro temporal (timestamp por amostra) em CSV.
- **Sprint 3 – item 3:** base experimental por condição (rodadas `normal` e `anomalo`).

A visualização (item 4) e as features/modelo (Sprint 4) ficam nos notebooks `2.3`/`2.4`/`2.5`.

## Limitações

- **Timestamp de recepção:** é a hora do PC ao ler a linha, não o instante da medição no ESP32.
- **Anomalia simulada:** chacoalho/sacudida não substitui ensaio industrial de vibração.
