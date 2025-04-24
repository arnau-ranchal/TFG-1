#ifndef F0_MATRIX_FIXED_H
#define F0_MATRIX_FIXED_H

#include <vector>

/**
 * Computes F0, its first and second derivatives dF0, d2F0 using matrix-based expressions.
 *
 * Q          : matrix of size (M x 1)
 * pi_matrix  : matrix of size (M x K)
 * w_matrix   : matrix of size (M x K)
 * rho        : scalar parameter
 * F0         : output scalar F0
 * dF0        : output scalar, first derivative
 * d2F0       : output scalar, second derivative
 */
void F0_matrix_fixed(const std::vector<std::vector<double>>& Q,
                     const std::vector<std::vector<double>>& pi_matrix,
                     const std::vector<std::vector<double>>& w_matrix,
                     double rho,
                     double& F0,
                     double& dF0,
                     double& d2F0);

#endif // F0_MATRIX_FIXED_H

