#ifndef RANDOM_FOREST_MODEL_HPP
#define RANDOM_FOREST_MODEL_HPP

#include <string.h>

// Parâmetros do StandardScaler (extraídos do .pkl)
namespace Scaler {
    constexpr float means[5] = {
        20.9052161f,    // Temperature
        27.6315805f,    // Humidity  
        130.871497f,    // Light
        690.110664f,    // CO2
        0.00422451468f  // HumidityRatio
    };
    
    constexpr float scales[5] = {
        1.05449380f,    // Temperature
        4.97678295f,    // Humidity
        210.837059f,    // Light  
        311.663153f,    // CO2
        0.000768450930f // HumidityRatio
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
        double var3[2];
        double var4[2];
        double var5[2];
        double var6[2];
        double var7[2];
        double var8[2];
        double var9[2];
        double var10[2];
        if (input[0] <= 0.288085013628006) {
            if (input[1] <= -0.5358040630817413) {
                if (input[3] <= -0.3289684057235718) {
                    if (input[4] <= -1.0033597350120544) {
                        if (input[2] <= 0.968181312084198) {
                            memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            if (input[3] <= -0.6803092658519745) {
                                if (input[4] <= -1.9509366750717163) {
                                    memcpy(var10, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                } else {
                                    if (input[3] <= -0.7447485029697418) {
                                        memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[0] <= -0.37953390181064606) {
                                            memcpy(var10, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var10, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            } else {
                                memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        }
                    } else {
                        if (input[4] <= -0.9744640588760376) {
                            if (input[2] <= 0.3217104971408844) {
                                memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[0] <= -0.3242150843143463) {
                                if (input[0] <= -0.6023896336555481) {
                                    memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    if (input[2] <= 0.45751839876174927) {
                                        memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                if (input[3] <= -0.5883296132087708) {
                                    memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    if (input[2] <= 0.3183903992176056) {
                                        memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[2] <= 1.3039856553077698) {
                                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[2] <= 1.3215347528457642) {
                                                memcpy(var10, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var10, (double[]){0.4444444444444444, 0.5555555555555556}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    if (input[2] <= 0.35870590806007385) {
                        memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[0] <= -0.2420271560549736) {
                            if (input[3] <= -0.0007561482489109039) {
                                memcpy(var10, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                            } else {
                                memcpy(var10, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[0] <= 0.2005232498049736) {
                                memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            } else {
                                if (input[3] <= 0.18841710686683655) {
                                    memcpy(var10, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                } else {
                                    memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    }
                }
            } else {
                if (input[3] <= 0.4111468940973282) {
                    if (input[4] <= 1.689648449420929) {
                        if (input[1] <= -0.4501563459634781) {
                            if (input[2] <= 0.49862439930438995) {
                                memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                if (input[0] <= 0.07945410534739494) {
                                    memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[2] <= 1.5022430419921875) {
                                        memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[2] <= 1.5420842170715332) {
                                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var10, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            }
                        } else {
                            if (input[1] <= 1.1766476035118103) {
                                if (input[2] <= 0.7505962252616882) {
                                    memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    if (input[3] <= -0.6198807656764984) {
                                        if (input[4] <= -0.5095642805099487) {
                                            memcpy(var10, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[1] <= 0.025803725235164165) {
                                            memcpy(var10, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                        } else {
                                            if (input[1] <= 0.5233640968799591) {
                                                memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var10, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            } else {
                                if (input[3] <= -0.6284370124340057) {
                                    memcpy(var10, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                } else {
                                    if (input[2] <= 1.2582947611808777) {
                                        memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            }
                        }
                    } else {
                        if (input[1] <= 2.032622218132019) {
                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        } else {
                            memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= 0.35159143805503845) {
                        memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[4] <= -0.53626848757267) {
                            memcpy(var10, (double[]){0.5, 0.5}, 2 * sizeof(double));
                        } else {
                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[3] <= 0.2328026294708252) {
                if (input[2] <= 1.3523642420768738) {
                    if (input[1] <= 0.9917891919612885) {
                        if (input[3] <= 0.21622490882873535) {
                            if (input[2] <= 0.7654181122779846) {
                                memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                if (input[0] <= 0.6114629209041595) {
                                    memcpy(var10, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                } else {
                                    memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            if (input[3] <= 0.22103779762983322) {
                                memcpy(var10, (double[]){0.75, 0.25}, 2 * sizeof(double));
                            } else {
                                memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        }
                    } else {
                        memcpy(var10, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                    }
                } else {
                    if (input[3] <= 0.12157143652439117) {
                        if (input[2] <= 1.376790702342987) {
                            if (input[1] <= 0.26114653050899506) {
                                if (input[0] <= 1.138967216014862) {
                                    memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var10, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[4] <= 0.1012619137763977) {
                                if (input[2] <= 2.7823785543441772) {
                                    memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[1] <= -0.5413297116756439) {
                                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var10, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        }
                    } else {
                        if (input[2] <= 2.1188329458236694) {
                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        } else {
                            memcpy(var10, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[0] <= 2.7214800119400024) {
                    if (input[2] <= 0.8003977239131927) {
                        if (input[2] <= -0.4808049201965332) {
                            memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            if (input[3] <= 1.451934039592743) {
                                memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                if (input[2] <= -0.46776168048381805) {
                                    memcpy(var10, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                } else {
                                    memcpy(var10, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                }
                            }
                        }
                    } else {
                        if (input[3] <= 0.525854080915451) {
                            if (input[0] <= 1.2812155485153198) {
                                if (input[4] <= -1.2167494893074036) {
                                    if (input[2] <= 1.7193537950515747) {
                                        if (input[4] <= -1.2236355543136597) {
                                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var10, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[4] <= -0.14845724776387215) {
                                    memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[2] <= 1.709867775440216) {
                                        if (input[4] <= 0.03211924713104963) {
                                            if (input[2] <= 1.3831937313079834) {
                                                if (input[3] <= 0.38916802406311035) {
                                                    memcpy(var10, (double[]){0.4444444444444444, 0.5555555555555556}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[4] <= 0.44758976995944977) {
                                                if (input[1] <= -0.49260345101356506) {
                                                    memcpy(var10, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            }
                        } else {
                            if (input[2] <= 1.8634223937988281) {
                                if (input[2] <= 1.3124439716339111) {
                                    memcpy(var10, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                } else {
                                    if (input[1] <= -0.19639469683170319) {
                                        if (input[4] <= 0.15248912572860718) {
                                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[2] <= 1.595838189125061) {
                                                if (input[0] <= 1.3369295001029968) {
                                                    if (input[1] <= -0.29317742586135864) {
                                                        memcpy(var10, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[1] <= -0.20979022979736328) {
                                                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            if (input[3] <= 1.2083483934402466) {
                                                                memcpy(var10, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                                            } else {
                                                                memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            }
                                                        }
                                                    }
                                                } else {
                                                    memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[2] <= 1.6333869695663452) {
                                                    memcpy(var10, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    } else {
                                        if (input[0] <= 0.9425854682922363) {
                                            if (input[2] <= 1.4515719413757324) {
                                                if (input[0] <= 0.8864763975143433) {
                                                    memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[1] <= 0.7280645966529846) {
                                                        if (input[3] <= 0.9181044697761536) {
                                                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var10, (double[]){0.125, 0.875}, 2 * sizeof(double));
                                                        }
                                                    } else {
                                                        memcpy(var10, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                                    }
                                                }
                                            } else {
                                                if (input[3] <= 0.6107480525970459) {
                                                    if (input[3] <= 0.5935018956661224) {
                                                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var10, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    if (input[4] <= 1.4261949062347412) {
                                                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[1] <= 1.1241537928581238) {
                                                            memcpy(var10, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    }
                                                }
                                            }
                                        } else {
                                            memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            } else {
                                if (input[4] <= 0.474552646279335) {
                                    if (input[1] <= -0.4031882584095001) {
                                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[1] <= -0.256172277033329) {
                                            if (input[1] <= -0.2976984232664108) {
                                                if (input[4] <= 0.37604551017284393) {
                                                    if (input[4] <= 0.11254942417144775) {
                                                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[3] <= 1.201984703540802) {
                                                memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                if (input[2] <= 1.9480849504470825) {
                                                    memcpy(var10, (double[]){0.7, 0.3}, 2 * sizeof(double));
                                                } else {
                                                    if (input[2] <= 2.1242873668670654) {
                                                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var10, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                                    }
                                                }
                                            }
                                        }
                                    }
                                } else {
                                    memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    }
                } else {
                    if (input[2] <= 1.7359543442726135) {
                        if (input[1] <= -0.9123022854328156) {
                            memcpy(var10, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            memcpy(var10, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                        }
                    } else {
                        memcpy(var10, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    }
                }
            }
        }
        double var11[2];
        if (input[2] <= 1.220350742340088) {
            if (input[2] <= 0.749410480260849) {
                if (input[1] <= 1.03197580575943) {
                    memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                } else {
                    if (input[3] <= -0.3645303100347519) {
                        memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[2] <= -0.4843621701002121) {
                            if (input[4] <= 1.192364752292633) {
                                if (input[4] <= 1.17365962266922) {
                                    memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var11, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[2] <= -0.4535326808691025) {
                                memcpy(var11, (double[]){0.5, 0.5}, 2 * sizeof(double));
                            } else {
                                memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        }
                    }
                }
            } else {
                if (input[3] <= -0.7087855041027069) {
                    memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                } else {
                    if (input[0] <= 0.5118890851736069) {
                        if (input[4] <= 0.0546160489320755) {
                            memcpy(var11, (double[]){0.2, 0.8}, 2 * sizeof(double));
                        } else {
                            memcpy(var11, (double[]){0.75, 0.25}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[4] <= -0.11429892852902412) {
                            memcpy(var11, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                        } else {
                            if (input[1] <= -0.43341192603111267) {
                                if (input[2] <= 0.8341915905475616) {
                                    memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var11, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        }
                    }
                }
            }
        } else {
            if (input[0] <= -0.24992984533309937) {
                if (input[4] <= -0.44835537672042847) {
                    if (input[3] <= -0.12629232183098793) {
                        if (input[3] <= -0.5066709369421005) {
                            if (input[4] <= -0.8200981616973877) {
                                if (input[3] <= -0.7491603195667267) {
                                    memcpy(var11, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                } else {
                                    if (input[4] <= -1.0549672842025757) {
                                        if (input[0] <= -0.571569174528122) {
                                            if (input[3] <= -0.7323152124881744) {
                                                memcpy(var11, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var11, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                if (input[4] <= -0.668739914894104) {
                                    memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var11, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[0] <= -0.3131513446569443) {
                            memcpy(var11, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                        } else {
                            memcpy(var11, (double[]){0.8571428571428571, 0.14285714285714285}, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= -0.6514319181442261) {
                        memcpy(var11, (double[]){0.25, 0.75}, 2 * sizeof(double));
                    } else {
                        if (input[2] <= 1.3523642420768738) {
                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        } else {
                            if (input[1] <= 0.6093466877937317) {
                                memcpy(var11, (double[]){0.2857142857142857, 0.7142857142857143}, 2 * sizeof(double));
                            } else {
                                memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        }
                    }
                }
            } else {
                if (input[1] <= -0.19304580986499786) {
                    if (input[3] <= -0.7206840515136719) {
                        memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[0] <= 1.2385410070419312) {
                            if (input[3] <= -0.6426082849502563) {
                                memcpy(var11, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                            } else {
                                if (input[4] <= 0.16815128922462463) {
                                    if (input[1] <= -1.6618661880493164) {
                                        if (input[3] <= -0.13672025501728058) {
                                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var11, (double[]){0.7142857142857143, 0.2857142857142857}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[0] <= 0.2866625338792801) {
                                            if (input[3] <= 0.06702536344528198) {
                                                memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                if (input[3] <= 0.09189195558428764) {
                                                    memcpy(var11, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                                } else {
                                                    if (input[2] <= 1.5760756731033325) {
                                                        if (input[0] <= 0.20842593908309937) {
                                                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            if (input[2] <= 1.5073022246360779) {
                                                                memcpy(var11, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                            } else {
                                                                memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            }
                                                        }
                                                    } else {
                                                        memcpy(var11, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                                    }
                                                }
                                            }
                                        } else {
                                            if (input[3] <= 0.4047297090291977) {
                                                if (input[1] <= -1.4013230204582214) {
                                                    if (input[4] <= -1.2265783548355103) {
                                                        memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[0] <= 0.7857171297073364) {
                                                            memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    }
                                                } else {
                                                    memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                } else {
                                    if (input[3] <= 1.13789302110672) {
                                        if (input[1] <= -0.21757638454437256) {
                                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[3] <= 1.1154329180717468) {
                                                memcpy(var11, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var11, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            }
                        } else {
                            if (input[0] <= 1.3250754475593567) {
                                if (input[3] <= 0.8624888956546783) {
                                    if (input[1] <= -0.3532899022102356) {
                                        memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[3] <= 0.2892203778028488) {
                                            memcpy(var11, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        }
                                    }
                                } else {
                                    memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[4] <= 0.04722852073609829) {
                                    if (input[0] <= 1.555043637752533) {
                                        if (input[1] <= -0.5197294056415558) {
                                            if (input[1] <= -0.561757892370224) {
                                                memcpy(var11, (double[]){0.42857142857142855, 0.5714285714285714}, 2 * sizeof(double));
                                            } else {
                                                if (input[2] <= 1.389833927154541) {
                                                    memcpy(var11, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        } else {
                                            if (input[4] <= 0.017338985111564398) {
                                                memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var11, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        if (input[3] <= 0.005259960889816284) {
                                            if (input[4] <= 0.0034589427523314953) {
                                                memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var11, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[2] <= 1.4210191369056702) {
                                                memcpy(var11, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                } else {
                                    if (input[4] <= 0.42497827112674713) {
                                        if (input[4] <= 0.362369567155838) {
                                            if (input[2] <= 1.4211377501487732) {
                                                memcpy(var11, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                            } else {
                                                if (input[0] <= 2.753485918045044) {
                                                    if (input[2] <= 2.7438416481018066) {
                                                        memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[0] <= 1.7468417882919312) {
                                                            memcpy(var11, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    }
                                                } else {
                                                    if (input[3] <= 0.11782807856798172) {
                                                        memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var11, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                    }
                                                }
                                            }
                                        } else {
                                            if (input[4] <= 0.3828900158405304) {
                                                memcpy(var11, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                                            } else {
                                                if (input[2] <= 2.097015142440796) {
                                                    memcpy(var11, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                                } else {
                                                    if (input[1] <= -0.28279589116573334) {
                                                        memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var11, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        if (input[4] <= 0.5788829922676086) {
                                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[4] <= 0.6064951717853546) {
                                                if (input[2] <= 2.428234577178955) {
                                                    memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var11, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[1] <= -0.32557788491249084) {
                                                    if (input[4] <= 0.7159104943275452) {
                                                        memcpy(var11, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    if (input[3] <= 1.4431104063987732) {
                        if (input[3] <= 1.435260832309723) {
                            if (input[2] <= 1.3523642420768738) {
                                if (input[1] <= 0.4246557652950287) {
                                    memcpy(var11, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                } else {
                                    memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[2] <= 2.1598598957061768) {
                                    if (input[3] <= 0.8892271518707275) {
                                        if (input[3] <= -0.19554444402456284) {
                                            if (input[4] <= 0.7553195655345917) {
                                                if (input[3] <= -0.21212216466665268) {
                                                    memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var11, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[4] <= 1.0849720239639282) {
                                            if (input[4] <= 0.3176381587982178) {
                                                if (input[3] <= 0.8928368091583252) {
                                                    memcpy(var11, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[0] <= 0.898330420255661) {
                                                memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                if (input[2] <= 1.4525600671768188) {
                                                    memcpy(var11, (double[]){0.7777777777777778, 0.2222222222222222}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    }
                                } else {
                                    if (input[3] <= 1.1980541348457336) {
                                        memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[1] <= -0.10371434316039085) {
                                            memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var11, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            }
                        } else {
                            memcpy(var11, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                        }
                    } else {
                        memcpy(var11, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    }
                }
            }
        }
        add_vectors(var10, var11, 2, var9);
        double var12[2];
        if (input[2] <= 1.220350742340088) {
            if (input[4] <= 1.3675192594528198) {
                if (input[2] <= 0.749410480260849) {
                    if (input[0] <= 3.233574151992798) {
                        memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        memcpy(var12, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                    }
                } else {
                    if (input[0] <= 0.4644729644060135) {
                        memcpy(var12, (double[]){0.4, 0.6}, 2 * sizeof(double));
                    } else {
                        if (input[3] <= 0.33186791837215424) {
                            if (input[0] <= 0.7750485241413116) {
                                if (input[1] <= -1.6841362118721008) {
                                    memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var12, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[0] <= 2.6385018825531006) {
                                memcpy(var12, (double[]){0.2, 0.8}, 2 * sizeof(double));
                            } else {
                                memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        }
                    }
                }
            } else {
                if (input[4] <= 1.3698520064353943) {
                    memcpy(var12, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                } else {
                    if (input[1] <= 1.086981177330017) {
                        memcpy(var12, (double[]){0.75, 0.25}, 2 * sizeof(double));
                    } else {
                        memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    }
                }
            }
        } else {
            if (input[0] <= -0.18851468712091446) {
                if (input[2] <= 1.4786465764045715) {
                    if (input[3] <= -0.6888387799263) {
                        if (input[0] <= -0.32856157422065735) {
                            if (input[1] <= -1.7710397839546204) {
                                memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                if (input[1] <= -0.9450376331806183) {
                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[3] <= -0.7113791108131409) {
                                        memcpy(var12, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var12, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                    }
                                }
                            }
                        } else {
                            memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 1.2631958723068237) {
                            if (input[1] <= 0.00487320264801383) {
                                memcpy(var12, (double[]){0.75, 0.25}, 2 * sizeof(double));
                            } else {
                                memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[3] <= 0.28520963340997696) {
                                if (input[0] <= -0.3131513446569443) {
                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[3] <= -0.520253986120224) {
                                        memcpy(var12, (double[]){0.2857142857142857, 0.7142857142857143}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                if (input[3] <= 0.4270561635494232) {
                                    memcpy(var12, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                } else {
                                    if (input[4] <= -0.45463891327381134) {
                                        memcpy(var12, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            }
                        }
                    }
                } else {
                    memcpy(var12, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                }
            } else {
                if (input[1] <= -0.148338221013546) {
                    if (input[2] <= 1.4124422669410706) {
                        if (input[1] <= -0.5689579844474792) {
                            if (input[3] <= -0.019071863149292767) {
                                memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            } else {
                                memcpy(var12, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[1] <= -0.5378636121749878) {
                                if (input[3] <= 0.3918953388929367) {
                                    memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var12, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[4] <= -0.07092812284827232) {
                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var12, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                                }
                            }
                        }
                    } else {
                        if (input[1] <= -0.494445338845253) {
                            if (input[1] <= -1.6889083981513977) {
                                if (input[4] <= -1.5128463506698608) {
                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var12, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[0] <= 0.7786047160625458) {
                                    if (input[3] <= 0.4035264849662781) {
                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[3] <= 0.4127511829137802) {
                                            memcpy(var12, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    }
                                } else {
                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            if (input[0] <= 0.10173968225717545) {
                                if (input[0] <= 0.07684621959924698) {
                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[4] <= 0.09377220273017883) {
                                    if (input[4] <= -0.37087956070899963) {
                                        if (input[1] <= -0.4852693974971771) {
                                            memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var12, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[4] <= 0.07175405323505402) {
                                            memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[4] <= 0.07873331010341644) {
                                                memcpy(var12, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                } else {
                                    if (input[4] <= 0.10069294273853302) {
                                        if (input[4] <= 0.09866200014948845) {
                                            memcpy(var12, (double[]){0.8888888888888888, 0.1111111111111111}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var12, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[4] <= 0.17620887607336044) {
                                            if (input[0] <= 1.2598782777786255) {
                                                if (input[1] <= -0.22234854847192764) {
                                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var12, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[1] <= -0.3532899022102356) {
                                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[2] <= 1.5053260326385498) {
                                                        if (input[4] <= 0.1567005142569542) {
                                                            memcpy(var12, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    } else {
                                                        memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                    }
                                                }
                                            }
                                        } else {
                                            if (input[1] <= -0.15101732313632965) {
                                                if (input[2] <= 2.577006697654724) {
                                                    if (input[1] <= -0.29016344249248505) {
                                                        if (input[0] <= 2.6823617219924927) {
                                                            if (input[1] <= -0.3018845319747925) {
                                                                if (input[1] <= -0.39515092968940735) {
                                                                    if (input[1] <= -0.42328155040740967) {
                                                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    } else {
                                                                        memcpy(var12, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                                                    }
                                                                } else {
                                                                    if (input[3] <= 0.7363377213478088) {
                                                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    } else {
                                                                        if (input[2] <= 1.983183205127716) {
                                                                            memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                        } else {
                                                                            memcpy(var12, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                                        }
                                                                    }
                                                                }
                                                            } else {
                                                                memcpy(var12, (double[]){0.5714285714285714, 0.42857142857142855}, 2 * sizeof(double));
                                                            }
                                                        } else {
                                                            memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                        }
                                                    } else {
                                                        if (input[2] <= 1.5491987466812134) {
                                                            memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            if (input[3] <= 1.1178393363952637) {
                                                                if (input[3] <= 1.0419026613235474) {
                                                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                } else {
                                                                    if (input[4] <= 0.4151792675256729) {
                                                                        memcpy(var12, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                                    } else {
                                                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    }
                                                                }
                                                            } else {
                                                                if (input[0] <= 1.6309093832969666) {
                                                                    if (input[0] <= 1.180456280708313) {
                                                                        memcpy(var12, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                                    } else {
                                                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    }
                                                                } else {
                                                                    memcpy(var12, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                                }
                                                            }
                                                        }
                                                    }
                                                } else {
                                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[4] <= 0.469703733921051) {
                                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var12, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    if (input[4] <= 0.7633571624755859) {
                        if (input[2] <= 1.3571072816848755) {
                            if (input[3] <= 0.45005108416080475) {
                                memcpy(var12, (double[]){0.625, 0.375}, 2 * sizeof(double));
                            } else {
                                memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[0] <= 1.2854830026626587) {
                                if (input[3] <= 0.6562031507492065) {
                                    if (input[3] <= 0.6531282365322113) {
                                        if (input[3] <= 0.5876194536685944) {
                                            memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[4] <= 0.33876702189445496) {
                                                memcpy(var12, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        memcpy(var12, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[2] <= 2.0946435928344727) {
                                    if (input[0] <= 1.2940179109573364) {
                                        memcpy(var12, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[4] <= 0.418435275554657) {
                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[1] <= -0.07451208308339119) {
                                            memcpy(var12, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var12, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        if (input[2] <= 1.451374351978302) {
                            if (input[4] <= 1.087064266204834) {
                                memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            } else {
                                if (input[2] <= 1.4365525245666504) {
                                    if (input[1] <= 0.7074689865112305) {
                                        if (input[1] <= 0.6868733465671539) {
                                            memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var12, (double[]){0.125, 0.875}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[4] <= 1.1796608567237854) {
                                        if (input[0] <= 0.8864763975143433) {
                                            memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var12, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            }
                        } else {
                            if (input[1] <= 1.116116464138031) {
                                memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            } else {
                                if (input[3] <= 1.4923088550567627) {
                                    memcpy(var12, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                } else {
                                    memcpy(var12, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    }
                }
            }
        }
        add_vectors(var9, var12, 2, var8);
        double var13[2];
        if (input[3] <= 0.3028568923473358) {
            if (input[2] <= 1.220350742340088) {
                if (input[2] <= 1.1395221948623657) {
                    if (input[0] <= -1.3049384355545044) {
                        if (input[0] <= -1.3207437992095947) {
                            memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            memcpy(var13, (double[]){0.75, 0.25}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.45894716680049896) {
                            if (input[0] <= 0.3563642203807831) {
                                memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                if (input[2] <= 0.9266800880432129) {
                                    memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    if (input[0] <= 0.9860502481460571) {
                                        memcpy(var13, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    }
                                }
                            }
                        } else {
                            memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var13, (double[]){0.5555555555555556, 0.4444444444444444}, 2 * sizeof(double));
                }
            } else {
                if (input[3] <= -0.7408714592456818) {
                    if (input[2] <= 1.3748934864997864) {
                        if (input[3] <= -0.7491603195667267) {
                            memcpy(var13, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                        } else {
                            memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                        }
                    } else {
                        memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    }
                } else {
                    if (input[0] <= -1.2654249668121338) {
                        memcpy(var13, (double[]){0.625, 0.375}, 2 * sizeof(double));
                    } else {
                        if (input[3] <= -0.6888387799263) {
                            if (input[2] <= 1.3776602745056152) {
                                if (input[3] <= -0.7048015296459198) {
                                    memcpy(var13, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                } else {
                                    memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[3] <= -0.1995551809668541) {
                                if (input[2] <= 2.475269317626953) {
                                    if (input[3] <= -0.5349867641925812) {
                                        if (input[3] <= -0.550628662109375) {
                                            memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var13, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[3] <= -0.3797711133956909) {
                                        memcpy(var13, (double[]){0.5714285714285714, 0.42857142857142855}, 2 * sizeof(double));
                                    } else {
                                        if (input[1] <= -0.5498693585395813) {
                                            memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[0] <= 1.7790847420692444) {
                                                if (input[0] <= 1.55978524684906) {
                                                    memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            } else {
                                if (input[2] <= 1.3559215068817139) {
                                    memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                } else {
                                    if (input[0] <= -0.27758924663066864) {
                                        memcpy(var13, (double[]){0.45454545454545453, 0.5454545454545454}, 2 * sizeof(double));
                                    } else {
                                        if (input[3] <= 0.2727763503789902) {
                                            if (input[1] <= -1.6618661880493164) {
                                                if (input[4] <= -1.4869932532310486) {
                                                    if (input[3] <= -0.14180052280426025) {
                                                        memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var13, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[1] <= -0.7854633033275604) {
                                                    memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[1] <= -0.7643653750419617) {
                                                        memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[0] <= 0.2479393631219864) {
                                                            if (input[0] <= 0.07945410534739494) {
                                                                memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            } else {
                                                                if (input[1] <= -0.46723565459251404) {
                                                                    if (input[0] <= 0.1768151968717575) {
                                                                        memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    } else {
                                                                        memcpy(var13, (double[]){0.14285714285714285, 0.8571428571428571}, 2 * sizeof(double));
                                                                    }
                                                                } else {
                                                                    memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                                }
                                                            }
                                                        } else {
                                                            if (input[2] <= 1.3784507513046265) {
                                                                if (input[4] <= 0.7299815118312836) {
                                                                    memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                } else {
                                                                    if (input[4] <= 0.7665141522884369) {
                                                                        memcpy(var13, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                                                    } else {
                                                                        memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    }
                                                                }
                                                            } else {
                                                                memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        } else {
                                            if (input[0] <= 1.2385410070419312) {
                                                if (input[3] <= 0.2815999686717987) {
                                                    memcpy(var13, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var13, (double[]){0.7142857142857143, 0.2857142857142857}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {
            if (input[2] <= 0.9895248115062714) {
                if (input[4] <= 2.477278470993042) {
                    if (input[1] <= -0.8492595553398132) {
                        if (input[1] <= -0.9294652938842773) {
                            if (input[3] <= 0.33935463428497314) {
                                memcpy(var13, (double[]){0.8888888888888888, 0.1111111111111111}, 2 * sizeof(double));
                            } else {
                                memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        } else {
                            memcpy(var13, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[4] <= 1.3656030297279358) {
                            if (input[0] <= 1.392248272895813) {
                                memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                if (input[4] <= 0.21890980750322342) {
                                    memcpy(var13, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                } else {
                                    memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            if (input[1] <= 1.1241537928581238) {
                                if (input[0] <= 0.741857260465622) {
                                    memcpy(var13, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                } else {
                                    if (input[3] <= 1.451934039592743) {
                                        memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var13, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        }
                    }
                } else {
                    memcpy(var13, (double[]){0.75, 0.25}, 2 * sizeof(double));
                }
            } else {
                if (input[1] <= -0.14867310225963593) {
                    if (input[2] <= 1.3950512409210205) {
                        if (input[1] <= -0.4840637892484665) {
                            if (input[3] <= 0.4729122966527939) {
                                memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                memcpy(var13, (double[]){0.4444444444444444, 0.5555555555555556}, 2 * sizeof(double));
                            }
                        } else {
                            memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.3532899022102356) {
                            if (input[4] <= 0.33764541149139404) {
                                if (input[1] <= -1.42710942029953) {
                                    if (input[3] <= 0.3646222949028015) {
                                        memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[1] <= -1.4334722757339478) {
                                            memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                        }
                                    }
                                } else {
                                    memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[0] <= 2.18472957611084) {
                                    if (input[2] <= 2.318038821220398) {
                                        memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var13, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            if (input[3] <= 0.6497859358787537) {
                                if (input[2] <= 1.483982503414154) {
                                    memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[4] <= 0.4566204100847244) {
                                        if (input[1] <= -0.2725818008184433) {
                                            if (input[2] <= 1.8802601099014282) {
                                                if (input[4] <= 0.1043265201151371) {
                                                    memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var13, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var13, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                if (input[0] <= 1.6467147469520569) {
                                    if (input[2] <= 2.24894917011261) {
                                        if (input[1] <= -0.21757639199495316) {
                                            memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[2] <= 1.5444557666778564) {
                                                memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                if (input[4] <= 0.1371014006435871) {
                                                    memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[3] <= 1.1711822152137756) {
                                                        memcpy(var13, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        if (input[4] <= 0.39978280663490295) {
                                            memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    }
                                } else {
                                    if (input[4] <= 0.41041307151317596) {
                                        if (input[0] <= 1.7763188481330872) {
                                            if (input[1] <= -0.22536253929138184) {
                                                memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var13, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var13, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[2] <= 1.941444754600525) {
                                            memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                        } else {
                                            if (input[2] <= 1.9683613181114197) {
                                                memcpy(var13, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    if (input[2] <= 2.097805619239807) {
                        if (input[3] <= 0.3991146683692932) {
                            if (input[0] <= 0.9220384657382965) {
                                memcpy(var13, (double[]){0.4, 0.6}, 2 * sizeof(double));
                            } else {
                                memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[0] <= 0.9443240463733673) {
                                if (input[0] <= 0.9244092702865601) {
                                    if (input[3] <= 0.6570052802562714) {
                                        if (input[2] <= 1.5740994215011597) {
                                            memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[1] <= 0.08633479848504066) {
                                                memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var13, (double[]){0.125, 0.875}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[4] <= 1.1057161688804626) {
                                        memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[4] <= 1.1956608295440674) {
                                            memcpy(var13, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            } else {
                                memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        }
                    } else {
                        if (input[1] <= -0.10346318036317825) {
                            memcpy(var13, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        } else {
                            if (input[3] <= 1.1961289644241333) {
                                memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                            } else {
                                memcpy(var13, (double[]){0.5, 0.5}, 2 * sizeof(double));
                            }
                        }
                    }
                }
            }
        }
        add_vectors(var8, var13, 2, var7);
        double var14[2];
        if (input[4] <= 0.22091792523860931) {
            if (input[2] <= 1.1395221948623657) {
                if (input[3] <= -0.6943468451499939) {
                    memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                } else {
                    if (input[3] <= -0.6930099427700043) {
                        if (input[0] <= -0.6055507361888885) {
                            memcpy(var14, (double[]){0.4, 0.6}, 2 * sizeof(double));
                        } else {
                            memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.8365630805492401) {
                            if (input[0] <= 1.396199643611908) {
                                memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                if (input[0] <= 1.4341325163841248) {
                                    memcpy(var14, (double[]){0.8461538461538461, 0.15384615384615385}, 2 * sizeof(double));
                                } else {
                                    memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            memcpy(var14, (double[]){0.75, 0.25}, 2 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[1] <= -1.746090590953827) {
                    if (input[4] <= -1.8520514965057373) {
                        if (input[3] <= -0.7491603195667267) {
                            memcpy(var14, (double[]){0.8571428571428571, 0.14285714285714285}, 2 * sizeof(double));
                        } else {
                            memcpy(var14, (double[]){0.125, 0.875}, 2 * sizeof(double));
                        }
                    } else {
                        memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    }
                } else {
                    if (input[2] <= 1.3895177841186523) {
                        if (input[0] <= 1.4210931062698364) {
                            if (input[0] <= -1.2654249668121338) {
                                memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                if (input[0] <= 1.0808824598789215) {
                                    if (input[0] <= -0.18394241482019424) {
                                        if (input[4] <= -0.5536623150110245) {
                                            if (input[3] <= -0.6913788914680481) {
                                                if (input[3] <= -0.7233578860759735) {
                                                    memcpy(var14, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[4] <= -0.6483281254768372) {
                                                    if (input[0] <= -0.308409720659256) {
                                                        memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[0] <= -0.27363790571689606) {
                                                            memcpy(var14, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    }
                                                } else {
                                                    memcpy(var14, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                                                }
                                            }
                                        } else {
                                            memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var14, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[4] <= 0.15301817655563354) {
                            if (input[3] <= 0.4047297090291977) {
                                if (input[2] <= 1.48457533121109) {
                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[4] <= 0.07941366732120514) {
                                        if (input[2] <= 1.7063105702400208) {
                                            if (input[0] <= 0.10173968225717545) {
                                                if (input[0] <= 0.07684621959924698) {
                                                    if (input[3] <= 0.05579528957605362) {
                                                        memcpy(var14, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    if (input[1] <= -0.46045416593551636) {
                                                        memcpy(var14, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                    }
                                                }
                                            } else {
                                                if (input[1] <= -0.47728231549263) {
                                                    if (input[3] <= 0.39416809380054474) {
                                                        memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var14, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var14, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                }
                                            }
                                        } else {
                                            if (input[4] <= -1.4869932532310486) {
                                                if (input[3] <= -0.14180052280426025) {
                                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[3] <= -0.10824399068951607) {
                                                        memcpy(var14, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                }
                                            } else {
                                                if (input[1] <= -0.5498693585395813) {
                                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[1] <= -0.537059873342514) {
                                                        memcpy(var14, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[3] <= -0.48378297686576843) {
                                                            memcpy(var14, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        if (input[0] <= 1.766045331954956) {
                                            if (input[4] <= 0.11418444663286209) {
                                                if (input[3] <= 0.3205041438341141) {
                                                    memcpy(var14, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var14, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            } else {
                                if (input[4] <= 0.13729224354028702) {
                                    if (input[0] <= 1.5313355922698975) {
                                        memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[4] <= 0.031917073763906956) {
                                            memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[3] <= 0.525854080915451) {
                                                memcpy(var14, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                } else {
                                    if (input[3] <= 0.9938807189464569) {
                                        memcpy(var14, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            }
                        } else {
                            if (input[1] <= -0.2670561522245407) {
                                if (input[2] <= 1.5290409326553345) {
                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[0] <= 1.3487834930419922) {
                                        memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var14, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                if (input[2] <= 1.546234369277954) {
                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[2] <= 1.558684766292572) {
                                        memcpy(var14, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                    } else {
                                        if (input[4] <= 0.17354924976825714) {
                                            memcpy(var14, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } else {
            if (input[0] <= 0.25189071148633957) {
                if (input[2] <= 0.7104783654212952) {
                    memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                } else {
                    if (input[2] <= 1.2582947611808777) {
                        memcpy(var14, (double[]){0.4, 0.6}, 2 * sizeof(double));
                    } else {
                        if (input[1] <= 0.6233282685279846) {
                            if (input[4] <= 0.4205272048711777) {
                                memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            } else {
                                memcpy(var14, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                            }
                        } else {
                            memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[2] <= 1.262642502784729) {
                    if (input[0] <= 1.8031879663467407) {
                        if (input[0] <= 0.37675315141677856) {
                            if (input[0] <= 0.35423049330711365) {
                                memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                if (input[4] <= 1.1696891784667969) {
                                    memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    if (input[3] <= 1.925869882106781) {
                                        memcpy(var14, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    }
                                }
                            }
                        } else {
                            memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 0.9895248115062714) {
                            memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            memcpy(var14, (double[]){0.75, 0.25}, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= -0.2106274515390396) {
                        if (input[0] <= 1.9135094285011292) {
                            if (input[2] <= 1.7282469272613525) {
                                memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            } else {
                                if (input[4] <= 0.3596855103969574) {
                                    memcpy(var14, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                } else {
                                    if (input[2] <= 2.309264302253723) {
                                        memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var14, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                    }
                                }
                            }
                        } else {
                            if (input[3] <= -0.3565088212490082) {
                                if (input[0] <= 2.6610244512557983) {
                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[2] <= 2.5191420316696167) {
                                        memcpy(var14, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var14, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                if (input[4] <= 0.38162197172641754) {
                                    if (input[4] <= 0.33764541149139404) {
                                        if (input[0] <= 2.753485918045044) {
                                            memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[3] <= 0.11782807856798172) {
                                                memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var14, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        memcpy(var14, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    } else {
                        if (input[1] <= 0.26206745207309723) {
                            if (input[0] <= 0.5249285101890564) {
                                if (input[3] <= 0.7189043760299683) {
                                    memcpy(var14, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                } else {
                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[2] <= 1.376790702342987) {
                                    if (input[4] <= 0.6749118268489838) {
                                        memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var14, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[0] <= 0.5917061865329742) {
                                        if (input[2] <= 1.5474201440811157) {
                                            if (input[4] <= 0.31979936361312866) {
                                                if (input[3] <= 0.8726494312286377) {
                                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[4] <= 0.2553883194923401) {
                                                        memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var14, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                                    }
                                                }
                                            } else {
                                                memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[1] <= 0.09160928800702095) {
                                                memcpy(var14, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                            } else {
                                                if (input[3] <= 0.5922986567020416) {
                                                    memcpy(var14, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    } else {
                                        if (input[2] <= 1.9166964292526245) {
                                            memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[2] <= 1.9319587349891663) {
                                                if (input[3] <= 1.4099129438400269) {
                                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var14, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[1] <= -0.06059747189283371) {
                                                    if (input[3] <= 1.28121018409729) {
                                                        if (input[1] <= -0.0722515881061554) {
                                                            memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var14, (double[]){0.125, 0.875}, 2 * sizeof(double));
                                                        }
                                                    } else {
                                                        memcpy(var14, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        } else {
                            if (input[1] <= 1.1211398243904114) {
                                if (input[3] <= 0.9209119975566864) {
                                    memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[0] <= 0.946931928396225) {
                                        if (input[0] <= 0.8864763975143433) {
                                            if (input[4] <= 1.4238151907920837) {
                                                memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var14, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[3] <= 1.1775993704795837) {
                                                memcpy(var14, (double[]){0.42857142857142855, 0.5714285714285714}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                memcpy(var14, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        }
                    }
                }
            }
        }
        add_vectors(var7, var14, 2, var6);
        double var15[2];
        if (input[2] <= 1.159087061882019) {
            if (input[3] <= 0.30553071200847626) {
                if (input[0] <= 0.35423049330711365) {
                    memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                } else {
                    if (input[4] <= 1.1717307567596436) {
                        if (input[0] <= 0.37675315141677856) {
                            if (input[2] <= 0.6292545199394226) {
                                memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            } else {
                                memcpy(var15, (double[]){0.25, 0.75}, 2 * sizeof(double));
                            }
                        } else {
                            memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        }
                    } else {
                        memcpy(var15, (double[]){0.75, 0.25}, 2 * sizeof(double));
                    }
                }
            } else {
                if (input[2] <= 0.4974386692047119) {
                    if (input[1] <= 1.0470457673072815) {
                        memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[2] <= -0.4843621701002121) {
                            memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            if (input[4] <= 1.3866872191429138) {
                                memcpy(var15, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                            } else {
                                memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        }
                    }
                } else {
                    if (input[4] <= 0.09133711084723473) {
                        if (input[0] <= 2.6610244512557983) {
                            memcpy(var15, (double[]){0.75, 0.25}, 2 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        }
                    } else {
                        memcpy(var15, (double[]){0.25, 0.75}, 2 * sizeof(double));
                    }
                }
            }
        } else {
            if (input[1] <= -1.746090590953827) {
                if (input[2] <= 1.3476212620735168) {
                    memcpy(var15, (double[]){0.2, 0.8}, 2 * sizeof(double));
                } else {
                    memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                }
            } else {
                if (input[4] <= 0.48632363975048065) {
                    if (input[3] <= -0.7118604183197021) {
                        memcpy(var15, (double[]){0.8888888888888888, 0.1111111111111111}, 2 * sizeof(double));
                    } else {
                        if (input[0] <= 1.2385410070419312) {
                            if (input[0] <= -0.24518822878599167) {
                                if (input[0] <= -0.3487134277820587) {
                                    if (input[2] <= 1.2631958723068237) {
                                        if (input[4] <= -0.3482774794101715) {
                                            memcpy(var15, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[1] <= -0.6704291701316833) {
                                        memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[1] <= -0.49863143265247345) {
                                            memcpy(var15, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var15, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            } else {
                                if (input[1] <= -1.6618661880493164) {
                                    if (input[2] <= 1.9148049354553223) {
                                        memcpy(var15, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[4] <= -0.4436533451080322) {
                                        if (input[1] <= -1.4304583072662354) {
                                            if (input[1] <= -1.4334722757339478) {
                                                memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var15, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[0] <= 0.10173968225717545) {
                                            if (input[1] <= -0.4102209061384201) {
                                                memcpy(var15, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[3] <= -0.4672052562236786) {
                                                memcpy(var15, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                            } else {
                                                if (input[3] <= 1.1095504760742188) {
                                                    if (input[3] <= 0.588020533323288) {
                                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[3] <= 0.6225129067897797) {
                                                            memcpy(var15, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                        } else {
                                                            if (input[1] <= 0.10728206112980843) {
                                                                memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            } else {
                                                                if (input[3] <= 0.9004572331905365) {
                                                                    if (input[4] <= 0.3294130563735962) {
                                                                        memcpy(var15, (double[]){0.14285714285714285, 0.8571428571428571}, 2 * sizeof(double));
                                                                    } else {
                                                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    }
                                                                } else {
                                                                    memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                }
                                                            }
                                                        }
                                                    }
                                                } else {
                                                    if (input[1] <= -0.20979022979736328) {
                                                        if (input[1] <= -0.22008804976940155) {
                                                            memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var15, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                        }
                                                    } else {
                                                        if (input[1] <= -0.2047669067978859) {
                                                            if (input[3] <= 1.2120917439460754) {
                                                                memcpy(var15, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                            } else {
                                                                memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            }
                                                        } else {
                                                            if (input[1] <= 0.00047181302215904) {
                                                                memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            } else {
                                                                if (input[0] <= 1.0808824300765991) {
                                                                    memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                } else {
                                                                    memcpy(var15, (double[]){0.14285714285714285, 0.8571428571428571}, 2 * sizeof(double));
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        } else {
                            if (input[1] <= -0.3641737550497055) {
                                if (input[4] <= 0.03434048034250736) {
                                    if (input[2] <= 1.4129165410995483) {
                                        if (input[3] <= 0.4294358640909195) {
                                            if (input[3] <= 0.36371318995952606) {
                                                if (input[1] <= -0.5757394731044769) {
                                                    memcpy(var15, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var15, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var15, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[1] <= -0.4031882584095001) {
                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[1] <= -0.3934764862060547) {
                                            memcpy(var15, (double[]){0.2857142857142857, 0.7142857142857143}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            } else {
                                if (input[4] <= 0.17620887607336044) {
                                    if (input[3] <= 0.7290649116039276) {
                                        if (input[3] <= 0.3761197477579117) {
                                            if (input[2] <= 1.5254837274551392) {
                                                memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var15, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var15, (double[]){0.14285714285714285, 0.8571428571428571}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[4] <= 0.3424825668334961) {
                                        if (input[2] <= 1.6106600761413574) {
                                            memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var15, (double[]){0.14285714285714285, 0.8571428571428571}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[1] <= -0.256172277033329) {
                                            if (input[0] <= 1.9169866442680359) {
                                                memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[4] <= 0.47226017713546753) {
                                                if (input[3] <= 1.1496579051017761) {
                                                    memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[2] <= 1.7572978138923645) {
                                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[3] <= 1.208722710609436) {
                                                            if (input[4] <= 0.4360325187444687) {
                                                                memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                            } else {
                                                                memcpy(var15, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                                            }
                                                        } else {
                                                            if (input[4] <= 0.4370242804288864) {
                                                                memcpy(var15, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                            } else {
                                                                memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            }
                                                        }
                                                    }
                                                }
                                            } else {
                                                if (input[3] <= 1.3381627202033997) {
                                                    memcpy(var15, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var15, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    if (input[2] <= 1.246832549571991) {
                        memcpy(var15, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[4] <= 0.7633571624755859) {
                            if (input[4] <= 0.760405957698822) {
                                if (input[3] <= -0.34247124195098877) {
                                    if (input[3] <= -0.3998248130083084) {
                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var15, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[4] <= 0.5129693448543549) {
                                        if (input[2] <= 1.414023220539093) {
                                            memcpy(var15, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                memcpy(var15, (double[]){0.5, 0.5}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[2] <= 1.4013752341270447) {
                                if (input[2] <= 1.397422730922699) {
                                    memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[4] <= 1.0075344443321228) {
                                        memcpy(var15, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                if (input[2] <= 1.4389240145683289) {
                                    if (input[2] <= 1.4365525245666504) {
                                        memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[3] <= 1.0793362259864807) {
                                            memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var15, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                        }
                                    }
                                } else {
                                    memcpy(var15, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    }
                }
            }
        }
        add_vectors(var6, var15, 2, var5);
        double var16[2];
        if (input[3] <= 0.2809315174818039) {
            if (input[2] <= 1.1347792148590088) {
                if (input[2] <= 0.2816321849822998) {
                    if (input[4] <= 1.1901116967201233) {
                        memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[3] <= -0.4730876684188843) {
                            memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            memcpy(var16, (double[]){0.75, 0.25}, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[1] <= -0.35797834396362305) {
                        memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[2] <= 0.2970469444990158) {
                            memcpy(var16, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                        } else {
                            if (input[3] <= -0.6601218581199646) {
                                if (input[1] <= -0.11937037855386734) {
                                    memcpy(var16, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                } else {
                                    memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        }
                    }
                }
            } else {
                if (input[3] <= -0.6344798505306244) {
                    if (input[3] <= -0.7502298355102539) {
                        if (input[1] <= -1.7901284098625183) {
                            memcpy(var16, (double[]){0.5, 0.5}, 2 * sizeof(double));
                        } else {
                            memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[0] <= -0.5557638108730316) {
                            if (input[1] <= -0.08747226372361183) {
                                if (input[1] <= -1.3234614729881287) {
                                    memcpy(var16, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                } else {
                                    memcpy(var16, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[0] <= -0.18394241482019424) {
                                memcpy(var16, (double[]){0.2857142857142857, 0.7142857142857143}, 2 * sizeof(double));
                            } else {
                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        }
                    }
                } else {
                    if (input[1] <= -0.3416525274515152) {
                        if (input[4] <= 0.5695571899414062) {
                            if (input[3] <= 0.2727763503789902) {
                                if (input[3] <= 0.07023395597934723) {
                                    if (input[4] <= 0.05179538577795029) {
                                        if (input[0] <= 1.6467147469520569) {
                                            if (input[0] <= -0.27758924663066864) {
                                                if (input[0] <= -0.3052486479282379) {
                                                    memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var16, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[3] <= -0.5115908086299896) {
                                                    if (input[4] <= -0.885816752910614) {
                                                        memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var16, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    if (input[0] <= 0.6707330644130707) {
                                                        memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[0] <= 0.6936508417129517) {
                                                            memcpy(var16, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    }
                                                }
                                            }
                                        } else {
                                            memcpy(var16, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[4] <= -0.4434572011232376) {
                                        memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[2] <= 1.5487244129180908) {
                                            if (input[3] <= 0.12221315503120422) {
                                                memcpy(var16, (double[]){0.9166666666666666, 0.08333333333333333}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[2] <= 2.5671650171279907) {
                                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var16, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            } else {
                                memcpy(var16, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                            }
                        } else {
                            memcpy(var16, (double[]){0.625, 0.375}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[0] <= 1.1448942422866821) {
                            memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        } else {
                            if (input[2] <= 1.3831937313079834) {
                                if (input[0] <= 1.673583984375) {
                                    memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        }
                    }
                }
            }
        } else {
            if (input[0] <= 0.10173968225717545) {
                if (input[1] <= 0.5634669959545135) {
                    if (input[2] <= 1.0043466985225677) {
                        memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[1] <= -0.4725101441144943) {
                            memcpy(var16, (double[]){0.6, 0.4}, 2 * sizeof(double));
                        } else {
                            memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= 0.5800459012389183) {
                        memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                    }
                }
            } else {
                if (input[4] <= -0.1199018582701683) {
                    if (input[2] <= 0.06463995575904846) {
                        memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[4] <= -1.2167494893074036) {
                            if (input[1] <= -1.4334722757339478) {
                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            } else {
                                if (input[2] <= 1.6274582147598267) {
                                    memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var16, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= 0.9895248115062714) {
                        if (input[2] <= -0.4535326808691025) {
                            memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            if (input[3] <= 1.4415863156318665) {
                                if (input[4] <= 0.22090692818164825) {
                                    memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    if (input[1] <= -0.42328156530857086) {
                                        memcpy(var16, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                memcpy(var16, (double[]){0.5, 0.5}, 2 * sizeof(double));
                            }
                        }
                    } else {
                        if (input[3] <= 0.6213096976280212) {
                            if (input[2] <= 1.3855652213096619) {
                                if (input[1] <= -0.4840637892484665) {
                                    memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[4] <= 0.21335726231336594) {
                                    if (input[4] <= 0.09377220273017883) {
                                        if (input[3] <= 0.504570409655571) {
                                            memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[3] <= 0.5358006954193115) {
                                                memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        if (input[3] <= 0.370237335562706) {
                                            if (input[1] <= -0.33981063961982727) {
                                                memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[3] <= 0.6092774569988251) {
                                                memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var16, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                } else {
                                    if (input[0] <= 0.5249285101890564) {
                                        memcpy(var16, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                    } else {
                                        if (input[3] <= 0.588020533323288) {
                                            memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[4] <= 0.3388553559780121) {
                                                memcpy(var16, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            }
                        } else {
                            if (input[1] <= -0.14867310225963593) {
                                if (input[2] <= 1.5533488988876343) {
                                    memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[1] <= -0.21983688324689865) {
                                        if (input[3] <= 0.7631562054157257) {
                                            if (input[2] <= 2.0519566535949707) {
                                                if (input[2] <= 1.9630253911018372) {
                                                    memcpy(var16, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var16, (double[]){0.5714285714285714, 0.42857142857142855}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[3] <= 1.0741222500801086) {
                                                if (input[1] <= -0.30941951274871826) {
                                                    if (input[2] <= 2.4385111331939697) {
                                                        if (input[0] <= 1.24209725856781) {
                                                            memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var16, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                        }
                                                    } else {
                                                        memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    if (input[2] <= 2.104129672050476) {
                                                        memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[1] <= -0.256172277033329) {
                                                            memcpy(var16, (double[]){0.7142857142857143, 0.2857142857142857}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    }
                                                }
                                            } else {
                                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        if (input[1] <= -0.19053415209054947) {
                                            if (input[3] <= 1.0889620184898376) {
                                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                if (input[3] <= 1.2035354971885681) {
                                                    memcpy(var16, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var16, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                }
                                            }
                                        } else {
                                            if (input[4] <= 0.469703733921051) {
                                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                if (input[0] <= 1.981788694858551) {
                                                    memcpy(var16, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    }
                                }
                            } else {
                                if (input[2] <= 1.918475091457367) {
                                    if (input[3] <= 1.4407039284706116) {
                                        if (input[4] <= 1.255982756614685) {
                                            if (input[4] <= 1.083637535572052) {
                                                if (input[0] <= 0.5759008228778839) {
                                                    if (input[0] <= 0.5510073602199554) {
                                                        memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[1] <= 0.10835370421409607) {
                                                            memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            if (input[4] <= 0.3294130563735962) {
                                                                memcpy(var16, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                                            } else {
                                                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            }
                                                        }
                                                    }
                                                } else {
                                                    memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[1] <= 0.7898515164852142) {
                                                    if (input[2] <= 1.4515719413757324) {
                                                        memcpy(var16, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        } else {
                                            memcpy(var16, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[2] <= 1.9292315244674683) {
                                        memcpy(var16, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                    } else {
                                        if (input[3] <= 1.2056745886802673) {
                                            if (input[1] <= -0.07526558265089989) {
                                                memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var16, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var16, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        add_vectors(var5, var16, 2, var4);
        double var17[2];
        if (input[0] <= 0.2866625338792801) {
            if (input[1] <= -0.4496540278196335) {
                if (input[2] <= 1.0114611685276031) {
                    memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                } else {
                    if (input[1] <= -1.7356253266334534) {
                        if (input[3] <= -0.7491603195667267) {
                            if (input[3] <= -0.7712193727493286) {
                                memcpy(var17, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                            } else {
                                memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        } else {
                            memcpy(var17, (double[]){0.25, 0.75}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -1.0665183663368225) {
                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        } else {
                            if (input[3] <= -0.7044004499912262) {
                                if (input[0] <= -0.5739399790763855) {
                                    memcpy(var17, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                } else {
                                    memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[2] <= 1.48457533121109) {
                                    if (input[2] <= 1.3203490376472473) {
                                        if (input[0] <= -0.27363790571689606) {
                                            if (input[0] <= -0.35108423233032227) {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var17, (double[]){0.7142857142857143, 0.2857142857142857}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[4] <= -1.0770507454872131) {
                                            memcpy(var17, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                        } else {
                                            if (input[2] <= 1.3466726541519165) {
                                                if (input[2] <= 1.3405067324638367) {
                                                    memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var17, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                } else {
                                    if (input[1] <= -0.5575717985630035) {
                                        memcpy(var17, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                    } else {
                                        if (input[0] <= 0.10173968225717545) {
                                            if (input[0] <= 0.07684621959924698) {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var17, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                if (input[3] <= 0.3995157480239868) {
                    if (input[4] <= 1.688637137413025) {
                        if (input[3] <= -0.4629271328449249) {
                            if (input[1] <= 1.1249073147773743) {
                                if (input[3] <= -0.6943468451499939) {
                                    memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    if (input[0] <= -1.0164903700351715) {
                                        if (input[1] <= -0.1354450173676014) {
                                            memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var17, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                if (input[3] <= -0.6464853584766388) {
                                    memcpy(var17, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                } else {
                                    if (input[3] <= -0.6240252256393433) {
                                        memcpy(var17, (double[]){0.8333333333333334, 0.16666666666666666}, 2 * sizeof(double));
                                    } else {
                                        if (input[2] <= 1.1575060486793518) {
                                            memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var17, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                        }
                                    }
                                }
                            }
                        } else {
                            if (input[1] <= 1.3835248351097107) {
                                if (input[2] <= 0.7895283401012421) {
                                    memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[4] <= 1.3413482308387756) {
                                    memcpy(var17, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                } else {
                                    memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    } else {
                        if (input[2] <= 0.4061358869075775) {
                            memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[0] <= 0.10173968225717545) {
                        if (input[1] <= 0.5391875803470612) {
                            if (input[0] <= 0.011649059131741524) {
                                if (input[1] <= -0.3414013683795929) {
                                    memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[2] <= 0.44585853815078735) {
                                        memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[4] <= 1.03702712059021) {
                                if (input[4] <= 0.5427016913890839) {
                                    memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[0] <= -0.13059929013252258) {
                                        if (input[4] <= 0.8793658316135406) {
                                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[4] <= 0.8892554044723511) {
                                            if (input[3] <= 1.0611542165279388) {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                if (input[2] <= 0.39664992690086365) {
                                                    memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        } else {
                                            if (input[0] <= -0.04999189078807831) {
                                                memcpy(var17, (double[]){0.9, 0.1}, 2 * sizeof(double));
                                            } else {
                                                if (input[2] <= 0.39664992690086365) {
                                                    memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    }
                                }
                            } else {
                                memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        }
                    } else {
                        if (input[2] <= 0.4369654059410095) {
                            memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[4] <= -1.5333003401756287) {
                if (input[3] <= -0.7271012365818024) {
                    memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                } else {
                    memcpy(var17, (double[]){0.6, 0.4}, 2 * sizeof(double));
                }
            } else {
                if (input[2] <= 1.1235145926475525) {
                    if (input[2] <= 0.9290515780448914) {
                        if (input[1] <= 1.9778679609298706) {
                            if (input[4] <= -1.3968144059181213) {
                                memcpy(var17, (double[]){0.75, 0.25}, 2 * sizeof(double));
                            } else {
                                if (input[4] <= 1.1809985637664795) {
                                    if (input[3] <= 0.520640105009079) {
                                        memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[1] <= -0.6400380581617355) {
                                            memcpy(var17, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        }
                                    }
                                } else {
                                    if (input[4] <= 1.3866872191429138) {
                                        if (input[1] <= 1.0736693739891052) {
                                            memcpy(var17, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var17, (double[]){0.8571428571428571, 0.14285714285714285}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    }
                                }
                            }
                        } else {
                            memcpy(var17, (double[]){0.5, 0.5}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= -0.09107231348752975) {
                            memcpy(var17, (double[]){0.25, 0.75}, 2 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= 1.3855652213096619) {
                        if (input[1] <= -0.29518676176667213) {
                            if (input[1] <= -0.6392845511436462) {
                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            } else {
                                if (input[3] <= 0.3285256326198578) {
                                    memcpy(var17, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                } else {
                                    memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            if (input[2] <= 1.296634018421173) {
                                memcpy(var17, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                            } else {
                                if (input[1] <= 0.2648302912712097) {
                                    if (input[4] <= 0.7145896255970001) {
                                        memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    } else {
                        if (input[3] <= -0.45757947862148285) {
                            if (input[1] <= -0.3692808151245117) {
                                if (input[3] <= -0.4797722399234772) {
                                    memcpy(var17, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                } else {
                                    memcpy(var17, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[1] <= -0.14867310225963593) {
                                if (input[0] <= 1.2385410070419312) {
                                    if (input[3] <= 1.1186414957046509) {
                                        if (input[3] <= 0.39590607583522797) {
                                            if (input[3] <= 0.3918953388929367) {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var17, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[0] <= 1.1211861371994019) {
                                            memcpy(var17, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                        } else {
                                            if (input[1] <= -0.20141802728176117) {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                if (input[2] <= 1.5341792702674866) {
                                                    memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var17, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    }
                                } else {
                                    if (input[0] <= 1.3250754475593567) {
                                        if (input[3] <= 0.7193054258823395) {
                                            if (input[3] <= -0.028029173612594604) {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                if (input[2] <= 1.5076975226402283) {
                                                    memcpy(var17, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                }
                                            }
                                        } else {
                                            if (input[2] <= 2.0365419387817383) {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var17, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        if (input[2] <= 1.4124422669410706) {
                                            if (input[3] <= 0.504570409655571) {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[1] <= -0.15101732313632965) {
                                                if (input[1] <= -0.33205798268318176) {
                                                    if (input[1] <= -0.776170164346695) {
                                                        if (input[1] <= -0.790319174528122) {
                                                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var17, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                        }
                                                    } else {
                                                        if (input[2] <= 2.803326725959778) {
                                                            if (input[4] <= 0.09751220047473907) {
                                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            } else {
                                                                if (input[3] <= 0.14515458047389984) {
                                                                    memcpy(var17, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                                } else {
                                                                    memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                }
                                                            }
                                                        } else {
                                                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    }
                                                } else {
                                                    if (input[2] <= 1.621529459953308) {
                                                        memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[2] <= 1.9335398077964783) {
                                                            if (input[2] <= 1.8209726214408875) {
                                                                memcpy(var17, (double[]){0.1111111111111111, 0.8888888888888888}, 2 * sizeof(double));
                                                            } else {
                                                                memcpy(var17, (double[]){0.875, 0.125}, 2 * sizeof(double));
                                                            }
                                                        } else {
                                                            if (input[4] <= 0.3828900158405304) {
                                                                if (input[1] <= -0.2968612164258957) {
                                                                    memcpy(var17, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                                } else {
                                                                    memcpy(var17, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                                }
                                                            } else {
                                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            }
                                                        }
                                                    }
                                                }
                                            } else {
                                                memcpy(var17, (double[]){0.36363636363636365, 0.6363636363636364}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            } else {
                                if (input[0] <= 0.7141978442668915) {
                                    if (input[3] <= 0.6144914031028748) {
                                        if (input[4] <= 0.3369506299495697) {
                                            if (input[1] <= 0.12099574133753777) {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var17, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[0] <= 0.686538428068161) {
                                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[1] <= 1.071660041809082) {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var17, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                } else {
                                    if (input[2] <= 1.918475091457367) {
                                        if (input[1] <= 0.7823165357112885) {
                                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[1] <= 0.7965492904186249) {
                                                memcpy(var17, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        if (input[3] <= 1.42884361743927) {
                                            memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[4] <= 0.4724009037017822) {
                                                memcpy(var17, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var17, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        add_vectors(var4, var17, 2, var3);
        double var18[2];
        if (input[2] <= 1.1347792148590088) {
            if (input[0] <= 3.245428204536438) {
                if (input[3] <= 3.5010212659835815) {
                    if (input[2] <= 0.9946630597114563) {
                        if (input[0] <= 0.35423049330711365) {
                            memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            if (input[4] <= 1.1809985637664795) {
                                if (input[3] <= 0.3369481861591339) {
                                    memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    if (input[3] <= 0.34777718782424927) {
                                        memcpy(var18, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                if (input[4] <= 1.3839367032051086) {
                                    if (input[1] <= 1.0854741930961609) {
                                        memcpy(var18, (double[]){0.5555555555555556, 0.4444444444444444}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    } else {
                        if (input[3] <= -0.1662660390138626) {
                            memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                        } else {
                            memcpy(var18, (double[]){0.6, 0.4}, 2 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var18, (double[]){0.625, 0.375}, 2 * sizeof(double));
                }
            } else {
                memcpy(var18, (double[]){0.75, 0.25}, 2 * sizeof(double));
            }
        } else {
            if (input[1] <= -1.746090590953827) {
                if (input[4] <= -1.8520514965057373) {
                    memcpy(var18, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                } else {
                    memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                }
            } else {
                if (input[0] <= 1.2385410070419312) {
                    if (input[3] <= -0.6494265496730804) {
                        if (input[3] <= -0.7233578860759735) {
                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        } else {
                            if (input[4] <= -0.5377598404884338) {
                                if (input[3] <= -0.6993468999862671) {
                                    memcpy(var18, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                } else {
                                    memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        }
                    } else {
                        if (input[0] <= -0.24992984533309937) {
                            if (input[2] <= 1.478251338005066) {
                                if (input[2] <= 1.2745000123977661) {
                                    if (input[2] <= 1.2713379859924316) {
                                        if (input[4] <= -0.538336843252182) {
                                            memcpy(var18, (double[]){0.14285714285714285, 0.8571428571428571}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        memcpy(var18, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[2] <= 1.3748934864997864) {
                                        if (input[0] <= -0.3131513446569443) {
                                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[1] <= 0.059962332248687744) {
                                                memcpy(var18, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var18, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                memcpy(var18, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[2] <= 1.2373465299606323) {
                                memcpy(var18, (double[]){0.5, 0.5}, 2 * sizeof(double));
                            } else {
                                if (input[3] <= 1.4431104063987732) {
                                    if (input[1] <= -1.661615014076233) {
                                        if (input[3] <= -0.13672025501728058) {
                                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var18, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[3] <= 1.4336565136909485) {
                                            if (input[4] <= 0.3176381587982178) {
                                                if (input[4] <= 0.31200648844242096) {
                                                    if (input[2] <= 1.48457533121109) {
                                                        memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[0] <= 0.10173968225717545) {
                                                            if (input[1] <= -0.4795428067445755) {
                                                                memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            } else {
                                                                memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                            }
                                                        } else {
                                                            if (input[3] <= 1.1170371770858765) {
                                                                if (input[1] <= -1.4237605333328247) {
                                                                    if (input[4] <= -1.217920184135437) {
                                                                        memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    } else {
                                                                        if (input[4] <= -1.2070039510726929) {
                                                                            memcpy(var18, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                                                        } else {
                                                                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                        }
                                                                    }
                                                                } else {
                                                                    if (input[1] <= 0.08826040476560593) {
                                                                        if (input[2] <= 1.4857611060142517) {
                                                                            if (input[3] <= 0.23079726845026016) {
                                                                                memcpy(var18, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                                            } else {
                                                                                memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                            }
                                                                        } else {
                                                                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                        }
                                                                    } else {
                                                                        if (input[2] <= 1.5681707262992859) {
                                                                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                        } else {
                                                                            memcpy(var18, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                                        }
                                                                    }
                                                                }
                                                            } else {
                                                                if (input[1] <= -0.20979022979736328) {
                                                                    if (input[3] <= 1.1318769454956055) {
                                                                        memcpy(var18, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                                    } else {
                                                                        memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    }
                                                                } else {
                                                                    if (input[4] <= 0.21034228801727295) {
                                                                        if (input[3] <= 1.1884284019470215) {
                                                                            memcpy(var18, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                                                        } else {
                                                                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                        }
                                                                    } else {
                                                                        memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                } else {
                                                    if (input[4] <= 0.31549103558063507) {
                                                        memcpy(var18, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[1] <= 0.08658596128225327) {
                                                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var18, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                                        }
                                                    }
                                                }
                                            } else {
                                                if (input[3] <= 1.1445776224136353) {
                                                    memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[3] <= 1.1551392078399658) {
                                                        memcpy(var18, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                }
                                            }
                                        } else {
                                            memcpy(var18, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                        }
                                    }
                                } else {
                                    memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    }
                } else {
                    if (input[2] <= 1.4124422669410706) {
                        if (input[0] <= 1.5787516832351685) {
                            if (input[4] <= -0.10157802328467369) {
                                memcpy(var18, (double[]){0.5, 0.5}, 2 * sizeof(double));
                            } else {
                                if (input[1] <= -0.4880824536085129) {
                                    memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var18, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                }
                            }
                        } else {
                            if (input[4] <= 0.4560089707374573) {
                                memcpy(var18, (double[]){0.5, 0.5}, 2 * sizeof(double));
                            } else {
                                memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        }
                    } else {
                        if (input[0] <= 1.3653791546821594) {
                            if (input[4] <= 0.20687378197908401) {
                                if (input[1] <= -0.3958207070827484) {
                                    memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[4] <= 0.10660286620259285) {
                                        memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    } else {
                                        if (input[1] <= -0.32834072411060333) {
                                            memcpy(var18, (double[]){0.3, 0.7}, 2 * sizeof(double));
                                        } else {
                                            if (input[2] <= 1.5071045756340027) {
                                                memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            } else {
                                memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[2] <= 1.859272301197052) {
                                memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            } else {
                                if (input[0] <= 1.8909077644348145) {
                                    if (input[0] <= 1.6901795864105225) {
                                        if (input[2] <= 1.868955910205841) {
                                            memcpy(var18, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                        } else {
                                            if (input[0] <= 1.4092390537261963) {
                                                if (input[2] <= 2.1491881608963013) {
                                                    memcpy(var18, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        if (input[3] <= 1.3877621293067932) {
                                            if (input[4] <= 0.4559105485677719) {
                                                if (input[4] <= 0.4106105864048004) {
                                                    if (input[4] <= 0.3476560711860657) {
                                                        memcpy(var18, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var18, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    }
                                } else {
                                    if (input[0] <= 3.0771008729934692) {
                                        if (input[3] <= -0.3922044038772583) {
                                            if (input[2] <= 2.475269317626953) {
                                                memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var18, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[2] <= 2.4124245643615723) {
                                                if (input[2] <= 2.329896330833435) {
                                                    memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var18, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        if (input[1] <= -0.7854633033275604) {
                                            memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[4] <= 0.8005732893943787) {
                                                memcpy(var18, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var18, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        add_vectors(var3, var18, 2, var2);
        double var19[2];
        if (input[0] <= 0.2866625338792801) {
            if (input[2] <= 0.7505962252616882) {
                memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
            } else {
                if (input[3] <= -0.7118604183197021) {
                    if (input[0] <= -0.3732117563486099) {
                        if (input[4] <= -1.9374032616615295) {
                            if (input[0] <= -0.7043342888355255) {
                                memcpy(var19, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                            } else {
                                memcpy(var19, (double[]){0.8571428571428571, 0.14285714285714285}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[2] <= 1.3473050594329834) {
                                memcpy(var19, (double[]){0.4, 0.6}, 2 * sizeof(double));
                            } else {
                                memcpy(var19, (double[]){0.75, 0.25}, 2 * sizeof(double));
                            }
                        }
                    } else {
                        memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    }
                } else {
                    if (input[2] <= 1.2631958723068237) {
                        if (input[4] <= -0.4367433339357376) {
                            if (input[1] <= -0.09500725194811821) {
                                memcpy(var19, (double[]){0.375, 0.625}, 2 * sizeof(double));
                            } else {
                                memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        } else {
                            memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 1.3132345080375671) {
                            if (input[1] <= -0.49863143265247345) {
                                if (input[0] <= -0.35108423233032227) {
                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    if (input[4] <= -0.7703505754470825) {
                                        memcpy(var19, (double[]){0.6, 0.4}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var19, (double[]){0.2222222222222222, 0.7777777777777778}, 2 * sizeof(double));
                                    }
                                }
                            } else {
                                memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[4] <= -0.408050075173378) {
                                if (input[4] <= -0.42132437229156494) {
                                    if (input[2] <= 1.5439814329147339) {
                                        if (input[3] <= -0.6963522136211395) {
                                            memcpy(var19, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                        } else {
                                            if (input[2] <= 1.499871551990509) {
                                                if (input[3] <= -0.5619657039642334) {
                                                    if (input[0] <= -0.39296846091747284) {
                                                        memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var19, (double[]){0.16666666666666666, 0.8333333333333334}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            } else {
                                                if (input[4] <= -0.4525112211704254) {
                                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    memcpy(var19, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                }
                                            }
                                        }
                                    } else {
                                        memcpy(var19, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var19, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[0] <= -0.5281043946743011) {
                                    memcpy(var19, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                } else {
                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    }
                }
            }
        } else {
            if (input[4] <= -1.5261791348457336) {
                if (input[3] <= -0.7271012365818024) {
                    memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                } else {
                    memcpy(var19, (double[]){0.625, 0.375}, 2 * sizeof(double));
                }
            } else {
                if (input[1] <= -1.1627150774002075) {
                    if (input[2] <= 0.38676867890171707) {
                        memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                    } else {
                        if (input[3] <= -0.11987513676285744) {
                            if (input[1] <= -1.6999596953392029) {
                                memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            } else {
                                memcpy(var19, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[1] <= -1.4304583072662354) {
                                if (input[1] <= -1.4334722757339478) {
                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var19, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                            }
                        }
                    }
                } else {
                    if (input[3] <= 0.520479679107666) {
                        if (input[2] <= 1.247425377368927) {
                            if (input[0] <= 0.37675315141677856) {
                                if (input[1] <= 1.0018358528614044) {
                                    memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var19, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                }
                            } else {
                                memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[3] <= 0.383071705698967) {
                                if (input[0] <= 1.2160183787345886) {
                                    if (input[3] <= -0.48378297686576843) {
                                        memcpy(var19, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                    } else {
                                        memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    if (input[0] <= 1.716258466243744) {
                                        if (input[4] <= 0.8454334437847137) {
                                            if (input[3] <= -0.2344486191868782) {
                                                if (input[3] <= -0.3380594104528427) {
                                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[2] <= 2.7527347803115845) {
                                                        memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var19, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                                    }
                                                }
                                            } else {
                                                if (input[3] <= 0.28414011001586914) {
                                                    memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[0] <= 1.2598782777786255) {
                                                        memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var19, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                                    }
                                                }
                                            }
                                        } else {
                                            memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[1] <= -0.3939788192510605) {
                                            memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                        } else {
                                            if (input[2] <= 2.4337680339813232) {
                                                memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var19, (double[]){0.8, 0.2}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            } else {
                                if (input[4] <= 0.19169249385595322) {
                                    if (input[4] <= 0.08012082241475582) {
                                        if (input[2] <= 1.3950512409210205) {
                                            if (input[2] <= 1.373233437538147) {
                                                memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var19, (double[]){0.6666666666666666, 0.3333333333333333}, 2 * sizeof(double));
                                            }
                                        } else {
                                            if (input[3] <= 0.49911579489707947) {
                                                memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var19, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                            }
                                        }
                                    } else {
                                        memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                    }
                                } else {
                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    } else {
                        if (input[3] <= 2.4774802923202515) {
                            if (input[1] <= 1.661398470401764) {
                                if (input[2] <= 0.9760863184928894) {
                                    if (input[4] <= 1.336536169052124) {
                                        if (input[3] <= 0.5816033780574799) {
                                            memcpy(var19, (double[]){0.75, 0.25}, 2 * sizeof(double));
                                        } else {
                                            memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                        }
                                    } else {
                                        if (input[1] <= 1.106907069683075) {
                                            if (input[3] <= 1.451934039592743) {
                                                memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                            } else {
                                                memcpy(var19, (double[]){0.8571428571428571, 0.14285714285714285}, 2 * sizeof(double));
                                            }
                                        } else {
                                            memcpy(var19, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                        }
                                    }
                                } else {
                                    if (input[1] <= -0.2611956000328064) {
                                        if (input[1] <= -0.3097544014453888) {
                                            if (input[1] <= -0.369833379983902) {
                                                memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            } else {
                                                if (input[3] <= 0.988693505525589) {
                                                    if (input[1] <= -0.3608248829841614) {
                                                        memcpy(var19, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                    } else {
                                                        if (input[4] <= 0.11618300154805183) {
                                                            memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            if (input[4] <= 0.38019463419914246) {
                                                                if (input[4] <= 0.28088028728961945) {
                                                                    memcpy(var19, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                                                } else {
                                                                    memcpy(var19, (double[]){0.2, 0.8}, 2 * sizeof(double));
                                                                }
                                                            } else {
                                                                memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            }
                                                        }
                                                    }
                                                } else {
                                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                }
                                            }
                                        } else {
                                            if (input[3] <= 0.690214216709137) {
                                                if (input[1] <= -0.3027217537164688) {
                                                    memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[4] <= 0.16887637227773666) {
                                                        memcpy(var19, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                    }
                                                }
                                            } else {
                                                if (input[0] <= 1.7763188481330872) {
                                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[4] <= 0.41557060182094574) {
                                                        memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                }
                                            }
                                        }
                                    } else {
                                        if (input[0] <= 0.7141978442668915) {
                                            if (input[3] <= 0.6568715870380402) {
                                                if (input[3] <= 0.6043308675289154) {
                                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                } else {
                                                    if (input[2] <= 1.5533488988876343) {
                                                        memcpy(var19, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                    } else {
                                                        memcpy(var19, (double[]){0.1, 0.9}, 2 * sizeof(double));
                                                    }
                                                }
                                            } else {
                                                if (input[0] <= 0.6936508417129517) {
                                                    if (input[3] <= 0.8908314406871796) {
                                                        if (input[4] <= 0.31460289657115936) {
                                                            memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        } else {
                                                            if (input[3] <= 0.8771949112415314) {
                                                                memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            } else {
                                                                memcpy(var19, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                            }
                                                        }
                                                    } else {
                                                        memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    memcpy(var19, (double[]){0.4, 0.6}, 2 * sizeof(double));
                                                }
                                            }
                                        } else {
                                            if (input[3] <= 1.438469409942627) {
                                                if (input[1] <= 0.7016921639442444) {
                                                    if (input[3] <= 1.4171742796897888) {
                                                        if (input[1] <= -0.14825449883937836) {
                                                            if (input[2] <= 1.5460367798805237) {
                                                                memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                            } else {
                                                                if (input[1] <= -0.21983688324689865) {
                                                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                } else {
                                                                    if (input[2] <= 1.936859905719757) {
                                                                        if (input[0] <= 1.4649530053138733) {
                                                                            if (input[2] <= 1.583980679512024) {
                                                                                if (input[2] <= 1.5705422163009644) {
                                                                                    memcpy(var19, (double[]){0.25, 0.75}, 2 * sizeof(double));
                                                                                } else {
                                                                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                                }
                                                                            } else {
                                                                                memcpy(var19, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                                            }
                                                                        } else {
                                                                            memcpy(var19, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                                        }
                                                                    } else {
                                                                        memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                                    }
                                                                }
                                                            }
                                                        } else {
                                                            memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    } else {
                                                        memcpy(var19, (double[]){0.3333333333333333, 0.6666666666666666}, 2 * sizeof(double));
                                                    }
                                                } else {
                                                    if (input[1] <= 0.752427726984024) {
                                                        if (input[2] <= 1.4436669945716858) {
                                                            memcpy(var19, (double[]){0.5, 0.5}, 2 * sizeof(double));
                                                        } else {
                                                            memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                        }
                                                    } else {
                                                        memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                                    }
                                                }
                                            } else {
                                                memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                            }
                                        }
                                    }
                                }
                            } else {
                                memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                            }
                        } else {
                            if (input[4] <= 1.88131982088089) {
                                if (input[2] <= 0.37293490767478943) {
                                    memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            } else {
                                if (input[2] <= 0.3646346628665924) {
                                    memcpy(var19, (double[]){1.0, 0.0}, 2 * sizeof(double));
                                } else {
                                    memcpy(var19, (double[]){0.0, 1.0}, 2 * sizeof(double));
                                }
                            }
                        }
                    }
                }
            }
        }
        add_vectors(var2, var19, 2, var1);
        mul_vector_number(var1, 0.1, 2, var0);
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