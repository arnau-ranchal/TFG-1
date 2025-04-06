%% E0 function
function result = E0(rho, N, q, x, w, z, SNR)
    sumTotal = 0.0;
    for index = 1:length(x)
        sumik = 0.0;
        for i = 1:N
            for k = 1:N
                fpResult = fp(x, index, q, z(i)+(1i*z(k)), rho, SNR);
                sumik = sumik + w(i)*w(k)*(fpResult)^rho; % exp(rho*log(fpresult))
            end
        end
        sumTotal = sumTotal + q(index)*sumik;
    end
    result = -log2((1/pi)*sumTotal);
end

%% fp function
function result = fp(x, index, q, z, rho, SNR)
    sum = 0.0;
    for barret = 1:length(x)
        gnum = G(z + sqrt(SNR)*x(index) - sqrt(SNR)*x(barret));
        sum = sum + q(barret)*gnum^(1/(1+rho)); % exp((1/(1+rho))*log(gnum))
    end
    den = G(z)^(1/(1+rho)); % exp((1/(1+rho))*log(G(z)))
    result = sum/den;
end

%% G function
function result = G(z)
    result = (1 / pi) * exp(-abs(z)^2);
end
