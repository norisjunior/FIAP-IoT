namespace ESP32Sensors {
  namespace Dryness {
    // Potenciômetro simula o nível de secura
    static uint8_t drynessPin = 0;

    // Faixa didática desejada para hr
    constexpr float DRY_MIN = 0;
    constexpr float DRY_MAX = 1022;

    struct DRY {
      float    valor;   
      bool     valido;
    };

    void inicializar(uint8_t pin) {
      drynessPin = pin;
      analogReadResolution(12);
      // Mantemos a atenuação padrão do ADC. Caso necessário, pode-se usar ADC_11db.
      pinMode(drynessPin, INPUT);
    }

    static inline float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    inline DRY ler() {
      DRY r;
      r.valor = 0;
      r.valido = false;

      // 1) Leitura do potenciômetro
      int valorLido = analogRead(drynessPin); // 0..4095
      if (valorLido < 0) return r;

      // 2) Converte a leitura para HR
      float valorObtido = mapf(static_cast<float>(valorLido), 0.0f, 4095.0f, DRY_MIN, DRY_MAX);

      // Preenche struct (sem acionar LED aqui para não acoplar módulos)
      r.valor  = valorObtido;
      r.valido = true;
      return r;
    }
  }
}
