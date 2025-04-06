
%% gradient_descent (amb matrius)
function rho = gradient_ascent(Q, pi_, G, rho, alpha, max_iteracions, threshold, R)
    for iteration = 1:max_iteracions
        % Gradient
        gradient = derivada_A_matrix(Q, pi_, G, rho, R);
       
        % Update
        rho = rho + alpha * gradient; % Ara ascent
        
        % Threshold
        if abs(alpha * gradient) < threshold
            break;
        end

    end
         
    % Clipping: rho->[0, 1]
    rho = max(0, min(1, rho));
    disp(iteration);
end

