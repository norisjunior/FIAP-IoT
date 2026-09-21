#ifndef STANDARD_SCALER_HPP
#define STANDARD_SCALER_HPP

// Ordem: mean_ax, mean_ay, mean_az, std_ax, std_ay, std_az, std_mag, p2p_mag.
namespace Scaler {
    const static float means[8] = {
        -0.1239266667f, -0.0139733333f, 0.8788266667f, 0.2479155556f, 0.1327711111f, 0.2249933333f, 0.2033222222f, 0.7809222222f
    };

    const static float scales[8] = {
        0.4441638451f, 0.4540238508f, 0.1856842510f, 0.4790651795f, 0.1467829798f, 0.3230766988f, 0.2651809876f, 1.1172538121f
    };

    inline void standardize(const float* input, float* output) {
        for (int i = 0; i < 8; i++) {
            output[i] = (input[i] - means[i]) / scales[i];
        }
    }
}

#endif
