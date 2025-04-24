#include "F0_fors.h"
#include <complex>
#include <cmath>

const double PI = 3.141592653589793;

double G(const std::complex<double>& z) {
    return (1.0 / PI) * std::exp(-std::norm(z));
}

void fp(const std::vector<double>& x, int index,
        const std::vector<double>& q,
        const std::complex<double>& z,
        double rho, double SNR,
        double& result, double& result2, double& result3) {
    
    double sum = 0.0;
    double sum2 = 0.0;
    double sum3 = 0.0;

    double exponent = 1.0 / (1.0 + rho);
    double den = std::pow(G(z), exponent);

    for (size_t barret = 0; barret < x.size(); ++barret) {
        std::complex<double> z_shifted = z + std::sqrt(SNR) * x[index] - std::sqrt(SNR) * x[barret];
        double gnum = G(z_shifted);
        double g_ratio = gnum / G(z);
        double log_g_ratio = std::log(g_ratio);

        double weight = q[barret] * std::pow(gnum, exponent);

        sum += weight;
        sum2 += weight * log_g_ratio;
        sum3 += weight * log_g_ratio * log_g_ratio * (-1.0 / std::pow(1.0 + rho, 2));
    }

    result = sum / den;
    result2 = sum2 / den;
    result3 = sum3 / den;
}

void F0_fors(double rho, int N,
             const std::vector<double>& q,
             const std::vector<double>& x,
             const std::vector<double>& w,
             const std::vector<double>& z,
             double SNR,
             double& f0, double& f0p, double& f02p) {

    f0 = 0.0;
    f0p = 0.0;
    f02p = 0.0;

    for (size_t index = 0; index < x.size(); ++index) {
        double sumF0ik = 0.0;
        double sumF0pik = 0.0;
        double sumF02pik = 0.0;

        for (int i = 0; i < N; ++i) {
            for (int k = 0; k < N; ++k) {
                std::complex<double> zc(z[i], z[k]);
                double fpResult, fptildeResult, fpt2;
                fp(x, index, q, zc, rho, SNR, fpResult, fptildeResult, fpt2);

                double common = std::pow(fpResult, rho);
                double ftilde_over_fp = fptildeResult / fpResult;
                double log_fp = std::log(fpResult);
                double one_rho = 1.0 + rho;

                double part1 = log_fp - rho / (one_rho * one_rho) * ftilde_over_fp;
                double part2 = part1 * part1
                    - rho / std::pow(one_rho, 4) * ftilde_over_fp * ftilde_over_fp
                    - 2.0 / std::pow(one_rho, 3) * ftilde_over_fp
                    - rho / (one_rho * one_rho) * (fpt2 / fpResult);

                double weight = w[i] * w[k];
                sumF0ik += weight * common;
                sumF0pik += weight * common * part1;
                sumF02pik += weight * common * part2;
            }
        }

        f0 += q[index] * sumF0ik;
        f0p += q[index] * sumF0pik;
        f02p += q[index] * sumF02pik;
    }

    f0 /= PI;
    f0p /= PI;
    f02p /= PI;
}
