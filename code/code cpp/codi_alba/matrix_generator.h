#ifndef MATRIX_GENERATOR_H
#define MATRIX_GENERATOR_H

#include <vector>

void matrix_generator(int N,
                      const std::vector<double>& q,
                      const std::vector<double>& x,
                      const std::vector<double>& w,
                      const std::vector<double>& z,
                      double SNR,
                      double clip,
                      std::vector<std::vector<double>>& Q,
                      std::vector<std::vector<double>>& pi_,
                      std::vector<std::vector<double>>& G);

#endif
