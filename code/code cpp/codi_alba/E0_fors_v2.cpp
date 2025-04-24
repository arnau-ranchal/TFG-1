#include "E0_fors_v2.h"
#include <complex>
#include <cmath>

const double PI = 3.14159265358979323846;

double G(const std::complex<double>& z) {
    return (1.0 / PI) * std::exp(-std::norm(z));
}

double fp_only(const std::vector<double>& x, int index,
               const std::vector<double>& q,
               const std::complex<double>& z,
               double rho, double SNR) {

    double sum = 0.0;
    double exponent = 1.0 / (1.0 + rho);
    double den = std::pow(G(z), exponent);

    for (size_t barret = 0; barret < x.size(); ++barret) {
        std::complex<double> z_shifted = z + std::sqrt(SNR) * x[index] - std::sqrt(SNR) * x[barret];
        double gnum = G(z_shifted);
        sum += q[barret] * std::pow(gnum, exponent);
    }

    return sum / den;
}

double E0_fors_v2(double rho, int N,
                  const std::vector<double>& q,
                  const std::vector<double>& x,
                  const std::vector<double>& w,
                  const std::vector<double>& z,
                  double SNR) {

    double sumTotal = 0.0;

    for (size_t index = 0; index < x.size(); ++index) {
        double sumik = 0.0;

        for (int i = 0; i < N; ++i) {
            for (int k = 0; k < N; ++k) {
                std::complex<double> zc(z[i], z[k]);
                double fpResult = fp_only(x, index, q, zc, rho, SNR);
                sumik += w[i] * w[k] * std::pow(fpResult, rho);
            }
        }

        sumTotal += q[index] * sumik;
    }

    return -std::log2((1.0 / PI) * sumTotal);
}
