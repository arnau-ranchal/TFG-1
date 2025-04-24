#include "PAM_generator.h"
#include <cmath>

void PAM_generator(int M, std::vector<double>& x, std::vector<double>& q) {
    q.resize(M, 1.0 / M);

    x.resize(M);
    int aux = -(M - 1);
    for (int i = 0; i < M; ++i) {
        x[i] = aux;
        aux += 2;
    }

    double energy = 0.0;
    for (int i = 0; i < M; ++i) {
        energy += q[i] * std::abs(x[i]) * std::abs(x[i]);
    }

    double scale = std::sqrt(energy);
    for (int i = 0; i < M; ++i) {
        x[i] /= scale;
    }
}
