// Ponte C para o Dart FFI. Expoe o classificador do Edge Impulse.
#include <stdint.h>
#include <stddef.h>
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "model-parameters/model_metadata.h"
#include "model-parameters/model_variables.h"

#define WW_EXPORT extern "C" __attribute__((visibility("default"))) __attribute__((used))

static const int16_t *g_samples = nullptr;

// O MFCC do Edge Impulse espera a escala int16 crua, nao [-1,1]. So cast.
static int ww_get_data(size_t offset, size_t length, float *out_ptr) {
    for (size_t i = 0; i < length; i++) {
        out_ptr[i] = static_cast<float>(g_samples[offset + i]);
    }
    return 0;
}

WW_EXPORT int ww_window_samples() {
    return EI_CLASSIFIER_RAW_SAMPLE_COUNT;
}

WW_EXPORT int ww_sample_rate() {
    return EI_CLASSIFIER_FREQUENCY;
}

WW_EXPORT int ww_label_count() {
    return EI_CLASSIFIER_LABEL_COUNT;
}

WW_EXPORT const char *ww_label(int idx) {
    if (idx < 0 || idx >= EI_CLASSIFIER_LABEL_COUNT) return "";
    return ei_classifier_inferencing_categories[idx];
}

// Retorna 0 em sucesso. out_scores recebe uma probabilidade por label.
WW_EXPORT int ww_classify(const int16_t *samples, int count,
                          float *out_scores, int out_len) {
    if (samples == nullptr || out_scores == nullptr) return -1;
    if (count != EI_CLASSIFIER_RAW_SAMPLE_COUNT) return -2;
    if (out_len < EI_CLASSIFIER_LABEL_COUNT) return -3;

    g_samples = samples;

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
    signal.get_data = &ww_get_data;

    ei_impulse_result_t result = {0};
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
    g_samples = nullptr;
    if (err != EI_IMPULSE_OK) return static_cast<int>(err);

    for (int i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        out_scores[i] = result.classification[i].value;
    }
    return 0;
}
