// INMP441 — microfone I2S.
// Ligacao: VDD->3V3  GND->GND  L/R->GND  SCK->26  WS->25  SD->33
//
// Este arquivo faz pelo INMP441 o que o analogRead() faz por um sensor
// analogico: esconde o barramento. Nao precisa ser digitado em aula.
//
// Se vier so silencio, troque ONLY_LEFT por ONLY_RIGHT.

#pragma once
#include <Arduino.h>
#include <driver/i2s.h>

namespace INMP441 {

const uint32_t TAXA  = 16000;  // amostras por segundo
const int      GANHO = 11;     // bits descartados do valor de 24 bits do sensor

inline void iniciar() {
  i2s_config_t cfg = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = TAXA,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 8,
    .dma_buf_len          = 256,
    .use_apll             = false,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
  };

  i2s_pin_config_t pinos = {
    .mck_io_num   = I2S_PIN_NO_CHANGE,
    .bck_io_num   = 26,
    .ws_io_num    = 25,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num  = 33
  };

  i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pinos);
}

// Preenche destino[] com n amostras do microfone.
inline void ler(int16_t *destino, int n) {
  int32_t bruto[256];
  int lidas = 0;

  while (lidas < n) {
    int pedido = (n - lidas < 256) ? (n - lidas) : 256;
    size_t bytes = 0;
    i2s_read(I2S_NUM_0, bruto, pedido * sizeof(int32_t), &bytes, portMAX_DELAY);

    for (size_t i = 0; i < bytes / sizeof(int32_t); i++) {
      destino[lidas++] = bruto[i] >> GANHO;
    }
  }
}

}  // namespace INMP441
