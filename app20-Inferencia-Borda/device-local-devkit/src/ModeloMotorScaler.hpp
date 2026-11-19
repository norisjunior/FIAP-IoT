#ifndef STANDARD_SCALER_HPP
#define STANDARD_SCALER_HPP

// Ordem: mean_ax, mean_ay, mean_az, std_ax, std_ay, std_az, std_mag, p2p_mag.
namespace Scaler {
    const static float means[8] = {
        0.0180166667f, -0.0099375000f, 0.8177333333f, 0.1359583333f, 0.0498791667f, 0.1721666667f, 0.1668083333f, 0.5897166667f
    };

    const static float scales[8] = {
        0.0778405404f, 0.5245609595f, 0.1673189685f, 0.2292852080f, 0.0626194025f, 0.2877291357f, 0.2708430910f, 0.8470418052f
    };

    inline void standardize(const float* input, float* output) {
        for (int i = 0; i < 8; i++) {
            output[i] = (input[i] - means[i]) / scales[i];
        }
    }
}

#endif
