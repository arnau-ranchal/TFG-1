function [rho, H] = hermite_interp(punt1, punt2, dp1, dp2)
        
%     f2 és E0'(1) - R

%     % Coeficients a, b, c de la derivada: a x^2 + b x + c
%     a = 3*(dp2 + dp1 - 2*f2);
%     b = 2*(3*f2 - 2*dp1 - dp2);
%     c = dp1;
%     
%     % Arrels de la derivada (forma manual de la fórmula)
%     %rho1 = (-b + sqrt(b^2 - 4*a*c)) / (2*a)
%     rho = (-b - sqrt(b^2 - 4*a*c)) / (2*a);  % No estem 100% de que sigui aquesta pero quien no arriesga no gana


    %Extraer valores de los puntos
    x1 = punt1(1); f1 = punt1(2);
    x2 = punt2(1); f2 = punt2(2);



    %Construcción de la tabla de diferencias divididas de Hermite
    %z = [x1, x1, x2, x2]; % Duplicamos cada x
    Q = zeros(4,4); % Matriz de diferencias divididas
    Q(:,1) = [f1; f1; f2; f2]; % Primera columna con valores

    % Primera diferencia dividida (con derivadas)
    Q(2,2) = dp1;  % Derivada en x1
    Q(3,2) = (f2 - f1) / (x2 - x1); % Diferencia entre f2 y f1
    Q(4,2) = dp2;  % Derivada en x2

    % Segunda y tercera diferencias divididas
    Q(3,3) = (Q(3,2) - Q(2,2)) / (x2 - x1);
    Q(4,3) = (Q(4,2) - Q(3,2)) / (x2 - x1);
    Q(4,4) = (Q(4,3) - Q(3,3)) / (x2 - x1);

    % Construcción del polinomio de Hermite
    syms x;
    H = Q(1,1) + Q(2,2)*(x - x1) + Q(3,3)*(x - x1)^2 + Q(4,4)*(x - x1)^2*(x - x2);    % Polinomi tret amb Hermite

    %Mostrar el polinomio resultante
    H = expand(H);
    disp('Polinomio de Hermite:');
    disp(H);
    
    % Display
    fplot(H, [x1, x2]); 
    hold on;
    plot([x1, x2], [f1, f2], 'ro', 'MarkerFaceColor', 'r'); % Puntos de interpolación
    xlabel('x');
    ylabel('f(x)');
    title('Interpolación de Hermite');
    grid on;
    hold off;

    %Trobar màxim
    syms x;

    dH = diff(H, x); % Derivada del polinomio de Hermite  % Derivada del polinomi H
    x_criticos = double(solve(dH == 0, x)); % Encontrar raíces de H'(x)
    %Com que sempre sera un polinomi de grau tres, ens trobarem que la seva derivada serà de grau 2, per tant només ens trobarem amb dos punts crítics: un màxim i un mínm.
    
    % Filtrar los valores dentro del intervalo (0,1)
    x_criticos = x_criticos(x_criticos > 0 & x_criticos < 1);
    
    % Evaluar el polinomio en los puntos críticos
    y_criticos = double(subs(H, x, x_criticos));
    
    % Encontrar el máximo dentro del intervalo (0,1)
    if isempty(x_criticos)
        disp('No hay máximos locales en el intervalo (0,1).');
    else
        [y_max, idx] = max(y_criticos);
        x_max = x_criticos(idx);
        disp(['Máximo en rho = ', num2str(x_max), ', f(x) = ', num2str(y_max)]);
        
        % Graficar el punto máximo
        hold on;
        plot(x_max, y_max, 'go', 'MarkerFaceColor', 'g', 'MarkerSize', 8);
        hold off;
    end

    rho = x_max;

end