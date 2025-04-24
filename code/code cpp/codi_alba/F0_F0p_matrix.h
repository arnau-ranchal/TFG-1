#ifndef F0_F0P_MATRIX_H
#define F0_F0P_MATRIX_H

#include <vector>

void F0_F0p_matrix(const std::vector<std::vector<double>>& Q,
                   const std::vector<std::vector<double>>& pi_matrix,
                   const std::vector<std::vector<double>>& w_matrix,
                   double rho,
                   double& F0,
                   double& dF0);

#endif