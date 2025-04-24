function [exponents, rho_opt] = exponents(SNR, R, M, threshold)
    %EXPONENTS Compute error probability
    %
    %   The function computes E(R) = max{E0(rho) - rho*R} over 0 <= rho <= 1, where:
    %   - E0(rho) is the exponent
    %   - rho is the optimization parameter
    %
    %   Inputs:
    %       SNR       : Signal-to-Noise Ratio values (linear scale array)
    %       R         : Target rate values (bits/channel use array)
    %       M         : Modulation orders (positive integer array)
    %       threshold : Convergence criteria for rho optimization in the newton method(scalar)
    %
    %   Outputs:
    %       Pe             : Exponent
    %       rho_opt        : Optimal rho maximizing E(R)
    %       exponent_aprox : Approximated exponent E(R)
    %
    %  

    % TODO: Acabar de fer bé la estimació del temps
    % TODO: N estimation --> Arnau


    % N Estimation 
    newton_flag = 0;
    if threshold <= 2
        N = 20; 
    else
        N = get_N(threshold);       % Això encara no està
        newton_flag = 1;
    end

%%%%%%%%%%%%%%%%%%%%%%%%%%%% TIME ESTIMATION %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    % --- This is an initial guess ----
    time = 1.809561372*(1/(1000*1000)); % Time from excel sheet
    total_estimated_time = 0;
    for i = 1:length(M)
        for j = 1:length(N)
            total_estimated_time = total_estimated_time + time*(M(i)*M(i)*N(j)*N(j));
        end
    end
    
    total_estimated_time = total_estimated_time * length(SNR) * length(R);
    
    % Convert total time to integer seconds 
    total_seconds = ceil(total_estimated_time);
    
    % Calculate hours, minutes, seconds
    hours = floor(total_seconds / 3600);
    remaining_seconds = mod(total_seconds, 3600);
    minutes = floor(remaining_seconds / 60);
    seconds = mod(remaining_seconds, 60);
    
    % Format the time display
    if hours > 0
        time_display = sprintf('%d h %d min %d sec', hours, minutes, seconds);
    elseif minutes > 0
        time_display = sprintf('%d min %d sec', minutes, seconds);
    else
        time_display = sprintf('%d sec', seconds);
    end

    rem_estimated_time = total_estimated_time;

    % BAR
    progress = 0;
    h = waitbar(progress, sprintf(' Remaining time: %s', time_display));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    % INIT PARAMETERS 
    rho_opt = zeros(length(SNR), length(R), length(M));
    exponents = zeros(length(SNR), length(R), length(M));

    % Param NEWTON METHOD v2
    % Iterations
    max_iterations = 10; % Això ho he decidit yo per la cara...
    
    % Clipping
    clip = 700;
    
    % PC MEMORY:
    [~, sys] = memory;
    memAvailable = sys.PhysicalMemory.Available;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    for snr = 1:length(SNR)
        % From dB to lineal
        SNR(snr) = 10^(SNR(snr)/10);

        for n = 1:length(N)
            [z, w] = GaussHermite_Locations_Weights(N(n));

            for m = 1:length(M)
                
                for_flag = 0;

                % Symbols and probabilities generation for a PAM const
                [x, q] = PAM_generator(M(m));

                % Check if we have enough space for matrices
                current_time = (M(m)*M(m)*N(n)*N(n));
                if current_time>memAvailable % ----------------------------------------------------> FORS
                    
                    % Update waitbar wiht a for message
                    warning = 'Low memory, estimated time may increase.';
                    message = sprintf('%s\nRemaining time: %s', warning, time_display);

                    waitbar(progress, h, message);  

                    
                    for_flag = 1;

                    % GRADIENTS
                        % Gradient E0 at 0 
                    [f, fp]= F0_F0p_fors(0, N(n), q, x, w, z, SNR(snr));
                    dE_0 = -fp/(f*log(2)); 
                        % Gradient E0 at 1
                    [f, fp]= F0_F0p_fors(1, N(n), q, x, w, z, SNR(snr));
                    dE_1 = -fp/(f*log(2)); 

                else % ----------------------------------------------------------------------------> MATRICES

                    % Quadarture and weights generation
                    % Matrices: Q, PI_, G
                    [Q, PI_, G] = matrix_generator(N(n), q, x, w, z, SNR(snr), clip);

                    % GRADIENTS
                        % Gradient E0 at 0 
                    [f, fp]= F0_F0p_matrix(Q, PI_, G, 0);
                    dE_0 = -fp/(f*log(2)); 
                        % Gradient E0 at 1
                    [f, fp]= F0_F0p_matrix(Q, PI_, G, 1);
                    dE_1 = -fp/(f*log(2)); 

                end

                for r = 1:length(R) 

                    % Update waitbar wiht a for message
                    warning = 'Searching in the database...';
                    message = sprintf('%s\nRemaining time: %s', warning, time_display);

                    waitbar(progress, h, message);  

                    
                    % SEEK IN THE DATA BASE
                    data = getAwsData(M(m), N, SNR(snr), R(r));
                    disp(data.status);
                    
                    if data.status == "success"
                        disp('Data recovered from AWS.');

                        E = data.items.errorExponent;
                        rho = data.items.optimalRho;
                    else
                        
                        disp('Data not found in the server.');

                        if for_flag == 1 % --------------------------------------------------------------------> FORS
    
                            Q = q.';

                            % CHECK IF NEWTONS METHOD IS NEEDED:
                            if dE_0 < R(r)         % Case 1
                                rho = 0;
                                E = 0;
                            elseif dE_1 > R(r)     % Case 2
                                rho = 1;
                                % E0(1)
                                sum = 0;
                                for i = 1:length(x)
                                    for j = 1:length(x)
                                       sum = sum + Q(i)*Q(j)*exp(-SNR(snr)*((x(i)-x(j))^2)/4);
                                    end
                                end
                                E = -log2(sum) - R(r);

                            else                   % Case 3     
                                % IINTERPOLACIÓ
                                [init_rho, a, b, c] = interpolacio(x, Q, SNR(snr), R(r), dE_0, dE_1);
    
                                if newton_flag == 1
                                    % NEWTON METHOD
                                    rho = newton_method_v2(Q, [], [], max_iterations, threshold, R(r), init_rho, N(n), x, w, z, SNR(snr), for_flag); 
                                    % Exponent
                                    E = E0_fors_v2(rho, N(n), q, x, w, z, SNR(snr)) - rho*R(r);
    
                                else
                                    % Exponent aprox for low accuracy
                                    rho = init_rho;
                                    E = a*init_rho^3/3 + b*init_rho^2/2 + c*init_rho;
                                end
    
                            end
                            
    
                        else % -----------------------------------------------------------------------------> MATRICES
                
                            % CHECK IF NEWTONS METHOD IS NEEDED:
                            if dE_0 < R(r)         % Case 1
                                rho = 0;
                                E = 0;
                            elseif dE_1 > R(r)     % Case 2
                                rho = 1;
                                % E0(1)
                                sum = 0;
                                for i = 1:length(x)
                                    for j = 1:length(x)
                                       sum = sum + Q(i)*Q(j)*exp(-SNR(snr)*((x(i)-x(j))^2)/4);
                                    end
                                end
                                E = -log2(sum) - R(r);
    
                            else                    % Case 3     
                                % IINTERPOLACIÓ
                                [init_rho, a, b, c] = interpolacio(x, Q, SNR(snr), R(r), dE_0, dE_1);
    
                                if newton_flag == 1
                                    % NEWTON METHOD
                                    rho = newton_method_v2(Q, PI_, G, max_iterations, threshold, R(r), init_rho, [], [], [], [], [], for_flag); 
                                    % Exponent
                                    E = E0_matrix(Q, PI_, G, rho) - rho*R(r);

                                else
                                    % Exponent aprox for low accuracy
                                    rho = init_rho;
                                    E = a*init_rho^3/3 + b*init_rho^2/2 + c*init_rho;
    
                                end
                            
                            end
    
                        end   

                    end
                    
                    % Save variables in a matrix
                    rho_opt(snr, r, m, n) = rho; 
                    exponents(snr, r, m, n) = E;
    
                    % ----------------------------------BAR------------------------------------
                    
                    % REMAINING ESTIMATED TIME
                    aux_time = time*current_time;
                    rem_estimated_time = rem_estimated_time - aux_time;

                    total_seconds = ceil(max(rem_estimated_time, 0));

                    % Calculate time components
                    hours = floor(total_seconds / 3600);
                    remaining_seconds = mod(total_seconds, 3600);
                    minutes = floor(remaining_seconds / 60);
                    seconds = mod(remaining_seconds, 60);

                    % Format time display
                    if hours > 0
                        time_display = sprintf('%d h %d min %d sec', hours, minutes, seconds);
                    elseif minutes > 0
                        time_display = sprintf('%d min %d sec', minutes, seconds);
                    else
                        time_display = sprintf('%d sec', seconds);
                    end
    
                    % BAR
                    progress = 1 - (rem_estimated_time / total_estimated_time);
    
                    % WAITBAR
                    waitbar(progress, h, sprintf('Remaining time %s', time_display))
    
                end
            end
            % CLEAR to free up memory
            clear x q Q PI_ G
        end
        % CLEAR to free up memory
        clear z w
    end
    
    close(h);

end