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
                        if (x[1] <= -0.7569425404071808) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[3] <= -0.17427348345518112) {
                                if (x[0] <= -0.019484274089336395) {
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

                        // tree #2
                        if (x[2] <= -0.927768886089325) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[7] <= 0.5150670446455479) {
                                if (x[0] <= -0.00021411478519439697) {
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

                        // tree #3
                        if (x[2] <= -0.9576519131660461) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[1] <= 0.8167544603347778) {
                                if (x[7] <= 0.4259333312511444) {
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

                        // tree #4
                        if (x[1] <= 0.7995972484350204) {
                            if (x[1] <= -0.7407385315746069) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[3] <= -0.1982610896229744) {
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
                        if (x[7] <= 0.5150670446455479) {
                            if (x[1] <= -0.7016582135111094) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[2] <= 0.46478094160556793) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #6
                        if (x[7] <= 0.398189727216959) {
                            if (x[0] <= -0.4948149845004082) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[0] <= -0.019484274089336395) {
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

                        // tree #7
                        if (x[2] <= -0.9606402218341827) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[6] <= 0.3108503520488739) {
                                if (x[0] <= -0.019484274089336395) {
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
                        if (x[3] <= -0.11757554858922958) {
                            if (x[1] <= 0.5784599371254444) {
                                if (x[2] <= 0.07032477855682373) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #9
                        if (x[1] <= -0.7569425404071808) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[7] <= 0.27127745747566223) {
                                if (x[0] <= -0.019484274089336395) {
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

                        // tree #10
                        if (x[1] <= -0.7474107593297958) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[5] <= 0.13322713971138) {
                                if (x[0] <= -0.019484274089336395) {
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

                        // tree #11
                        if (x[1] <= -0.7569425404071808) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[2] <= 0.1450323760509491) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[4] <= 0.23348727449774742) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        // tree #12
                        if (x[1] <= 0.8015036284923553) {
                            if (x[7] <= 0.335028737783432) {
                                if (x[1] <= -0.7026113918982446) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #13
                        if (x[2] <= -0.9576519131660461) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[5] <= 0.11063647270202637) {
                                if (x[0] <= -0.019484274089336395) {
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

                        // tree #14
                        if (x[1] <= -0.7607552707195282) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[2] <= 0.1450323760509491) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[5] <= 0.10542324185371399) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        // tree #15
                        if (x[1] <= 0.7938781976699829) {
                            if (x[7] <= 0.32735496759414673) {
                                if (x[0] <= -0.4370044991374016) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            votes[1] += 1;
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