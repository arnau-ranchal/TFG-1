#ifndef INTERPOLACIO_H
#define INTERPOLACIO_H

#include <vector>

void interpolacio(const std::vector<double>& x, const std::vector<double>& Q,
                  double SNR, double R, double dE_0, double dE_1,
                  double& rho, double& a, double& b, double& c);

#endif
