
%% gradient_descent (amb matrius)
function rho = newton_method(x, SNR, Q, pi_, G, max_iteracions, threshold, R)
    
    [f, fp]= F0_F0p(Q, pi_, G, 0);
    dE_0 = -fp/(f*log(2)); % primera derivada

    [f, fp]= F0_F0p(Q, pi_, G, 1);
    dE_1 = -fp/(f*log(2)); % primera derivada

    if dE_0 < R 
        rho = 0;
    elseif dE_1 > R
        rho = 1;
    else
        % Interpolacio:
        % Punt 2: E(1)-R
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
        
        % Arrels de la derivada (forma manual de la fórmula)
        %rho1 = (-b + sqrt(b^2 - 4*a*c)) / (2*a)
        rho = (-b - sqrt(b^2 - 4*a*c)) / (2*a);  % No estem 100% de que sigui aquesta pero quien no arriesga no gana

%                 % Init rho
%         % A hermite_interp esta explicat amb codi com hem arribat fins a rho
%         punt1 = [0,0];
%         punt2 = [1,f2];
%         rho = hermite_interp(punt1, punt2, dp1, dp2); % Paràmetres: [E0(1) - R, E0'(0)-R, E0'(1) - R] 
%         [f, fp, f2p] = F0_matrix_fixed(Q, pi_, G, rho);
%         curvatura_fixed = -(f2p/f - (fp/f)*(fp/f))/log(2); % segona derivada FIXADA !!

        for iteration = 1:max_iteracions
            % Derivades
            [f, fp, f2p] = F0_matrix_fixed(Q, pi_, G, rho); % Això s'ha de canviar per la fixed un cop estigui fixed de veritat

            gradient = -fp/(f*log(2))-R; % primera derivada
            curvatura = -(f2p/f - (fp/f)*(fp/f))/log(2); % segona derivada

            % Update
            rho = rho - gradient/curvatura;
        
            % Threshold
            if abs(gradient/curvatura) < threshold
                break;
            end

        end
        %disp(['Num of iterations: ', num2str(iteration)]);
    end
         
    % Clipping: rho->[0, 1]
    rho = max(0, min(1, rho));

end

