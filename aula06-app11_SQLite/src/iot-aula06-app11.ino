#include <Arduino.h>
#include <SD.h>
#include <sqlite3.h>
#include "ESP32Sensors.hpp"   // LED, DHT22, MPU6050

#define SD_CS 5
#define DB_PATH "/sd/sensores.db"   //O SD Card é montado em /sd/
#define INTERVALO_COLETA_MS 5000

sqlite3 *db = nullptr;
unsigned long ultimoRegistro = 0;

// Declaração de funções
void inicializaBanco();
void gravaMedicoes();
void imprimeBanco();

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!SD.begin(SD_CS)) {
    Serial.println("[FATAL] Falha ao montar o SD Card. O programa será interrompido.");
    while (true) {}
  }

  ESP32Sensors::beginAll();

  inicializaBanco();

  // Só imprime que está pronto se o banco de dados foi inicializado com sucesso
  if (db) {
    Serial.println("[OK] Sistema pronto - gravando no SQLite no SD Card.");
  } else {
    Serial.println("[FATAL] Falha ao inicializar o banco de dados. O programa será interrompido.");
    while (true) {}
  }
}

void loop() {
  if (millis() - ultimoRegistro >= INTERVALO_COLETA_MS) {
    ultimoRegistro = millis();
    gravaMedicoes();
  }
}

void inicializaBanco() {
  // Tenta abrir o banco de dados. Se falhar, o ponteiro 'db' permanecerá nullptr.
  if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
    Serial.printf("[ERRO] Não abriu/criou banco SQLite: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db); // É seguro chamar close mesmo se open falhou parcialmente
    db = nullptr; // Garante que o ponteiro é nulo em caso de falha
    return;
  }

  Serial.println("[OK] Banco de dados aberto com sucesso.");

  const char *sql =
    "CREATE TABLE IF NOT EXISTS leituras ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "timestamp TEXT,"
    "temp_C REAL, umid_pct REAL, ic_C REAL,"
    "ax_g REAL, ay_g REAL, az_g REAL);";

  char *errMsg = nullptr;
  if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
    Serial.printf("[ERRO] Criar tabela: %s\n", errMsg);
    sqlite3_free(errMsg);
    // Se a criação da tabela falhou, fechamos o DB e consideramos uma falha geral
    sqlite3_close(db);
    db = nullptr;
  } else {
    Serial.println("[OK] Tabela 'leituras' verificada/criada com sucesso.");
  }
  // Não fechamos o banco de dados aqui! Ele permanecerá aberto.
}

void gravaMedicoes() {
  // Checa se o banco de dados está aberto antes de usar
  if (!db) {
    Serial.println("[ERRO] Banco de dados não está aberto. Pulando gravação.");
    return;
  }

  ESP32Sensors::Ambiente::AMBIENTE ambiente = ESP32Sensors::Ambiente::medirAmbiente();
  if (!ambiente.valido) {
    Serial.println("[ERRO] Leituras DHT inválidas - pulando ciclo");
    return;
  }

  sensors_event_t accel = ESP32Sensors::Accel::medirAccel();
  ESP32Sensors::LED::on();

  const char *sql =
    "INSERT INTO leituras "
    "(timestamp, temp_C, umid_pct, ic_C, ax_g, ay_g, az_g) "
    "VALUES (datetime('now','localtime'), ?, ?, ?, ?, ?, ?);";

  sqlite3_stmt *stmt;
  // A função não abre mais o banco, apenas o utiliza
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
    sqlite3_bind_double(stmt, 1, ambiente.temp);
    sqlite3_bind_double(stmt, 2, ambiente.umid);
    sqlite3_bind_double(stmt, 3, ambiente.ic);
    sqlite3_bind_double(stmt, 4, accel.acceleration.x);
    sqlite3_bind_double(stmt, 5, accel.acceleration.y);
    sqlite3_bind_double(stmt, 6, accel.acceleration.z);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
      // Mensagem de sucesso apenas se a inserção funcionou
      Serial.println("[SALVO] Registro inserido no banco.");
    } else {
      Serial.printf("[ERRO] Falha ao inserir dados: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
  } else {
    Serial.printf("[ERRO] Falha no prepare statement: %s\n", sqlite3_errmsg(db));
  }
  // Não fechamos o banco de dados aqui!

  ESP32Sensors::LED::off();
  imprimeBanco(); // Chama a função de impressão para debug
}

void imprimeBanco() {
  if (!db) {
    Serial.println("[ERRO] Banco de dados não está aberto para leitura.");
    return;
  }

  const char *sql = "SELECT * FROM leituras ORDER BY id DESC LIMIT 5;"; // Imprime os 5 últimos
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
    Serial.println("--- Últimos 5 registros no Banco de Dados ---");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      Serial.printf("[%d] %s | T: %.2f°C | U: %.2f%% | IC: %.2f°C | A(g): %.2f, %.2f, %.2f\n",
                    sqlite3_column_int(stmt, 0),
                    sqlite3_column_text(stmt, 1),
                    sqlite3_column_double(stmt, 2),
                    sqlite3_column_double(stmt, 3),
                    sqlite3_column_double(stmt, 4),
                    sqlite3_column_double(stmt, 5),
                    sqlite3_column_double(stmt, 6),
                    sqlite3_column_double(stmt, 7));
      Serial.println("");
    }
        
    Serial.println("---------------------------------------------");
    sqlite3_finalize(stmt);
  } else {
    Serial.printf("[ERRO] Falha ao preparar leitura do DB: %s\n", sqlite3_errmsg(db));
  }
  // Não fechamos o banco de dados aqui!
}