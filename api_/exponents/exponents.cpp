// exponents.cpp
#include <cmath>
#include <cstring>
#include "functions.h"
#include <iostream>
#define DEBUG 0

extern "C" {

    float* exponents(float M, const char* typeM, float SNR, float R, float N, float* results) {

        int it = 20;

        setMod(M, typeM);
        setQ(); // matrix Q
        setR(R);
        setSNR(SNR);
        setN(N);

        // matrices
        setPI();
        setW();

        double rho_gd, rho_interpolated;
        double r;
        double e0 = GD_iid(r, rho_gd, rho_interpolated, it, N);

        // Per fer debug
        #if DEBUG
        std::cout << "e0: " << e0 << std::endl;
        std::cout << "rho_gd: " << rho_gd << std::endl;
        #endif

        results[0] = -1; // Pe
        results[1] = e0; // exp
        results[2] = rho_gd;
        //results[2] = 1.1;
        return results;
    }
}
