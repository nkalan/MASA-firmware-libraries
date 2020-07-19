close all;

C = [uint32(42836), uint32(38938), uint32(26130), uint32(23835), uint32(32083), uint32(27666)];

Dmax = 16777216;
N = 1677722;

D1_datasheet = 6465444; %normal pressure
D1_mine = 6202848;
D2_vec = uint32(linspace(0, 16777210, N));   %temp

zero_vec = linspace(0, 0, N);

dT_vec = zeros(1, N);
TEMP_vec = zeros(1,N);
OFF_vec = zeros(1, N);
SENS_vec = zeros(1, N);
PRES_vec = zeros(1, N);

for i = 1:N
    if (mod(i,10000) == 0)
        disp(i);
    end
    
    [dT, TEMP, OFF, SENS, PRES] = pres_temp(D1_mine, D2_vec(i), C);
    dT_vec(i) = dT;
    TEMP_vec(i) = TEMP;
    OFF_vec(i) = OFF;
    SENS_vec(i) = SENS;
    PRES_vec(i) = PRES;
end

figure;
plot(D2_vec, dT_vec);
title('Binary Temperature vs dT');
xlabel('Binary Temperature');
ylabel('dT');

figure;
plot(D2_vec, TEMP_vec);
title('Binary Temperature vs TEMP');
xlabel('Binary Temperature');
ylabel('TEMP');

figure;
plot(D2_vec, OFF_vec);
title('Binary Temperature vs OFF');
xlabel('Binary Temperature');
ylabel('OFF');

figure;
plot(D2_vec, SENS_vec);
title('Binary Temperature vs SENS');
xlabel('Binary Temperature');
ylabel('SENS');

figure;
plot(D2_vec, PRES_vec);
title('Binary Temperature vs PRES');
xlabel('Binary Temperature');
ylabel('PRES');

function [dT, TEMP, OFF, SENS, PRES] = pres_temp(D1, D2, C)
    dT = int32(D2 - C(5)*(2^8));
    TEMP = int32(int32(2000) + int32(dT*int32(C(6))/(2^23)));
    
    OFF = int64(int32(C(2)*(2^17)) + (int32(C(4))*dT)/(2^6));
    SENS = int64(int32(C(1)*(2^16)) + (int32(C(3))*dT)/(2^7));
    
    
    if (TEMP < 2000)
        T2 = int32(dT^2 / (2^31));
        OFF2 = int64(61*(TEMP - 2000)^2/(2^4));
        SENS2 = int64(2*(TEMP - 2000)^2);
        
        if (TEMP < -1500)
            OFF2 = OFF2 + 15*(TEMP + 1500)^2;
            SENS2 = SENS2 + 8*(TEMP + 1500)^2;
        end
        
        TEMP = TEMP - T2;
        OFF = OFF - OFF2;
        SENS = SENS - SENS2;
    end
    
    
    PRES = int32((D1 * SENS/(2^21) - OFF)/(2^15));
    
end