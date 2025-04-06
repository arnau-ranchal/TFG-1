#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <boost/math/special_functions/hermite.hpp>
#include <boost/math/quadrature/gauss_hermite.hpp>

using namespace std;
using namespace boost::math;

// Función para calcular puntos y pesos de Gauss-Hermite
tuple<vector<double>, vector<double>> GaussHermite_Locations_Weights(int n) {
    if (n <= 0) {
        throw invalid_argument("El orden n debe ser positivo");
    }

    // Usamos la implementación de Boost si está disponible
    if (n <= 20) { // Límite práctico para Boost
        auto gh = gauss_hermite<double, 20>(n);
        vector<double> xi(gh.abscissas().begin(), gh.abscissas().end());
        vector<double> wi(gh.weights().begin(), gh.weights().end());
        return make_tuple(xi, wi);
    }

    // Implementación manual para n > 20
    vector<double> xi(n), wi(n);
    
    // Estimación inicial de raíces (aproximación asintótica)
    for (int i = 0; i < n; ++i) {
        double x = sqrt(2.0 * n + 1.0) * cos(M_PI * (4.0 * (i + 1) - 1.0) / (4.0 * n + 2.0));
        xi[i] = x;
    }

    // Refinamiento con Newton-Raphson
    const double eps = 1e-15;
    for (int k = 0; k < n; ++k) {
        double x = xi[k];
        double delta;
        int iterations = 0;
        const int max_iter = 100;
        
        do {
            double Hn = hermite(n, x);
            double Hn_1 = hermite(n-1, x);
            delta = Hn / (2.0 * n * Hn_1);
            x -= delta;
            iterations++;
        } while (fabs(delta) > eps && iterations < max_iter);
        
        xi[k] = x;
        
        // Calcular pesos
        double Hn_1 = hermite(n-1, x);
        wi[k] = pow(2.0, n-1) * tgamma(n+1) * sqrt(M_PI) / 
                pow(n * Hn_1, 2);
    }

    return make_tuple(xi, wi);
}

// Programa de prueba
int main() {
    try {
        int n;
        cout << "Introduce el orden n de la cuadratura de Gauss-Hermite: ";
        cin >> n;

        auto [xi, wi] = GaussHermite_Locations_Weights(n);

        cout << "\nPuntos de cuadratura (xi) y pesos (wi):\n";
        cout << "------------------------------------------------\n";
        cout << "   i\t\txi\t\t\twi\n";
        cout << "------------------------------------------------\n";
        
        cout << fixed;
        cout.precision(15);
        
        for (int i = 0; i < n; ++i) {
            cout << "  " << i+1 << "\t" << xi[i] << "\t" << wi[i] << endl;
        }

        // Verificación: suma de pesos debería ser ≈ √π
        double sum_wi = accumulate(wi.begin(), wi.end(), 0.0);
        cout << "\nSuma de pesos: " << sum_wi << " (teórico: " << sqrt(M_PI) << ")\n";

        // Verificación: integral de x² debería ser ≈ √π/2
        double integral = 0.0;
        for (int i = 0; i < n; ++i) {
            integral += wi[i] * xi[i] * xi[i];
        }
        cout << "Integral de x²: " << integral << " (teórico: " << sqrt(M_PI)/2 << ")\n";

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}