namespace ESP32Sensors {
  namespace LDR {

    const uint8_t LDR_PIN = 35;

    // Constantes do modelo LDR (fornecidas)
    constexpr float RL10_kOhm = 50.0f;   // resistência a 10 lux (kΩ)
    constexpr float GAMMA     = 0.7f;    // expoente característico
    constexpr float R_BOARD   = 10400.0f; // resistência interna/series (ohms)

    struct DADOS_LDR {
      float lux;   // Iluminância estimada (lux)
      bool  valido;
    };

    void inicializar() {
      analogReadResolution(12); // 0..4095
      pinMode(LDR_PIN, INPUT);
    }

    DADOS_LDR ler() {
      DADOS_LDR r;
      r.lux = 0.0f;
      r.valido = false;
      
      int amostra = analogRead(LDR_PIN);
      if (amostra < 0) return r;

      // Conversões
      const float vref = 3.3f;
      float voltagem = (amostra / 4095.0f) * vref;
      // Evita divisão por zero ou saturações
      if (voltagem <= 0.001f) voltagem = 0.001f;
      if (voltagem >= (vref - 0.001f)) voltagem = vref - 0.001f;

      float resistenciaLDR = (R_BOARD * voltagem) / (vref - voltagem);
      // Fórmula: lux = (RL10*1000*10^GAMMA / R_ldr)^(1/GAMMA)
      float numerador = (RL10_kOhm * 1000.0f) * powf(10.0f, GAMMA);
      r.lux = powf(numerador / resistenciaLDR, 1.0f / GAMMA);

      r.valido = true;
      return r;
    }
  }
}
