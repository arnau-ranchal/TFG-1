#ifndef NEWTON_METHOD_V2_H
#define NEWTON_METHOD_V2_H

#include <vector>

double newton_method_v2(const std::vector<std::vector<double>>& Q,
                        const std::vector<double>& pi_,
                        const std::vector<std::vector<double>>& G,
                        int max_iteracions,
                        double threshold,
                        double R,
                        double rho,
                        int N,
                        const std::vector<double>& x,
                        const std::vector<double>& w,
                        const std::vector<double>& z,
                        double SNR,
                        int flag);

#endif
