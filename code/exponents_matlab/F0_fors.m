%% F0 + F0' + F0''
function [f0, f0p, f02p] = F0_fors(rho, N, q, x, w, z, SNR)
    f0 = 0.0; f0p = 0; f02p = 0;
    for index = 1:length(x)
        sumF0ik = 0.0; sumF0pik = 0.0; sumF02pik = 0.0;
        for i = 1:N
            for k = 1:N
                [fpResult, fptildeResult,fpt2] = fp(x, index, q, z(i)+(1i*z(k)), rho, SNR);
               
                sumF0ik = sumF0ik + w(i)*w(k)*(fpResult)^rho;
                sumF0pik = sumF0pik + w(i)*w(k)*(fpResult)^rho * (log(fpResult) - rho/(1+rho)^2 * fptildeResult / fpResult);
                sumF02pik = sumF02pik + w(i)*w(k)*(fpResult)^rho * ((log(fpResult) - rho/(1+rho)^2 * fptildeResult/fpResult)^2 - rho/(1+rho)^4 * (fptildeResult/fpResult)^2 - 2/(1+rho)^3*(fptildeResult/fpResult) - rho/(1+rho)^2 * (fpt2/fpResult)) ;
            end
        end
        f0 = f0 + q(index)*sumF0ik;
        f0p = f0p + q(index)*sumF0pik;
        f02p = f02p + q(index)*sumF02pik;
    end
    f0 = (1/pi)*f0; f0p = (1/pi)*f0p; f02p = (1/pi)*f02p;
end

%% fp function
function [result, result2, result3] = fp(x, index, q, z, rho, SNR)
    sum = 0.0;
    sum2 = 0.0;
    sum3 = 0.0;
    for barret = 1:length(x)
        gnum = G(z + sqrt(SNR)*x(index) - sqrt(SNR)*x(barret));
        sum = sum + q(barret)*gnum^(1/(1+rho));
        sum2 = sum2 + q(barret)*gnum^(1/(1+rho)) * log(gnum/G(z));
        sum3 = sum3 + q(barret)*gnum^(1/(1+rho)) * log(gnum/G(z)) * log(gnum/G(z)) * (-1/(1+ rho)^2);
    end
    den = G(z)^(1/(1+rho));
    result = sum/den;
    result2 = sum2/den;
    result3 = sum3/den;
    
end

%% G function
function result = G(z)
    result = (1 / pi) * exp(-abs(z)^2);
end

