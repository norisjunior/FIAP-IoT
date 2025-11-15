#ifndef STANDARD_SCALER_HPP
#define STANDARD_SCALER_HPP

namespace Scaler {
    const static float means[5] = {
        20.9052161386f, 27.6315804605f, 130.8714967310f, 690.1106636624f, 0.0042245147f
    };
    
    const static float scales[5] = {
        1.0544937977f, 4.9767829467f, 210.8370594788f, 311.6631533248f, 0.0007684509f
    };
    
    inline void std(const float* input, float* output) {
        for (int i = 0; i < 5; i++) {
            output[i] = (input[i] - means[i]) / scales[i];
        }
    }
}

#endif
