/*
 * ESP32TFLite.hpp
 * Camada de abstração para TensorFlow Lite Micro no ESP32
 * 
 * Uso simples:
 *   1. Inclua seu modelo: #include "modelo_xxx.h"
 *   2. Inicialize: ESP32TFLite::inicializar(modelo_xxx, modelo_xxx_len, NUM_INPUTS, NUM_OUTPUTS);
 *   3. Execute inferência: ESP32TFLite::inferir(entradas, saidas);
 */

#ifndef ESP32_TFLITE_HPP
#define ESP32_TFLITE_HPP

#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

namespace ESP32TFLite {
    // Tamanho da arena de memória (ajuste se necessário)
    constexpr int kTensorArenaSize = 8 * 1024;

    // Variáveis internas do módulo
    static uint8_t* tensorArena = nullptr;
    static const tflite::Model* modelo = nullptr;
    static tflite::MicroInterpreter* interpreter = nullptr;
    static TfLiteTensor* tensorEntrada = nullptr;
    static TfLiteTensor* tensorSaida = nullptr;
    static tflite::AllOpsResolver resolver;

    static int numEntradas = 0;
    static int numSaidas = 0;
    static bool inicializado = false;

    /**
     * Inicializa o interpretador TFLite com o modelo fornecido
     * 
     * @param modeloData      Ponteiro para o array do modelo (gerado via xxd ou similar)
     * @param modeloLen       Tamanho do modelo em bytes
     * @param qtdEntradas     Número de entradas do modelo
     * @param qtdSaidas       Número de saídas do modelo
     * @return true se inicializado com sucesso
     */
    bool inicializar(const unsigned char* modeloData, unsigned int modeloLen, int qtdEntradas, int qtdSaidas) {
        
        // Aloca arena de memória
        tensorArena = (uint8_t*)heap_caps_malloc(kTensorArenaSize, MALLOC_CAP_8BIT);
        if (tensorArena == nullptr) {
            Serial.println("[TFLite] ERRO: Falha ao alocar arena de memória");
            return false;
        }

        // Carrega o modelo
        modelo = tflite::GetModel(modeloData);
        if (modelo->version() != TFLITE_SCHEMA_VERSION) {
            Serial.printf("[TFLite] ERRO: Versão do modelo (%d) incompatível com schema (%d)\n", 
                        modelo->version(), TFLITE_SCHEMA_VERSION);
            return false;
        }

        // Cria o interpretador
        static tflite::MicroInterpreter staticInterpreter(modelo, resolver, tensorArena, kTensorArenaSize);
        interpreter = &staticInterpreter;

        // Aloca tensores
        if (interpreter->AllocateTensors() != kTfLiteOk) {
            Serial.println("[TFLite] ERRO: Falha ao alocar tensores");
            return false;
        }

        // Obtém ponteiros para entrada e saída
        tensorEntrada = interpreter->input(0);
        tensorSaida = interpreter->output(0);

        numEntradas = qtdEntradas;
        numSaidas = qtdSaidas;
        inicializado = true;

        // Info de debug
        Serial.println("[TFLite] Modelo carregado com sucesso!");
        Serial.printf("[TFLite] Tamanho do modelo: %d bytes\n", modeloLen);
        Serial.printf("[TFLite] Entradas: %d | Saídas: %d\n", numEntradas, numSaidas);
        Serial.printf("[TFLite] Arena usada: %d bytes\n", interpreter->arena_used_bytes());

        return true;
    }

    /**
     * Executa inferência no modelo
     * 
     * @param entradas   Array de floats com os valores de entrada
     * @param saidas     Array de floats para receber os valores de saída
     * @return true se a inferência foi executada com sucesso
     */
    bool inferir(float* entradas, float* saidas) {
        if (!inicializado) {
            Serial.println("[TFLite] ERRO: Modelo não inicializado!");
            return false;
        }

        // Copia dados de entrada para o tensor
        for (int i = 0; i < numEntradas; i++) {
            tensorEntrada->data.f[i] = entradas[i];
        }

        // Executa inferência
        if (interpreter->Invoke() != kTfLiteOk) {
            Serial.println("[TFLite] ERRO: Falha na inferência");
            return false;
        }

        // Copia dados de saída
        for (int i = 0; i < numSaidas; i++) {
            saidas[i] = tensorSaida->data.f[i];
        }

        return true;
    }

    /**
     * Verifica se o modelo está inicializado
     */
    bool estaInicializado() {
        return inicializado;
    }

    /**
     * Retorna quantidade de memória usada pela arena
     */
    size_t memoriaUsada() {
        if (!inicializado) return 0;
        return interpreter->arena_used_bytes();
    }

} // namespace ESP32TFLite

#endif // ESP32_TFLITE_HPP
