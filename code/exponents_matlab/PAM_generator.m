function [x,q] = PAM_generator(M)
    % Probabilities
    q = zeros(1,M);
    for i = 1:M
        q(i) = 1/M;
    end

    % Symbols
    x = zeros(1,M);
    aux = -(M - 1);
    for i = 1:M
        x(i) = aux;
        aux = aux + 2;    
    end

    % Average energy
    energy = 0;
    for i = 1:length(x)
        energy = energy + q(i)*abs(x(i))^2;
    end

    % Unit energy
    x = x/sqrt(energy);
    
end
