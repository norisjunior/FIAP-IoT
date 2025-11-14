#ifndef AIOT_RANDOMFOREST_MC2GEN_HPP
#define AIOT_RANDOMFOREST_MC2GEN_HPP

#include <math.h>
#include <string.h>

namespace RandomForest {

    void add_vectors(double *v1, double *v2, int size, double *result) {
        for(int i = 0; i < size; ++i)
            result[i] = v1[i] + v2[i];
    }
    void mul_vector_number(double *v1, double num, int size, double *result) {
        for(int i = 0; i < size; ++i)
            result[i] = v1[i] * num;
    }
    void score(double * input, double * output) {
        double var0[2];
        double var1[2];
        double var2[2];
        double var3[2];
        if (input[0] <= 0.288085013628006) {
            if (input[1] <= -0.5358040630817413) {
                var3[0] = 0.9339309428950863; var3[1] = 0.06606905710491368;
            } else {
                var3[0] = 0.9634941329856584; var3[1] = 0.03650586701434159;
            }
        } else {
            if (input[3] <= 0.2328026294708252) {
                var3[0] = 0.7029592406476829; var3[1] = 0.29704075935231716;
            } else {
                var3[0] = 0.15971705137751302; var3[1] = 0.840282948622487;
            }
        }
        double var4[2];
        if (input[2] <= 1.220350742340088) {
            if (input[2] <= 0.749410480260849) {
                var4[0] = 0.9995342771982116; var4[1] = 0.00046572280178837556;
            } else {
                var4[0] = 0.9558011049723757; var4[1] = 0.04419889502762431;
            }
        } else {
            if (input[1] <= -0.19304580986499786) {
                var4[0] = 0.09436722750548647; var4[1] = 0.9056327724945136;
            } else {
                var4[0] = 0.011385199240986717; var4[1] = 0.9886148007590133;
            }
        }
        add_vectors(var3, var4, 2, var2);
        double var5[2];
        if (input[2] <= 1.220350742340088) {
            if (input[4] <= 1.3675192594528198) {
                var5[0] = 0.9991470002843332; var5[1] = 0.0008529997156667614;
            } else {
                var5[0] = 0.9954954954954955; var5[1] = 0.0045045045045045045;
            }
        } else {
            if (input[0] <= -0.18851468712091446) {
                var5[0] = 0.1744186046511628; var5[1] = 0.8255813953488372;
            } else {
                var5[0] = 0.03348837209302326; var5[1] = 0.9665116279069768;
            }
        }
        add_vectors(var2, var5, 2, var1);
        mul_vector_number(var1, 0.3333333333333333, 2, var0);
        memcpy(output, var0, 2 * sizeof(double));
    }

} // namespace RandomForest

#endif
