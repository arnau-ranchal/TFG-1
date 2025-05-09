%% F0 + F0' + F0''
function derivada = derivada_A(rho, N, q, x, w, z, SNR)
    f0 = 0.0; f0p = 0; %f02p = 0;
    for index = 1:length(x)
        sumF0ik = 0.0; sumF0pik = 0.0; %sumF02pik = 0.0;
        for i = 1:N
            for k = 1:N
                fpResult = fp(x, index, q, z(i)+(1i*z(k)), rho, SNR);
                sumF0ik = sumF0ik + w(i)*w(k)*(fpResult)^rho;
                sumF0pik = sumF0pik + w(i)*w(k)*(fpResult)^rho * log(fpResult);
                %sumF02pik = sumF02pik + w(i)*w(k)*(fpResult)^rho * log(fpResult) * log(fpResult);
            end
        end
        f0 = f0 + q(index)*sumF0ik;
        f0p = f0p + q(index)*sumF0pik;
        %f02p = f02p + q(index)*sumF02pik;
    end
    f0 = (1/pi)*f0; f0p = (1/pi)*f0p; %f02p = (1/pi)*f02p;
    derivada = - f0p / (f0 * log(2)) - R;
end

%% fp function
function result = fp(x, index, q, z, rho, SNR)
    sum = 0.0;
    for barret = 1:length(x)
        gnum = G(z + sqrt(SNR)*x(index) - sqrt(SNR)*x(barret));
        sum = sum + q(barret)*gnum^(1/(1+rho));
    end
    den = G(z)^(1/(1+rho));
    result = sum/den;
end

%% G function
function result = G(z)
    result = (1 / pi) * exp(-abs(z)^2);
end

%% F0 + F0' + F0''
function derivada = derivada_F0(rho, N, q, x, w, z, SNR)
    f0 = 0.0; f0p = 0; f02p = 0;
    for index = 1:length(x)
        sumF0pik = 0.0;
        for i = 1:N
            for k = 1:N
                fpResult = fp(x, index, q, z(i)+(1i*z(k)), rho, SNR);
                sumF0pik = sumF0pik + w(i)*w(k)*(fpResult)^rho * log(fpResult);
            end
        end
        f0p = f0p + q(index)*sumF0pik;
    end
    derivada = (1/pi)*f0p;
end

%% fp function
function result = fp(x, index, q, z, rho, SNR)
    sum = 0.0;
    for barret = 1:length(x)
        gnum = G(z + sqrt(SNR)*x(index) - sqrt(SNR)*x(barret));
        sum = sum + q(barret)*gnum^(1/(1+rho));
    end
    den = G(z)^(1/(1+rho));
    result = sum/den;
end

%% G function
function result = G(z)
    result = (1 / pi) * exp(-abs(z)^2);
end

