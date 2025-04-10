// exponents.cpp
#include <cmath> // Para usar pow()
#include <cstring>  // Para strcmp

extern "C" {
    float* exponents(float M, const char* typeM, float SNR, float R, float N) {
        // Adaptar M según el tipo de modulación
        if (strcmp(typeM, "PAM") == 0) {
            M = M;
        } else if (strcmp(typeM, "QAM") == 0) {
            M = M + 1;
        } else {
            M = 0;
        }

        // Crear array estático (3 valores)
        static float results[3];

        float Pe = M * SNR * R * N;
        float exp = pow(Pe, 2);
        float rho = log(Pe);

        results[0] = Pe;
        results[1] = exp;
        results[2] = rho;

        return results;
    }
}
