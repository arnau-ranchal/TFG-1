#include "F0_matrix_fixed.h"
#include <vector>
#include <cmath>

const double PI = 3.14159265358979323846;

void F0_matrix_fixed(const std::vector<std::vector<double>>& Q,
                     const std::vector<std::vector<double>>& pi_matrix,
                     const std::vector<std::vector<double>>& w_matrix,
                     double rho,
                     double& F0,
                     double& dF0,
                     double& d2F0) {
    int M = Q.size();
    int K = pi_matrix[0].size(); // number of columns (should match w_matrix)

    // Precompute factors
    double s   = 1.0 / (1.0 + rho);
    double sp  = -1.0 / std::pow(1.0 + rho, 2);
    double spp =  2.0 / std::pow(1.0 + rho, 3);

    // Allocate intermediate matrices/vectors
    std::vector<std::vector<double>> log_W(M, std::vector<double>(K));
    std::vector<std::vector<double>> pi_term(M, std::vector<double>(K));
    std::vector<std::vector<double>> inner_exp(M, std::vector<double>(K));

    // Compute log_W, pi_term, inner_exp
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < K; ++j) {
            log_W[i][j]    = std::log(w_matrix[i][j]);
            pi_term[i][j]  = pi_matrix[i][j] * std::exp(-s * rho * log_W[i][j]);
            inner_exp[i][j]= std::exp(s * log_W[i][j]);
        }
    }

    // Compute Q_inner = Q' * inner_exp  (1 x K)
    std::vector<double> Q_inner(K, 0.0);
    for (int j = 0; j < K; ++j) {
        for (int i = 0; i < M; ++i) {
            Q_inner[j] += Q[i][0] * inner_exp[i][j];
        }
    }

    // Compute log_Q_inner, outer_exp, outer_term
    std::vector<double> log_Q_inner(K), outer_exp(K), outer_term(K);
    for (int j = 0; j < K; ++j) {
        log_Q_inner[j] = std::log(Q_inner[j]);
        outer_exp[j]   = std::exp(rho * log_Q_inner[j]);
        outer_term[j]  = outer_exp[j];
    }

    // Helper: compute vector t = Q' * A for any A (M x K) -> t (1 x K)
    auto mult_Qt = [&](const std::vector<std::vector<double>>& A) {
        std::vector<double> t(K, 0.0);
        for (int j = 0; j < K; ++j)
            for (int i = 0; i < M; ++i)
                t[j] += Q[i][0] * A[i][j];
        return t;
    };

    // F0 = (1/pi) * (Q' * pi_term * outer_term)
    std::vector<double> t0 = mult_Qt(pi_term);
    F0 = 0.0;
    for (int j = 0; j < K; ++j)
        F0 += t0[j] * outer_term[j];
    F0 /= PI;

    // First derivative dF0
    // Term1
    std::vector<std::vector<double>> term1_mat(M, std::vector<double>(K));
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < K; ++j) {
            double part = (-sp * rho * log_W[i][j]) - (s * log_W[i][j]);
            term1_mat[i][j] = pi_term[i][j] * part;
        }
    std::vector<double> T1 = mult_Qt(term1_mat);

    // Term2
    // inner_exp_logW = inner_exp .* log_W
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
        dF0 += T2[j] * d_outer_exp[j];
    // combine term1 and term2
    for (int j = 0; j < K; ++j)
        dF0 += T1[j] * outer_term[j];
    dF0 /= PI;

    // Second derivative d2F0
    // Precompute part1, part1_squared, part2
    std::vector<std::vector<double>> part1(M, std::vector<double>(K));
    std::vector<std::vector<double>> part1_sq(M, std::vector<double>(K));
    std::vector<std::vector<double>> part2(M, std::vector<double>(K));
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < K; ++j) {
            double p1 = (-sp * rho * log_W[i][j]) - (s * log_W[i][j]);
            part1[i][j] = p1;
            part1_sq[i][j] = p1 * p1;
            double p2 = (-spp * rho * log_W[i][j]) - (2.0 * sp * log_W[i][j]);
            part2[i][j] = p2;
        }
    }

    // Term1_sec
    std::vector<std::vector<double>> mat1_sec(M, std::vector<double>(K));
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < K; ++j)
            mat1_sec[i][j] = pi_term[i][j] * part1_sq[i][j];
    std::vector<double> T1_sec = mult_Qt(mat1_sec);

    // Term2_sec
    std::vector<std::vector<double>> mat2_sec(M, std::vector<double>(K));
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < K; ++j)
            mat2_sec[i][j] = pi_term[i][j] * part2[i][j];
    std::vector<double> T2_sec = mult_Qt(mat2_sec);

    // Term3 = 2 * Q' * (pi_term .* part1) * d_outer_term
    std::vector<std::vector<double>> pi_part1(M, std::vector<double>(K));
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < K; ++j)
            pi_part1[i][j] = pi_term[i][j] * part1[i][j];
    std::vector<double> T3_vec = mult_Qt(pi_part1);
    double T3 = 0.0;
    for (int j = 0; j < K; ++j)
        T3 += T3_vec[j] * d_outer_exp[j];
    T3 *= 2.0;

    // Term4 = Q' * pi_term * (outer_exp .* (derivative_outer_part)^2)
    std::vector<double> derivative_outer_sq(K);
    for (int j = 0; j < K; ++j)
        derivative_outer_sq[j] = derivative_outer_part[j] * derivative_outer_part[j];
    std::vector<double> T4_vec(K);
    for (int j = 0; j < K; ++j)
        T4_vec[j] = outer_exp[j] * derivative_outer_sq[j];
    std::vector<double> T4_mult = mult_Qt(pi_term);
    double T4 = 0.0;
    for (int j = 0; j < K; ++j)
        T4 += T4_mult[j] * T4_vec[j];

    // Term5
    // component1_part already related to num and Q_inner
    std::vector<double> comp1(K), comp2(K), comp3(K);
    for (int j = 0; j < K; ++j)
        comp1[j] = 2.0 * (sp * (num[j] / Q_inner[j]));

    // inner_exp_spp_part
    std::vector<std::vector<double>> inner_spp(M, std::vector<double>(K));
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < K; ++j) {
            double term_spp = spp * log_W[i][j];
            double term_sp2 = sp * sp * log_W[i][j] * log_W[i][j];
            inner_spp[i][j] = (term_spp + term_sp2) * inner_exp[i][j];
        }
    std::vector<double> num2 = mult_Qt(inner_spp);
    for (int j = 0; j < K; ++j)
        comp2[j] = rho * (num2[j] / Q_inner[j]);

    for (int j = 0; j < K; ++j)
        comp3[j] = -rho * ((sp * (num[j] / Q_inner[j])) * (sp * (num[j] / Q_inner[j])));

    double T5 = 0.0;
    for (int j = 0; j < K; ++j) {
        double inner = comp1[j] + comp2[j] + comp3[j];
        double term5_val = outer_exp[j] * inner;
        // multiply by Q' * pi_term: use t0 from earlier
        T5 += t0[j] * term5_val;
    }

    double T1 = 0.0, T2 = 0.0;
    for (int j = 0; j < K; ++j) {
        T1 += T1_sec[j] * outer_term[j];
        T2 += T2_sec[j] * outer_term[j];
    }

    // Sum all for second derivative
    d2F0 = (T1 + T2 + T3 + T4 + T5) / PI;

}
