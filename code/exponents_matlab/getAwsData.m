function data = getAwsData(M, N, SNR, R)
    % Specify the Python script file name to be executed.
    pythonScript = 'get_data.py';
    
    % Define the table name that will be used by the Python script.
    tableName = 'exponents';
    
    % Build the command string to execute the Python script.
    % The sprintf function formats the command string with the provided numeric parameters.
    % --table: specifies the table name
    % --constellationM: passes the value of M
    % --nodesN: passes the value of N
    % --SNR: passes the signal-to-noise ratio
    % --transmissionRate: passes the transmission rate R

    %% We need to check the OS to see which cmd command should I run
    cmd = sprintf('python "%s" --table %s --constellationM %f --nodesN %f --SNR %f --transmissionRate %f', ...
        pythonScript, tableName, M, N, SNR, R);
    
    % Execute the command using the system function.
    % status: return code (0 means success)
    % cmdout: output from the Python script
    [status, cmdout] = system(cmd);
    
    if status == 0
        try
            % Decode the JSON string output from the Python script into a MATLAB structure.
            data = jsondecode(cmdout);
        catch ME
            % If JSON decoding fails, create an error structure with the error message.
            data = struct('status', 'error', 'message', ['JSON decoding failed: ' ME.message]);
        end
    else
        % If the Python script execution fails, create an error structure with the failure status.
        data = struct('status', 'error', 'message', sprintf('Python script call failed with status %d', status));
    end
end
