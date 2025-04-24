#include "matrix_generator.h"
#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>

// Funciones auxiliares
std::vector<std::vector<double>> get_q_matrix(const std::vector<double>& q);
std::vector<std::vector<double>> get_pi_matrix(const std::vector<double>& w, int N, int M);
std::vector<std::vector<double>> get_g_matrix(const std::vector<double>& z, const std::vector<double>& x, double SNR, int N, int M);
double g(const std::complex<double>& z);

// Función principal
void matrix_generator(int N,
                      const std::vector<double>& q,
                      const std::vector<double>& x,
                      const std::vector<double>& w,
                      const std::vector<double>& z,
                      double SNR,
                      double clip,
                      std::vector<std::vector<double>>& Q,
                      std::vector<std::vector<double>>& pi_,
                      std::vector<std::vector<double>>& G) {
    int M = x.size();
    Q = get_q_matrix(q);
    pi_ = get_pi_matrix(w, N, M);
    G = get_g_matrix(z, x, SNR, N, M);

    double min_val = std::exp(-clip);
    for (auto& row : G) {
        for (auto& val : row) {
            val = std::max(val, min_val);
        }
    }
}

// Transpose vector (Matlab q.')
std::vector<std::vector<double>> get_q_matrix(const std::vector<double>& q) {
    std::vector<std::vector<double>> Q(q.size(), std::vector<double>(1));
    for (size_t i = 0; i < q.size(); ++i) {
        Q[i][0] = q[i];
    }
    return Q;
}

std::vector<std::vector<double>> get_pi_matrix(const std::vector<double>& w, int N, int M) {
    int fil = M;
    int col = M * (N * N);
    std::vector<std::vector<double>> pi_(fil, std::vector<double>(col, 0.0));

    int block = 0;
    for (int i = 0; i < M; ++i) {
        int pos1 = 0;
        int pos2 = 0;
        for (int j = 0; j < (N * N); ++j) {
            pi_[i][j + block] = w[pos1] * w[pos2];
            if (pos2 == N - 1) {
                pos1++;
                pos2 = 0;
            } else {
                pos2++;
            }
        }
        block += (N * N);
    }

    return pi_;
}

std::vector<std::vector<double>> get_g_matrix(const std::vector<double>& z,
                                              const std::vector<double>& x,
                                              double SNR, int N, int M) {
    int fil = M;
    int col = M * (N * N);
    std::vector<std::vector<double>> G(fil, std::vector<double>(col, 0.0));

    for (int i = 0; i < M; ++i) {
        int block = 0;
        int posz1 = 0;
        int posz2 = 0;
        for (int j = 0; j < col; ++j) {
            std::complex<double> z_complex(z[posz1], z[posz2]);
            std::complex<double> diff = z_complex + std::sqrt(SNR) * x[i] - std::sqrt(SNR) * x[block];
            G[i][j] = g(diff);

            if (posz2 == N - 1) {
                if (posz1 == N - 1) {
                    block++;
                    posz1 = 0;
                    posz2 = 0;
                } else {
                    posz1++;
                    posz2 = 0;
                }
            } else {
                posz2++;
            }
        }
    }

    return G;
}

double g(const std::complex<double>& z) {
    return (1.0 / M_PI) * std::exp(-std::norm(z));
}
