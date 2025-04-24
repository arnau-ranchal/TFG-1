#include "F0_F0p_matrix.h"
#include <cmath>

const double PI = 3.14159265358979323846;

void F0_F0p_matrix(const std::vector<std::vector<double>>& Q,
                   const std::vector<std::vector<double>>& pi_matrix,
                   const std::vector<std::vector<double>>& w_matrix,
                   double rho,
                   double& F0,
                   double& dF0) {

    int M = Q.size();
    int K = pi_matrix[0].size();

    double s = 1.0 / (1.0 + rho);
    double sp = -1.0 / std::pow(1.0 + rho, 2);

    std::vector<std::vector<double>> log_W(M, std::vector<double>(K));
    std::vector<std::vector<double>> pi_term(M, std::vector<double>(K));
    std::vector<std::vector<double>> inner_exp(M, std::vector<double>(K));

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < K; ++j) {
            log_W[i][j] = std::log(w_matrix[i][j]);
            pi_term[i][j] = pi_matrix[i][j] * std::exp(-s * rho * log_W[i][j]);
            inner_exp[i][j] = std::exp(s * log_W[i][j]);
        }
    }

    std::vector<double> Q_inner(K, 0.0);
    for (int j = 0; j < K; ++j)
        for (int i = 0; i < M; ++i)
            Q_inner[j] += Q[i][0] * inner_exp[i][j];

    std::vector<double> log_Q_inner(K), outer_exp(K), outer_term(K);
    for (int j = 0; j < K; ++j) {
        log_Q_inner[j] = std::log(Q_inner[j]);
        outer_exp[j] = std::exp(rho * log_Q_inner[j]);
        outer_term[j] = outer_exp[j];
    }

    auto mult_Qt = [&](const std::vector<std::vector<double>>& A) {
        std::vector<double> t(K, 0.0);
        for (int j = 0; j < K; ++j)
            for (int i = 0; i < M; ++i)
                t[j] += Q[i][0] * A[i][j];
        return t;
    };

    std::vector<double> t0 = mult_Qt(pi_term);
    F0 = 0.0;
    for (int j = 0; j < K; ++j)
        F0 += t0[j] * outer_term[j];

    std::vector<std::vector<double>> term1_mat(M, std::vector<double>(K));
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < K; ++j) {
            double part = (-sp * rho * log_W[i][j]) - (s * log_W[i][j]);
            term1_mat[i][j] = pi_term[i][j] * part;
        }
    std::vector<double> T1 = mult_Qt(term1_mat);

    std::vector<std::vector<double>> inner_exp_logW(M, std::vector<double>(K));
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < K; ++j)
            inner_exp_logW[i][j] = inner_exp[i][j] * log_W[i][j];

    std::vector<double> num = mult_Qt(inner_exp_logW);
    for (int j = 0; j < K; ++j)
        num[j] *= sp;

    std::vector<double> d_log_Q_inner(K), derivative_outer_part(K), d_outer_exp(K);
    for (int j = 0; j < K; ++j) {
        d_log_Q_inner[j] = num[j] / Q_inner[j];
        derivative_outer_part[j] = log_Q_inner[j] + rho * d_log_Q_inner[j];
        d_outer_exp[j] = outer_exp[j] * derivative_outer_part[j];
    }

    std::vector<double> T2 = mult_Qt(pi_term);
    dF0 = 0.0;
    for (int j = 0; j < K; ++j)
        dF0 += T1[j] * outer_term[j] + T2[j] * d_outer_exp[j];
}