%% NEWTON METHOD (amb matrius)
function rho = newton_method_v2(Q, pi_, G, max_iteracions, threshold, R, rho, N, x, w, z, SNR, flag)

    if flag == 1 %---------------------------------------------------------FORS CASE
        % STEP SIZE - curvatura
        q = Q.';
        [f, fp, f2p] = F0_fors(rho, N, q, x, w, z, SNR); 

        curvatura_fixada = -(f2p/f - (fp/f)*(fp/f))/log(2); % segona derivada FIXADA !!

        for iteration = 1:max_iteracions
            % GRADIENT
            [f, fp] = F0_F0p_fors(rho, N, q, x, w, z, SNR);
            gradient = -fp/(f*log(2))-R; 
    
            % Update rho
            rho = rho - gradient/curvatura_fixada;
        
            % Apply threshold
            if abs(gradient/curvatura_fixada) < threshold
                break;
            end

        end
        %disp(['Num of iterations: ', num2str(iteration)]);
             
        % Clipping: rho->[0, 1]
        rho = max(0, min(1, rho));

    else %---------------------------------------------------------------- MATRICES CASE
        % STEP SIZE - curvatura
        [f, fp, f2p] = F0_matrix_fixed(Q, pi_, G, rho);
        curvatura_fixada = -(f2p/f - (fp/f)*(fp/f))/log(2); % segona derivada FIXADA !!

        for iteration = 1:max_iteracions
            % GRADIENT
            [f, fp] = F0_F0p_matrix(Q, pi_, G, rho);
            gradient = -fp/(f*log(2))-R; 
    
            % Update rho
            rho = rho - gradient/curvatura_fixada;
        
            % Apply threshold
            if abs(gradient/curvatura_fixada) < threshold
                break;
            end

        end
        %disp(['Num of iterations: ', num2str(iteration)]);
             
        % Clipping: rho->[0, 1]
        rho = max(0, min(1, rho));

    end 


end

