function [] = graphics(y, x, rang_y, rang_x, color, lineType, plotType, pTitle, x_axis, y_axis)
    % GRAPHICS FUNCTION
    % Plots data with customizable parameters for visualization.
    %
    % Inputs:
    %   y: Data array for y-axis (or Z matrix for contour plots)
    %   x: Data array for x-axis
    %   rang_y: [min, max] for y-axis limits (optional)
    %   rang_x: [min, max] for x-axis limits (optional)
    %   color: Line/marker color (e.g., 'r', 'blue', [RGB triplet])
    %   lineType: Line style (e.g., '-', '--', 'o-')
    %   plotType: Type of plot ('linear', 'logy', 'logx', 'loglog', 'contour')
    %
    % Outputs:
    %   A plot according to specified parameters

    % Validate inputs
    if nargin < 7
        error('All 7 input arguments are required.');
    end
    
    % Create figure window
    figure;
    hold on;
    
    % Combine color and line type into a plot specifier
    lineSpec = strcat(color, lineType);
    
    % Handle different plot types
    switch lower(plotType)
        case 'linear'
            plot(x, y, lineSpec);
            
        case 'logy'
            semilogy(x, y, lineSpec);
            
        case 'logx'
            semilogx(x, y, lineSpec);
            
        case 'loglog'
            loglog(x, y, lineSpec);
            
        case 'contour'
            % Validate contour inputs
            if ~ismatrix(y) || numel(x) ~= size(y, 2)
                error('For contour plots, y must be a matrix with columns matching x elements.');
            end
            [X_grid, Y_grid] = meshgrid(x, 1:size(y, 1)); % Adjust grid as needed
            contour(X_grid, Y_grid, y);
            
        otherwise
            error('Invalid plot type. Use: linear, logy, logx, loglog, contour.');
    end
    
    % Apply axis ranges if provided
    if ~isempty(rang_y)
        ylim(rang_y);
    end
    if ~isempty(rang_x)
        xlim(rang_x);
    end
    
    % Add labels and grid
    title(pTitle);
    xlabel(x_axis);
    ylabel(y_axis);
    grid on;
    hold off;

end