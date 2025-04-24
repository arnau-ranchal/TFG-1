#ifndef E0_MATRIX_H
#define E0_MATRIX_H

#include <vector>

double E0_matrix(const std::vector<std::vector<double>>& Q,
                 const std::vector<std::vector<double>>& PI_,
                 const std::vector<std::vector<double>>& G,
                 double rho);

#endif
