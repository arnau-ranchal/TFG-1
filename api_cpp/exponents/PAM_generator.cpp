#include <vector>
#include <cmath>
#include <tuple>

using namespace std;

tuple<vector<double>, vector<double>> PAM_generator(int M) {
    // Probabilities
    vector<double> q(M);
    for (int i = 0; i < M; i++) {
        q[i] = 1.0 / M;
    }

    // Symbols
    vector<double> x(M);
    double aux = -(M - 1);
    for (int i = 0; i < M; i++) {
        x[i] = aux;
        aux += 2;
    }

    // Calculate average energy
    double energy = 0;
    for (size_t i = 0; i < x.size(); i++) {
        energy += q[i] * pow(x[i], 2);
    }

    // Normalize to unit energy
    double norm_factor = sqrt(energy);
    for (auto& symbol : x) {
        symbol /= norm_factor;
    }

    return make_tuple(x, q);
}