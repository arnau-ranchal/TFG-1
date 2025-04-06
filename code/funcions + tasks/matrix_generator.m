%% Matrix generator
function [Q, pi_, G] = matrix_generator(N, q, x, w, z, SNR)
    M = length(x);
    % Obtenir matrius
    Q = get_q_matrix(q);
    pi_ = get_pi_matrix(w, N, M);
    G = get_g_matrix(z, x, SNR, N, M);
end

%% Matriu Q
function Q = get_q_matrix(q)
    Q = q.'; %Transpose
end

%% Matriu PI
function pi_ = get_pi_matrix(w, N, M)
    fil = M;
    col = M*(N^2);
    pi_ = zeros(fil, col);
    block = 0;
    for i = 1:M     %files
        pos1 = 1;
        pos2 = 1;
        for j = 1:(N^2)     %columnes no buides
            pi_(i,j + block) = w(pos1)*w(pos2);
            if pos2 == N
                pos1 = pos1 + 1;
                pos2 = 1;
            else
                pos2 = pos2 + 1;
            end 
        end
        block = block + (N^2);
    end
end

%% Matriu G
function G = get_g_matrix(z, x, SNR, N, M)
    fil = M;
    col = M*(N^2);
    G = zeros(fil, col);
    for i = 1:M
        block = 1;
        posz1 = 1;
        posz2 = 1;
        for j = 1:col
            G(i, j) = g(z(posz1) + 1i*z(posz2) + sqrt(SNR)*x(i) - sqrt(SNR)*x(block));
            if posz2 == N
                if posz1 == N
                    block = block + 1;
                    posz1 = 1;
                    posz2 = 1;
                else
                    posz1 = posz1 + 1;
                    posz2 = 1;
                end
            else
                posz2 = posz2 + 1;
            end 
        end
    end
end

%% G function
function result = g(z)
    result = (1 / pi) * exp(-abs(z)^2);
end
