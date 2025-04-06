function [exponents, rhos] = exponents(SNR, R, M, threshold)
    exponents = SNR + R;
    rhos = M + threshold;
