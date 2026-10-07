namespace ESP32Sensors {
  namespace HR {
    // Potenciômetro simula o nível de CO2
    static uint8_t hrPin = 0;

    // Faixa didática desejada para hr
    constexpr float HR_MIN = 0.002674f;   // baseline atmosférico aproximado
    constexpr float HR_MAX = 0.006476f;  // limite superior típico para ambiente fechado

    struct DADOS_HR {
      float    valor;   // CO2 mapeado linearmente (413..2000)
      bool     valido;
    };

    void inicializar(uint8_t pin) {
      hrPin = pin;
      analogReadResolution(12);
      // Mantemos a atenuação padrão do ADC. Caso necessário, pode-se usar ADC_11db.
      pinMode(hrPin, INPUT);
    }

    static inline float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    inline DADOS_HR ler() {
      DADOS_HR r;
      r.valor = 0;
      r.valido = false;

      // 1) Leitura do potenciômetro
      int valorLido = analogRead(hrPin); // 0..4095
      if (valorLido < 0) return r;

      // 2) Converte a leitura para HR
      float valorObtido = mapf(static_cast<float>(valorLido), 0.0f, 4095.0f, HR_MIN, HR_MAX);

      // Preenche struct (sem acionar LED aqui para não acoplar módulos)
      r.valor  = valorObtido;
      r.valido = true;
      return r;
    }
  }
}
