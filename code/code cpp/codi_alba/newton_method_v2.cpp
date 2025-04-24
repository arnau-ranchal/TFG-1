#include "newton_method_v2.h"
#include <cmath>
#include <algorithm>

// Declaraciones de funciones externas que debes definir en algún archivo aparte
void F0_fors(double rho, int N, const std::vector<double>& q,
             const std::vector<double>& x, const std::vector<double>& w,
             const std::vector<double>& z, double SNR,
             double& f, double& fp, double& f2p);

void F0_F0p_fors(double rho, int N, const std::vector<double>& q,
                 const std::vector<double>& x, const std::vector<double>& w,
                 const std::vector<double>& z, double SNR,
                 double& f, double& fp);

void F0_matrix_fixed(const std::vector<std::vector<double>>& Q,
                     const std::vector<double>& pi_,
                     const std::vector<std::vector<double>>& G,
                     double rho,
                     double& f, double& fp, double& f2p);

void F0_F0p_matrix(const std::vector<std::vector<double>>& Q,
                   const std::vector<double>& pi_,
                   const std::vector<std::vector<double>>& G,
                   double rho,
                   double& f, double& fp);

double newton_method_v2(const std::vector<std::vector<double>>& Q,
                        const std::vector<double>& pi_,
                        const std::vector<std::vector<double>>& G,
                        int max_iteracions,
                        double threshold,
                        double R,
                        double rho,
                        int N,
                        const std::vector<double>& x,
                        const std::vector<double>& w,
                        const std::vector<double>& z,
                        double SNR,
                        int flag) {

    if (flag == 1) { // ----------------------------- FORS CASE
        std::vector<double> q(Q.size());
        for (size_t i = 0; i < Q.size(); ++i)
            q[i] = Q[i][0]; // Suponiendo Q es columna para este caso

        double f, fp, f2p;
        F0_fors(rho, N, q, x, w, z, SNR, f, fp, f2p);
        double curvatura_fixada = -(f2p / f - (fp / f) * (fp / f)) / std::log(2.0);

        for (int iteration = 0; iteration < max_iteracions; ++iteration) {
            F0_F0p_fors(rho, N, q, x, w, z, SNR, f, fp);
            double gradient = -fp / (f * std::log(2.0)) - R;

            rho = rho - gradient / curvatura_fixada;

            if (std::abs(gradient / curvatura_fixada) < threshold)
                break;
        }

        rho = std::max(0.0, std::min(1.0, rho));
    } else { // ----------------------------- MATRICES CASE
        double f, fp, f2p;
        F0_matrix_fixed(Q, pi_, G, rho, f, fp, f2p);
        double curvatura_fixada = -(f2p / f - (fp / f) * (fp / f)) / std::log(2.0);

        for (int iteration = 0; iteration < max_iteracions; ++iteration) {
            F0_F0p_matrix(Q, pi_, G, rho, f, fp);
            double gradient = -fp / (f * std::log(2.0)) - R;

            rho = rho - gradient / curvatura_fixada;

            if (std::abs(gradient / curvatura_fixada) < threshold)
                break;
        }

        rho = std::max(0.0, std::min(1.0, rho));
    }

    return rho;
}
