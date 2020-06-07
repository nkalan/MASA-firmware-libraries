function AtmPropDriver
    % Adapted from AERO201 project 3 code
    
    close all;

    hplot_min = 0;
    hplot_max = 31060;
    
    N = (hplot_max - hplot_min) / 0.20;
    N = 500;
    
    hvec = linspace(hplot_min, hplot_max, N);
    Pvec = AtmProp(hvec, N);
    
    % make sure the pressure axis is strictly increasing
    hvec = flip(hvec);
    Pvec = flip(Pvec);
    Pvec = round(Pvec);     % round to the nearest unit (0.01 mbar / 1 Pa)
    
    
    % Units are mbar/100 for the firmware (last 2 digits are decimals)
    %hvec = hvec * 100;
    
    
    %table = [Pvec, hvec];
    %process_table(table, N);
    
    Pvec = [1 3 6 9 13];
    hvec = [1 2 3 4 5];
    table = [Pvec; hvec];
    
    disp(table');
    processed_table = process_function(table, 10);
    
    %{
    fileID = fopen('pressure_altitude_lookup_table_temp.txt', 'w');
    fprintf(fileID, '#"include <stdint.h>"\r\n');
    fprintf(fileID, 'static const int32_t table[120001 = {\r\n');
    fprintf(fileID,'%13s %12s\r\n', 'Pressure [Pa]', 'Altitude [cm]');
    fprintf(fileID,'%19.10f %13.0f\r\n', lookup_table);
    fclose(fileID);
    %}
    
    
    %{
    fileID = fopen('pressure_altitude_lookup_table.txt', 'w');
    fprintf(fileID,'%13s %12s\r\n', 'Pressure [Pa]', 'Altitude [cm]');
    fprintf(fileID,'%19.0f %13.0f\r\n', lookup_table);
    fclose(fileID);
    %}
    

end

function map = process_function(table, N)
    % Size changes - store it in a new variable
    length = N;
    
    % copy the data table and edit it as needed
    map = table;
    
    % Delete redundant rows and interpolate missing data
    index = 1;
    while (index <= length)
        disp(['index: ', num2str(index)]);
        
        % if the next pressure is the same as the current
        if (index ~= length && map(1, index+1) == map(1, index))
            
            % first find the index of the last occurrance of that pressure
            last_repeat_index = index;
            while (last_repeat_index ~= length && map(1, last_repeat_index+1) == map(1, index))
                last_repeat_index = last_repeat_index + 1;
            end
            disp(['last_repeat_index: ', num2str(last_repeat_index)]);
            
            
            % replace that chunk of redundant rows with their average
            
            % edge case: repeated chunk is the entire list (lolwut?)
            if (index == 1 && last_repeat_index == length)
                map = [map(1, index); mean(map(2, :))];
            
            % edge case: repeated chunk is at the beginning
            elseif (index == 1)
                map = [map(1, index), map(1, last_repeat_index+1:length); ...
                    mean(map(2, 1:last_repeat_index)), map(2, last_repeat_index+1:length)];
                
            % edge case: repeated chunk is at the end
            elseif (last_repeat_index == length)
                map = [map(1, 1:index-1), map(1, index); ...
                    map(2, 1:index-1), mean(map(2, index:length))];
                
            % nominal: repeated chunk is in the middle
            else
                map = [map(1, 1:index-1), map(1, index), map(1, last_repeat_index+1:length); ...
                    map(2, 1:index-1), mean(map(2, index:last_repeat_index)), map(2, last_repeat_index+1:length)];
            end
            
            % update the length varaible
            old_length = length;
            length = length - (last_repeat_index - index);
            assert(old_length > length);
            
        % if the next pressure is more than 1 unit away the current one
        elseif (index ~= length && abs(map(1, index+1) - map(1, index)) > 1)
            
            % linear interpolation of missing data points on the independent axis
            slope = (map(2, index+1) - map(2, index)) / (map(1, index+1) - map(1, index));
            array_insert = linspace(map(1, index)+1, map(1, index+1)-1, map(1, index) + 1 - map(1, index+1));
            disp('insert:');
            disp(array_insert);
            
        end
        
        disp(map');
        index = index+1;
    end

end

function pres_vec = AtmProp(h, N)
% PURPOSE: This function calculates the standard temperature, pressure, and
%   density of the atmosphere at a certain height. Functions with one
%   altitude input or a vector of altitudes.
% INPUTS: Vector of altitudes [m], size of altitude vector
% OUTPUTS: pressure [N/m^2]

    % Atmospheric constants
    constants.g = 9.80665;    % gravitational acceleration [m/s^2]
    constants.R  = 287.053;   % gas constant for air [J/kg.K]
    constants.p0 = 101325;    % sea-level pressure [N/m^2]
    
    constants.hl = [0, 11000, 20000, 32000, 47000, 51000, 71000, 85000];                % altitudes at layer endpoints [m]
    constants.Tl = [288.15, 216.65, 216.65, 228.65, 270.65, 270.65, 214.65, 186.65];    % temperature at start of each layer [K]
    constants.Ll = [-.0065, 0, .0010, .0028, 0, -0.0028, -0.0020, 0];                   % lapse rate for each layer [K/m]
    constants.Pl = [101325, 22635.6245004134, 5476.611571261, 868.42118529855, ...      % pressures at start of each layer [Pa]*
                    110.9716823, 66.98677854757, 3.9598792576, 0.3637966549];           %   *calculted with previous version of code, then hardcoded
                                                                                        %    to reduce runtime
    
    % calculate atmospheric properties at each altitude
    pres_vec = zeros(1, N);
    for i = 1:N
        pres_vec(i) = Pres(h(i), constants);      % calculate pressures [N/m^2]
    end
end

% Local function to calculate the temperature at a given height
function temp = Temp(h, constants)
    layer = getLayerIndex(h, constants);
    
    % Determine the temperature
    temp = constants.Tl(layer) + constants.Ll(layer) * (h - constants.hl(layer));
end

% Local function to calculate the air pressure at a given height
function pres = Pres(h, constants)
    layer = getLayerIndex(h, constants);
    
    if (isGradientLayer(layer))
        pres = constants.Pl(layer) * (Temp(h, constants) / Temp(constants.hl(layer), constants)) ^ ...
            (-1 * constants.g / (constants.R * constants.Ll(layer)));
    else
        pres = constants.Pl(layer) * exp(-1 * constants.g * (h - constants.hl(layer)) / (constants.R * Temp(h, constants)));
    end
end

% Returns a number 1-8 for what layer h is in
function layer = getLayerIndex(h, constants)
    assert(h >= 0);
    layer = -1;
    
    for layer_index = 1:8
        if (constants.hl(layer_index) <= h)
            layer = layer_index;
        end
    end
    
    assert(layer > 0);
end

% Return true if layer is a constant gradient temperature layer, false otherwise
function is_gradient = isGradientLayer(layer)
    assert(1 <= layer && layer <= 8);
    if (layer == 1 || layer == 3 || layer == 4 || layer == 6 || layer == 7)
        is_gradient = true;
    elseif (layer == 2 || layer == 5 || layer == 8)
        is_gradient = false;
    else 
        assert(false);
        disp('Invalid layer input in isGradientLayer()');
    end
end