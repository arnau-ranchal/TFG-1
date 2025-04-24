#ifndef E0_FORS_V2_H
#define E0_FORS_V2_H

#include <vector>

double E0_fors_v2(double rho, int N,
                  const std::vector<double>& q,
                  const std::vector<double>& x,
                  const std::vector<double>& w,
                  const std::vector<double>& z,
                  double SNR);

#endif