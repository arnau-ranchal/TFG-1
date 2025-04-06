// exponents.cpp
#include <cmath> // Para usar pow()

extern "C" {
    // TODO: Canviar funció per la de veritat + adaptar input/output parametres
    float exponents(float M, float SNR, float R, float threshold) {
        float sumatori = M + SNR + R + threshold;
        
        return sumatori; 
    }
}
