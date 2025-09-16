namespace ESP32Sensors {
  namespace CO2 {
    // Potenciômetro simula o nível de CO2
    const uint8_t CO2_PIN = 34;

    // Faixa didática desejada para CO2 em ppm
    constexpr float PPM_MIN = 413.0f;   // baseline atmosférico aproximado
    constexpr float PPM_MAX = 2000.0f;  // limite superior típico para ambiente fechado

    struct DADOS_CO2 {
      uint16_t raw;   // leitura bruta do ADC (0..4095)
      float    ppm;   // CO2 mapeado linearmente (413..2000)
      uint8_t  intensity; // intensidade 0..255 (análoga ao exemplo com analogWrite)
      bool     valido;
    };

    inline void inicializar() {
      analogReadResolution(12);
      // Mantemos a atenuação padrão do ADC. Caso necessário, pode-se usar ADC_11db.
      pinMode(CO2_PIN, INPUT);
    }

    static inline float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    inline DADOS_CO2 ler() {
      DADOS_CO2 r;
      r.raw = 0;
      r.ppm = 0.0f;
      r.intensity = 0;
      r.valido = false;

      // 1) Leitura do potenciômetro
      int potValue = analogRead(CO2_PIN); // 0..4095
      if (potValue < 0) return r;

      // 2) Mapeia intensidade 0..255
      int ledIntensity = map(potValue, 0, 4095, 0, 255);

      // 3) Converte a leitura para CO2 (ppm): intervalo 413 a 2000
      float ppm = mapf(static_cast<float>(potValue), 0.0f, 4095.0f, PPM_MIN, PPM_MAX);

      // Preenche struct (sem acionar LED aqui para não acoplar módulos)
      r.raw       = static_cast<uint16_t>(potValue);
      r.ppm       = ppm;
      r.intensity = static_cast<uint8_t>(ledIntensity);
      r.valido    = true;
      return r;
    }
  }
}
