%% F0 + F0' + F0'' matrices
function [F0, dF0, d2F0] = F0_matrix_fixed(Q, pi_matrix, w_matrix, rho)
    s = 1/(1+rho);
    sp = -1/(1+rho)^2;
    spp = 2/(1+rho)^3;
    
    % Compute log(W) for numerical stability
    log_W = log(w_matrix); % Element-wise logarithm of W
    
    % Compute π .* W^{-sρ} using exponential and logarithm
    pi_term = pi_matrix .* exp(-s * rho * log_W);
    
    % Compute Q^T * W^s using logarithmic approach
    inner_exp = exp(s * log_W);    % Equivalent to W.^s
    Q_inner = Q' * inner_exp;      % Row vector (1 x n*M)
    
    % Compute (Q^T * W^s)^ρ and transpose to column vector
    log_Q_inner = log(Q_inner);    % Element-wise log
    outer_exp = exp(rho * log_Q_inner); % Equivalent to Q_inner.^rho
    outer_term = outer_exp.';      % Column vector (n*M x 1)
    
    % Final computation of F0
    F0 = Q' * pi_term * outer_term;
    
    %E0 = -log2((1/pi)*F0);
    
    % Compute the first derivative F0_prime
    % Term1: Derivative of the pi_term component
    term1_part = (-sp * rho * log_W) - (s * log_W);
    term1_matrix = pi_term .* term1_part;
    Term1 = Q' * term1_matrix * outer_term;
    
    % Term2: Derivative of the outer_term component
    inner_exp_logW = inner_exp .* log_W;             % Element-wise multiplication
    numerator = (Q' * inner_exp_logW) * sp;          % 1 x (n*M)
    d_log_Q_inner = numerator ./ Q_inner;            % Element-wise division
    derivative_outer_part = log_Q_inner + rho * d_log_Q_inner;
    d_outer_exp = outer_exp .* derivative_outer_part;% Apply derivative to outer_exp
    d_outer_term = d_outer_exp.';                    % Transpose to column vector
    Term2 = Q' * pi_term * d_outer_term;
    
    % Total derivative F0_prime
    dF0 = Term1 + Term2;
    
    % Compute dE0 using F0 and F0_prime (derivative of E0)
    %dE0 = -dF0 / (F0 * log(2));  % log(2) is natural logarithm of 2
    
    % Compute the second derivative F0_double_prime
    % Precompute reused terms
    part1 = (-sp * rho * log_W) - s * log_W;                   % From first derivative
    part1_squared = part1.^2;
    part2 = (-spp * rho * log_W) - 2 * sp * log_W;             % s'' = 2/(1+rho)^3
    
    % Term 1: Q^T * (π ⋅ e^{-sρlnW} ⋅ (-s'ρlnW - slnW)^2) * outer_term
    term1_matrix = pi_term .* part1_squared;
    Term1 = Q' * term1_matrix * outer_term;
    
    % Term 2: Q^T * (π ⋅ e^{-sρlnW} ⋅ (-s''ρlnW - 2s'lnW)) * outer_term
    term2_matrix = pi_term .* part2;
    Term2 = Q' * term2_matrix * outer_term;
    
    % Term 3: 2 * Q^T * (π ⋅ e^{-sρlnW} ⋅ part1) * d_outer_term (from first derivative)
    Term3 = 2 * (Q' * (pi_term .* part1)) * d_outer_term;
    
    % Term 4: Q^T * π_term * (outer_exp ⋅ (derivative_outer_part)^2)
    derivative_outer_part_squared = derivative_outer_part.^2;  % derivative_outer_part from first derivative
    Term4_inner = outer_exp .* derivative_outer_part_squared;
    Term4 = Q' * pi_term * Term4_inner.';
    
    % Term 5: Q^T * π_term * (outer_exp ⋅ [component1 + component2 + component3])
    inner_exp_logW = inner_exp .* log_W;                        % From first derivative
    numerator_component1 = Q' * inner_exp_logW;
    component1_part = sp * (numerator_component1 ./ Q_inner);
    component1 = 2 * component1_part;
    
    log_W_squared = log_W.^2;                                   % Element-wise square
    spp_term = spp * log_W;
    sp_squared_term = sp^2 * log_W_squared;
    combined_spp_part = spp_term + sp_squared_term;
    inner_exp_spp_part = combined_spp_part .* inner_exp;        % Element-wise multiply
    numerator_component2 = Q' * inner_exp_spp_part;
    component2 = rho * (numerator_component2 ./ Q_inner);
    component3 = -rho * (component1_part.^2);                   % Squared term
    
    Term5_inner = component1 + component2 + component3;
    Term5_matrix = outer_exp .* Term5_inner;
    Term5 = Q' * pi_term * Term5_matrix.';
    
    % Sum all terms for F0_double_prime
    d2F0 = Term1 + Term2 + Term3 + Term4 + Term5;
    
    % Compute d2E0 using F0, F0_prime, and F0_double_prime
    %d2E0 = -(d2F0 * F0 - dF0^2) / (F0^2 * log(2));
    
    end
