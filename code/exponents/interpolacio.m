function [rho, a, b, c] = interpolacio(x, Q, SNR, R, dE_0, dE_1)
% IINTERPOLACIÓ - Hermites
% E0(1)
sum = 0;
for i = 1:length(x)
    for j = 1:length(x)
       sum = sum + Q(i)*Q(j)*exp(-SNR*((x(i)-x(j))^2)/4);
    end
end

f2 = -log2(sum)-R;
dp1 = dE_0 - R;
dp2 = dE_1 - R;

% Coeficients a, b, c de la derivada: a x^2 + b x + c
a = 3*(dp2 + dp1 - 2*f2);
b = 2*(3*f2 - 2*dp1 - dp2);
c = dp1;

% Arrels de la derivada
%rho = (-b + sqrt(b^2 - 4*a*c)) / (2*a)
rho = (-b - sqrt(b^2 - 4*a*c)) / (2*a);  % No estem 100% de que sigui aquesta pero quien no arriesga no gana
end