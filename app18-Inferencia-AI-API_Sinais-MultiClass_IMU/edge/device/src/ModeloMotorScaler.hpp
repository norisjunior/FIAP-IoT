#ifndef STANDARD_SCALER_HPP
#define STANDARD_SCALER_HPP

// ATENCAO: SCALER SINTETICO, do mesmo treino que gerou o ModeloMotorRF.hpp sintetico.
// Substitua pelo arquivo que o seu Colab gerar -- os dois SEMPRE juntos, do mesmo treino.
//
// StandardScaler ajustado no treino: guarda a media e o desvio-padrao de cada
// PADRONIZACAO: (input - means) / scales, deixando media 0 e desvio 1.
// Ordem: mean_ax, mean_ay, mean_az, std_ax, std_ay, std_az, std_mag, p2p_mag.
namespace Scaler {
    const static float means[8] = {
        -0.0009419483f, -0.0000197652f, 0.9548196720f, 0.0648160148f, 0.0666950176f, 0.0644961376f, 0.0798845106f, 0.3629787513f
    };

    const static float scales[8] = {
        0.2970160040f, 0.0074532413f, 0.0455328939f, 0.0617378420f, 0.0633172231f, 0.0591534782f, 0.0748611406f, 0.3413349236f
    };

    inline void standardize(const float* input, float* output) {
        for (int i = 0; i < 8; i++) {
            output[i] = (input[i] - means[i]) / scales[i];
        }
    }
}

#endif
