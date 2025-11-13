#ifndef RANDOM_FOREST_MODEL_HPP
#define RANDOM_FOREST_MODEL_HPP

#include <string.h>

// Parâmetros do StandardScaler (extraídos do .pkl)
namespace Scaler {
    constexpr float means[5] = {
        20.9052161386f,    // Temperature
        27.6315804605f,    // Humidity  
        130.871496731f,    // Light
        690.1106636624f,    // CO2
        0.0042245147f     // HumidityRatio
    };
    
    constexpr float scales[5] = {
        1.0544937977f,    // Temperature
        4.9767829467f,    // Humidity
        210.8370594788f,    // Light  
        311.6631533248f,    // CO2
        0.0007684509f     // HumidityRatio
    };
    
    // Função para normalizar os dados
    void normalize(float* input, float* output) {
        for(int i = 0; i < 5; i++) {
            output[i] = (input[i] - means[i]) / scales[i];
        }
    }
}

// Modelo Random Forest convertido pelo m2cgen
namespace RandomForest {
    // ########################################################################
    // Início do código exportado pelo m2cgen
    // ########################################################################

    void add_vectors(double *v1, double *v2, int size, double *result) {
        for(int i = 0; i < size; ++i)
            result[i] = v1[i] + v2[i];
    }
    
    void mul_vector_number(double *v1, double num, int size, double *result) {
        for(int i = 0; i < size; ++i)
            result[i] = v1[i] * num;
    }
    
    // Função principal de inferência
    // Retorna as probabilidades: [0] = Vazia, [1] = Ocupada
    void score(double * input, double * output) {
        double var0[2];
        double var1[2];
        double var2[2];
        if (input[0] <= 0.288085013628006) {
            if (input[1] <= -0.5358040630817413) {
                var2[0] = 0.9339309428950863; var2[1] = 0.06606905710491368;
            } else {
                var2[0] = 0.9634941329856584; var2[1] = 0.03650586701434159;
            }
        } else {
            if (input[3] <= 0.2328026294708252) {
                var2[0] = 0.7029592406476829; var2[1] = 0.29704075935231716;
            } else {
                var2[0] = 0.15971705137751302; var2[1] = 0.840282948622487;
            }
        }
        double var3[2];
        if (input[2] <= 1.220350742340088) {
            if (input[2] <= 0.749410480260849) {
                var3[0] = 0.9995342771982116; var3[1] = 0.00046572280178837556;
            } else {
                var3[0] = 0.9558011049723757; var3[1] = 0.04419889502762431;
            }
        } else {
            if (input[1] <= -0.19304580986499786) {
                var3[0] = 0.09436722750548647; var3[1] = 0.9056327724945136;
            } else {
                var3[0] = 0.011385199240986717; var3[1] = 0.9886148007590133;
            }
        }
        add_vectors(var2, var3, 2, var1);
        mul_vector_number(var1, 0.5, 2, var0);
        memcpy(output, var0, 2 * sizeof(double));
    }
    // ########################################################################
    // Fim do código exportado pelo m2cgen
    // ########################################################################

    // Função auxiliar para obter a classe predita
    int predict(double* probabilities) {
        return (probabilities[1] > probabilities[0]) ? 1 : 0;
    }

    // Função para obter confiança da predição
    float getConfidence(double* probabilities) {
        float maxProb = probabilities[0];
        if (probabilities[1] > maxProb) {
            maxProb = probabilities[1];
        }
        return maxProb * 100.0f;
    }
}

#endif