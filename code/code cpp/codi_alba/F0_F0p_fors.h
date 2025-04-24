#ifndef F0_F0P_FORS_H
#define F0_F0P_FORS_H

#include <vector>

void F0_F0p_fors(double rho, int N,
                 const std::vector<double>& q,
                 const std::vector<double>& x,
                 const std::vector<double>& w,
                 const std::vector<double>& z,
                 double SNR,
                 double& f0, double& f0p);

#endif