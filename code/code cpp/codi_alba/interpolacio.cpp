#include "interpolacio.h"
#include <cmath>

void interpolacio(const std::vector<double>& x, const std::vector<double>& Q,
                  double SNR, double R, double dE_0, double dE_1,
                  double& rho, double& a, double& b, double& c) {
    double sum = 0.0;
    int len = x.size();
    for (int i = 0; i < len; ++i) {
        for (int j = 0; j < len; ++j) {
            double diff = x[i] - x[j];
            sum += Q[i] * Q[j] * std::exp(-SNR * (diff * diff) / 4.0);
        }
    }

    double f2 = -std::log2(sum) - R;
    double dp1 = dE_0 - R;
    double dp2 = dE_1 - R;

    a = 3.0 * (dp2 + dp1 - 2.0 * f2);
    b = 2.0 * (3.0 * f2 - 2.0 * dp1 - dp2);
    c = dp1;

    double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0) {
        rho = 0; // o algún valor por defecto
    } else {
        rho = (-b - std::sqrt(discriminant)) / (2.0 * a);
    }
}
