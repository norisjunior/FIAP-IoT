# app17-0 — Gráfico da Serial em tempo real

Dois scripts Python que leem a Serial do ESP32 e desenham o gráfico:

| Script | Lê | Use com |
|---|---|---|
| `plot_accel.py` | `>acc_x`, `>acc_y`, `>acc_z` | app17-2 |
| `plot_generico.py` | qualquer linha `>nome: valor` (um gráfico por nome) | qualquer app que imprima nesse formato |

Antes de rodar:
- ajuste `PORTA` no início do script (ex.: `COM7`);
- **feche o Serial Monitor**: só um programa abre a porta por vez.

A escala do gráfico é **±4 g**. Chacoalhando forte passa disso; aumente o `setYRange` se
precisar.

O script abre a porta com DTR e RTS desligados. Em placas com conversor CH9102 ou CH340,
sem isso o ESP32 fica preso em reset e o gráfico não recebe nada. Por isso abrir a porta
**não reinicia** a placa: as mensagens de inicialização não aparecem, só as leituras.

1) Criar ambiente virtual
```
python -m venv venv
```

2) Ativar
```
.\venv\Scripts\activate.bat
```

3) Atualizar pip
```
python -m pip install --upgrade pip
```

4) Instalar dependências
```
python -m pip install -r requirements.txt
```

5) Rodar
```
python plot_accel.py
```
