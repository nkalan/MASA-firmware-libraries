/** ms5607.c
 *
 * Firmware implementation for communicating with MS5607 Pressure Altimeter
 * datasheet: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5607-02BA03%7FB2%7Fpdf%7FEnglish%7FENG_DS_MS5607-02BA03_B2.pdf%7FCAT-BLPS0035
 *
 * From the datasheet: "The MS5607-02BA consists of a piezo-resistive sensor and a sensor interface IC.
 * The main function of the MS5607-02BA is to convert the uncompensated analogue output voltage from the piezo-resistive
 * pressure sensor to a 24-bit digital value, as well as providing a 24-bit digital value for the temperature of the sensor."
 * 
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created May 3, 2020
 */

#include <stdint.h>
#include "MS5607.h"


uint16_t SENS_T1;       // C1 - Pressure sensitivity
uint16_t OFF_T1;        // C2 - Pressure offset
uint16_t TCS;           // C3 - Temperature coefficient of pressure sensitivity
uint16_t TCO;           // C4 - Temperature coefficient of pressure offset
uint16_t T_REF;         // C5 - Reference temperature
uint16_t TEMPSENS;      // C6 - Temperature coefficient of the temperature

uint32_t D1;            // Digital pressure value
uint32_t D2;            // Digital pressure value

int32_t dT;             // Difference between actual and reference temperature
                        //  dT = D2 - T_REF = D2 - C5*(2^8)
                            
int32_t TEMP;           // Actual temperature (C*100)
                        //  TEMP = 20 C + dT*TEMPSENS = 2000+ dT * (C6 / 2^23)

int64_t OFF;            // Offset at actual temperature
                        //  OFF = OFF_T1 + TCO*dT = C2*(2^17)
                            
int64_t SENS;           // Sensitivity at actual temperature
                        //  SENS = SENS_T1 + TCS*dT = C1*(2^16)+ (C3*dT)/(2^7)

int64_t P;              // Temperature compensated pressure (mbar*100)
                        //  P = D1*SENS - OFF = (D1*SENS/(2^21) - OFF) / (2^15)


void read_calibration_coefficients() {

}

void read_digital_pressure() {

}

void read_digital_temperature() {

}