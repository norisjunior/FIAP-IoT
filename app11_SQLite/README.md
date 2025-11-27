# Physical Computing, Embedded AI, Robotics & Cognitive IoT

## Aplicação 11 - Banco de Dados SQLite em Cartão SD

Esta aplicação demonstra o uso de banco de dados SQLite armazenado em cartão SD para persistência estruturada de dados de sensores. O sistema coleta dados de temperatura, umidade e acelerômetro, armazenando-os em um banco relacional.

### Sensores e Atuadores

**Sensores:**
- DHT22 (Temperatura e Umidade)
- MPU6050 (Acelerômetro)

**Atuadores:**
- LED (Indica gravação de dados)

**Interface SD Card (SPI):**
- CS (Chip Select) - Pino GPIO 5
- SCK, MISO, MOSI - Pinos SPI padrão

### Funcionamento

O sistema cria e mantém um banco de dados SQLite (/sd/sensores.db) no cartão SD, coletando dados a cada 5 segundos:

**Estrutura da Tabela:**
```sql
CREATE TABLE leituras (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  timestamp TEXT,
  temp_C REAL,
  umid_pct REAL,
  ic_C REAL,
  ax_g REAL,
  ay_g REAL,
  az_g REAL
);
```

**Campos armazenados:**
- Timestamp (data/hora local)
- Temperatura em °C
- Umidade em %
- Índice de calor em °C
- Aceleração nos eixos X, Y, Z em g (gravidade)

O uso de SQLite permite consultas SQL avançadas, filtragem por data/hora, agregações e análise estruturada dos dados coletados. O LED pisca durante cada gravação no banco.