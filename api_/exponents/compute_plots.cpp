// compute_plots.cpp
#include <cmath>
#include <cstring>
#include "functions.h"
#include <iostream>

extern "C" {
    float* compute_plots(float* M,
                     char** typeMs,
                     float* SNRs,
                     float* Rs,
                     float N,
                     int num_M,
                     int num_typeMs,
                     int num_SNRs,
                     int num_Rs,
                     float* results // IMPORTANT: Output buffer (size >= 3*num_M*num_typeMs*num_SNRs*num_Rs)) {

        const int it = 20;
        const int n_ = 100;

        int result_index = 0;

        for(int m_idx = 0; m_idx < num_M; m_idx++) {
            for(int type_idx = 0; type_idx < num_typeMs; type_idx++) {
                for(int snr_idx = 0; snr_idx < num_SNRs; snr_idx++) {
                    for(int r_idx = 0; r_idx < num_Rs; r_idx++) {

                        // Set parameters
                        setMod(M[m_idx], typeMs[type_idx]);
                        setSNR(SNRs[snr_idx]);
                        setR(Rs[r_idx]);
                        setN(N);

                        // Initialize matrices
                        setQ();
                        setPI();
                        setW();

                        // Calculate exponents
                        double rho_gd, rho_interpolated;
                        double r;
                        double e0 = GD_iid(r, rho_gd, rho_interpolated, it, N);

                        results[result_index++] = pow(2, -n_ * e0);   // Error probability
                        results[result_index++] = e0;                 // Exponent
                        results[result_index++] = rho_gd;             // Optimal rho
                    }
                }
            }
        }

        return results;
    }

}

