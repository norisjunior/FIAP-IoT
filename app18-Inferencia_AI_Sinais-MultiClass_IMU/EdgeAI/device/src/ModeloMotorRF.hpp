// ATENCAO: MODELO SINTETICO, so para o projeto compilar de saida.
// Substitua pelo arquivo que o seu Colab gerar com o dataset do app17-7.
#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class RandomForest {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        uint8_t votes[4] = { 0 };
                        // tree #1
                        if (x[5] <= 0.05579634755849838) {
                            if (x[0] <= -0.7006284222006798) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[0] <= 0.7120689265429974) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #2
                        if (x[5] <= 0.009188920259475708) {
                            if (x[0] <= -0.6975399553775787) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[0] <= 0.7014402262866497) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #3
                        if (x[0] <= 0.7067155465483665) {
                            if (x[0] <= -0.7025812491774559) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[7] <= -0.32611432671546936) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #4
                        if (x[0] <= 0.7067155465483665) {
                            if (x[2] <= -0.012089192867279053) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[3] <= -0.023147299885749817) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #5
                        if (x[0] <= -0.7032546233385801) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[6] <= 0.09894181787967682) {
                                if (x[2] <= 0.004457056522369385) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        // tree #6
                        if (x[2] <= -0.05539490282535553) {
                            if (x[0] <= 0.006462812423706055) {
                                votes[2] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[4] <= 0.09816998243331909) {
                                votes[3] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        // tree #7
                        if (x[0] <= -0.7038557156920433) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[6] <= 0.09894181787967682) {
                                if (x[0] <= 0.69626759365201) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        // tree #8
                        if (x[0] <= -0.7025812491774559) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[3] <= -0.052130021154880524) {
                                if (x[2] <= 0.07648995518684387) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        // tree #9
                        if (x[4] <= 0.1544840931892395) {
                            if (x[0] <= -0.6984167248010635) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[0] <= 0.7015101127326488) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #10
                        if (x[0] <= -0.7025812491774559) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[0] <= 0.7067155465483665) {
                                if (x[5] <= -0.01081298291683197) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        // tree #11
                        if (x[0] <= 0.7014402262866497) {
                            if (x[2] <= -0.07194115221500397) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[4] <= 0.10105854272842407) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #12
                        if (x[0] <= 0.7067155465483665) {
                            if (x[2] <= -0.07194115221500397) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[3] <= -0.02958587557077408) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #13
                        if (x[5] <= 0.04785449802875519) {
                            if (x[2] <= -0.002442002296447754) {
                                if (x[0] <= 5.650520324707031e-05) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #14
                        if (x[0] <= -0.7025812491774559) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[6] <= 0.1347193717956543) {
                                if (x[0] <= 0.69626759365201) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        // tree #15
                        if (x[3] <= 0.03552490472793579) {
                            if (x[2] <= 0.010804712772369385) {
                                if (x[0] <= 5.650520324707031e-05) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            votes[0] += 1;
                        }

                        // return argmax of votes
                        uint8_t classIdx = 0;
                        float maxVotes = votes[0];

                        for (uint8_t i = 1; i < 4; i++) {
                            if (votes[i] > maxVotes) {
                                classIdx = i;
                                maxVotes = votes[i];
                            }
                        }

                        return classIdx;
                    }

                protected:
                };
            }
        }
    }