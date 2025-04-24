#include "E0_matrix.h"
#include <cmath>
#include <vector>
#include <cstddef>

const double PI = 3.141592653589793;

static std::vector<std::vector<double>> elementwise_log(const std::vector<std::vector<double>>& mat) {
    std::vector<std::vector<double>> result = mat;
    for (auto& row : result)
        for (auto& val : row)
            val = std::log(val);
    return result;
}

static std::vector<std::vector<double>> elementwise_exp(const std::vector<std::vector<double>>& mat) {
    std::vector<std::vector<double>> result = mat;
    for (auto& row : result)
        for (auto& val : row)
            val = std::exp(val);
    return result;
}

static std::vector<std::vector<double>> elementwise_mult(const std::vector<std::vector<double>>& A,
                                                         const std::vector<std::vector<double>>& B) {
    std::vector<std::vector<double>> result = A;
    for (size_t i = 0; i < A.size(); ++i)
        for (size_t j = 0; j < A[0].size(); ++j)
            result[i][j] *= B[i][j];
    return result;
}

static std::vector<std::vector<double>> transpose(const std::vector<std::vector<double>>& mat) {
    size_t rows = mat.size();
    size_t cols = mat[0].size();
    std::vector<std::vector<double>> result(cols, std::vector<double>(rows));
    for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j)
            result[j][i] = mat[i][j];
    return result;
}

static std::vector<std::vector<double>> matmul(const std::vector<std::vector<double>>& A,
                                               const std::vector<std::vector<double>>& B) {
    size_t rows = A.size(), cols = B[0].size(), inner = B.size();
    std::vector<std::vector<double>> result(rows, std::vector<double>(cols, 0.0));
    for (size_t i = 0; i < rows; ++i)
        for (size_t j = 0; j < cols; ++j)
            for (size_t k = 0; k < inner; ++k)
                result[i][j] += A[i][k] * B[k][j];
    return result;
}

double E0_matrix(const std::vector<std::vector<double>>& Q,
                 const std::vector<std::vector<double>>& PI_,
                 const std::vector<std::vector<double>>& G,
                 double rho) {

    auto logG = elementwise_log(G);

    double alpha = -rho / (1.0 + rho);
    double beta = 1.0 / (1.0 + rho);

    auto G1 = elementwise_exp(G);  // G1 = exp(-rho/(1+rho) * logG)
    for (auto& row : G1)
        for (auto& val : row)
            val = std::exp(alpha * std::log(val));

    auto G2 = elementwise_exp(G);  // G2 = exp(1/(1+rho) * logG)
    for (auto& row : G2)
        for (auto& val : row)
            val = std::exp(beta * std::log(val));

    auto Qt = transpose(Q);
    auto QtG2 = matmul(Qt, G2);

    // qg2rho = (Qt * G2)^rho
    for (auto& row : QtG2)
        for (auto& val : row)
            val = std::exp(rho * std::log(val));

    auto PIxG1 = elementwise_mult(PI_, G1);
    auto QtPIxG1 = matmul(Qt, PIxG1);
    auto final_product = matmul(QtPIxG1, transpose(QtG2));

    double m = final_product[0][0];  // as all intermediate products are vectors
    return -std::log2((1.0 / PI) * m);
}
