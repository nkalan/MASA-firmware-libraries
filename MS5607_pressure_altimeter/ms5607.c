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
#include <math.h>
#include "MS5607.h"

/** Command bytes 
 * Each command has a corresponding 1-byte sequence, with
 * each bit representing something
 * 
 * datasheet p.10:
 * Bit #    Bit name
 * 0        PRM
 * 1        COV
 * 2        -
 * 3        Typ
 * 4        Ad2/Os2
 * 5        Ad1/Os1
 * 6        Ad0/Os0
 * 7        Stop
 * 
 * _OSR_####_ is "oversampling ratio", with commands for
 * each of the five settings: 256/512/1024/2048/4096
 * 
 */
#define RESET_CMD                   0x1E    // 0b00011110
#define CONVERT_D1_OSR_256_CMD      0x40    // 0b01000000
#define CONVERT_D1_OSR_512_CMD      0x42    // 0b01000010
#define CONVERT_D1_OSR_1024_CMD     0x44    // 0b01000100
#define CONVERT_D1_OSR_2048_CMD     0x46    // 0b01000110
#define CONVERT_D1_OSR_4096_CMD     0x48    // 0b01001000
#define CONVERT_D2_OSR_256_CMD      0x50    // 0b01010000
#define CONVERT_D2_OSR_512_CMD      0x52    // 0b01010010
#define CONVERT_D2_OSR_1024_CMD     0x54    // 0b01010100
#define CONVERT_D2_OSR_2048_CMD     0x56    // 0b01010110
#define CONVERT_D2_OSR_4096_CMD     0x58    // 0b01011000
#define ADC_READ_CMD                0x00    // 0b00000000
#define PROM_READ_CMD_BASE          0xA0    // 0b1010---0   //8 different commands, the first one is defined


/** ADC Maximum Conversion times
 * The altimeter's onboard ADC must wait a certain amount of time
 * after receiving the CONVERT command before data can be read with
 * the ADC_READ command. If the ADC_READ command is given too early or the
 * CONVERT command is not given, the ADC will return a 0 instead of data.
 * 
 */
#define MAX_ADC_CONVERSION_TIME_OSR_256   9040    //units in microseconds
#define MAX_ADC_CONVERSION_TIME_OSR_512   4540
#define MAX_ADC_CONVERSION_TIME_OSR_1024  2280
#define MAX_ADC_CONVERSION_TIME_OSR_2048  1170
#define MAX_ADC_CONVERSION_TIME_OSR_4096   600


/** Temperature compensation constants
 * Used to determine if the temperature is low enough to warrant compensation calculations
 * 
 */
#define MS5607_LOW_TEMPERATURE          =  2000     //Celsius*100 (20 C)
#define MS5607_VERY_LOW_TEMPERATURE     = -1500     //Celsius*100 (-15 C)


/** initialize
 * This function MUST be run once after starting up the microcontroller.
 * Initializes the chip by resetting its memory and reading calibration constants from its memory
 * 
 * @param altimeter         <MS5607_CalibrationConstants*>      altimeter object to store settings and constants
 * @param OSR_setting_in    <MS5607_OversamplingRatio>          OSR setting for the chip to operate at
 * @param timer_in          <TIM_HandleTypeDef*>                Timer object for tracking ADC conversion times
 * @param SPI_BUS           <SPI_HandleTypeDef*>                SPI object altimeter is on
 * @param cs_base           <GPIO_TypeDef*>                     GPIO pin array chip select pin is on
 * @param cs_pin            <uint16_t>                          GPIO pin connected to altimeter chip select
 * 
 */
void initialize(MS5607_Altimeter* altimeter, MS5607_OversamplingRatio OSR_setting_in, TIM_HandleTypeDef* timer_in,
        SPI_HandlTypeDef* SPI_BUS_in,GPIO_TypeDef* cs_base_in, uint16_t cs_pin_in) {

    altimeter->OSR_setting = OSR_in;
    altimeter->timer = htim_in;
    altimeter->SPI_BUS = SPI_BUS_in;
    altimeter->cs_base = cs_base_in;
    altimeter->cs_pin = cs_pin_in;
    
    MS5607_write_to_spi(altimeter, RESET_CMD);  //reset the device ROM
    read_calibration_constants(altimeter);      //read the 6 calibration constants into the altimeter object
}

/** read_calibration_constants
 * Reads the 6 factory calibration constants used to calculate pressure and temperature
 * and stores them in the altimeter object
 * 
 * @param altimeter     <MS5607_CalibrationConstants*>      altimeter object to store settings and constants
 *  
 */ 
void read_calibration_constants(MS5607_CalibrationConstants* altimeter) {
    
    //organize the 6 transmit commands into an array for looping
    uint8_t[6] tx_PROM_commands = {PROM_READ_CMD_BASE + 1,
                                PROM_READ_CMD_BASE + 2,
                                PROM_READ_CMD_BASE + 3,
                                PROM_READ_CMD_BASE + 4,
                                PROM_READ_CMD_BASE + 5,
                                PROM_READ_CMD_BASE + 6};

    //prepare the buffer for receiving the 6 constants
    uint16_t[6] rx_buffer;

    //disable inturrupt requests and select the chip
    __disable_irq();
    MS5607_chip_select(altimeter);

    //send and receive 6 times, once for each constant
    for (int i = 0; i < 6; i++) {
        HAL_SPI_Transmit(SPI_BUS, tx_PROM_commands[i], 1, TIMEOUT_VAL);     //send a 1 byte command
        HAL_SPI_Receive(SPI_BUS, rx_buffer[i], 2, TIMEOUT_VAL);             //receive a 2 byte command
    }

    //re-enable inturrupt requests and release the chip
    MS5607_chip_release(altimeter);
    __enable_irq();
    
    //store the received values in the altimeter object
    for (int i = 0; i < 6, i++) {
        altimeter->calibration_constants[i] = rx_buffer[i];
    }
}


/** read_pres_and_temp
 * reads pressure and temperature from the altimeter, then corrects them using the calibration constants
 * calculations are documented on datasheet p.8-9
 * 
 * @param altimeter     <MS5607_CalibrationConstants*>      altimeter object to store settings and constants
 * @param pres          <double*>                           pointer to double to store the pressure
 * @param temp          <double*>                           poniter to double to store the temperature
 * 
 */
void read_pres_and_temp(MS5607_CalibrationConstants altimeter, double* pres, double* temp) {

    // Select the proper values for the altimeter's oversampling ratio
    uint8_t pressure_conversion_command, temperature_conversion_command;
    uint16_t conversion_delay_microseconds;
    
    switch (altimeter->OSR_setting) {
        case OSR_256:
            pressure_conversion_command = CONVERT_D1_OSR_256_CMD;
            temperature_conversion_command = CONVERT_D2_OSR_256_CMD;
            conversion_delay_microseconds = MAX_ADC_CONVERSION_TIME_OSR_256;
            break;

        case OSR_512:
            pressure_conversion_command = CONVERT_D1_OSR_512_CMD;
            temperature_conversion_command = CONVERT_D2_OSR_512_CMD;
            conversion_delay_microseconds = MAX_ADC_CONVERSION_TIME_OSR_512; 
            break;

        case OSR_1024:
            pressure_conversion_command = CONVERT_D1_OSR_1024_CMD;
            temperature_conversion_command = CONVERT_D2_OSR_1024_CMD;
            conversion_delay_microseconds = MAX_ADC_CONVERSION_TIME_OSR_1024; 
            break;

        case OSR_2048:
            pressure_conversion_command = CONVERT_D1_OSR_2048_CMD;
            temperature_conversion_command = CONVERT_D2_OSR_2048_CMD;
            conversion_delay_microseconds = MAX_ADC_CONVERSION_TIME_OSR_2048; 
            break;
            
        case OSR_4096:
            pressure_conversion_command = CONVERT_D1_OSR_4096_CMD;
            temperature_conversion_command = CONVERT_D2_OSR_4096_CMD;
            conversion_delay_microseconds = MAX_ADC_CONVERSION_TIME_OSR_4096; 
            break;
            
        default:
            assert(false);
            break;
    }

    // READING UNCOMPENSATED PRESSURE (D1) AND TEMPERATURE (D2)

    uint32_t D1;        //read digital pressure (24 bits)
    uint32_t D2;        //read digital temperature (24 bits)
    uint32_t current_time = get_time_microseconds(altimeter->timer);

    //TEMPORARY - NEEDS PROPER IMPLEMENTATION
    //uint32_t time_since_last_conversion_microseconds = current_time - altimeter->time_of_last_conversion;

    //temporary, for  debugging
    assert(time_since_last_conversion >= 0);

    if (time_since_last_conversion_microseconds >= conversion_delay_microseconds) {
        
    }

    else {

    }

    // TEMPERATURE CALCULATION

    // Difference between actual and reference temperature
    // dT = D2 - C5*(2^8)
    int32_t dT = D2 - (altimeter->C5 * pow(2, 8));

    // Actual temperature (last 2 digits in base 10 are decimals; units are Celsius*100)
    // TEMP = 2000 + dT * (C6 / 2^23)
    int32_t TEMP = MS5607_REFERENCE_TEMPERATURE + dT * ((double)altimeter->C6 / pow(2, 23));

    // Offset at actual temperature
    // OFF = (C2 * 2^17) + (C4 * dT) / 2^6
    int64_t OFF = (altimeter->C2 * pow(2, 17)) + dT * ((double)altimeter->C4 / pow(2, 6));

    // Sensitivity at actual temperature
    // SENS = (C1 * 2^16) + (C3 * dT) / 2^7
    int64_t SENS = (altimeter->C1 * pow(2, 16)) + dt * ((double)altimeter->C3 / pow(2, 7));
    

    // TEMPERATURE COMPENSATION CALCULATIONS

    int T2 = 0;
    int OFF2 = 0;
    int SENS2 = 0;

    // if temperature is low (TEMP < 20 C), compensate
    if (TEMP < MS5607_LOW_TEMPERATURE) {
        T2 = (dT*dT) >> 31;
        OFF2 = 61 * pow(TEMP - MS5607_REFERENCE_TEMPERATURE, 2) / (double)pow(2, 4);
        SENS2 = 2 * pow(TEMP - MS5607_REFERENCE_TEMPERATURE, 2);

        // if temperature is very low (TEMP < -15 C), compensate again
        if (TEMP < MS5607_VERY_LOW_TEMPERATURE) {
            OFF2 += 15 * pow(TEMP - MS5607_VERY_LOW_TEMPERATURE, 2);
            SENS2 += 8 * pow(TEMP - MS5607_VERY_LOW_TEMPERATURE, 2);
        }
    }
    
    TEMP -= T2;
    OFF -= OFF2;
    SENS -= SENS2;

    // PRESSURE CALCULATION

    // Temperature compensated pressure (units are mbar*100)
    // P = (D1*SENS/(2^21) - OFF) / (2^15)
    int64_t P = (D1 * (SENS * pow(2, 21)) - OFF) * pow(2, 15);

}


/** MS5607_chip_select
 * helper function to set the CS pin active
 *
 * @param altimeter     <MS5607_CalibrationConstants*>      altimeter object to store settings and constants
 *
 */
void MS5607_chip_select(altimeter) {
    HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_ACTIVE);
}


/** MS5607_chip_release
 * helper function to set the CS pin inactive
 *
 * @param altimeter     <MS5607_CalibrationConstants*>      altimeter object to store settings and constants
 *
 */
void MS5607_chip_release(altimeter) {
    HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_INACTIVE);
}


/** write_to_spi
 * helper function to transmit a 1 byte message to MS5607 altimeter over SPI
 *
 * @param altimeter     <MS5607_CalibrationConstants*>      altimeter object to store settings and constants
 * @param tx            <uint16_t>                          value to write to the MS5607
 *
 */
void MS5607_write_to_spi(MS5607_Altimeter altimeter, uint8_t tx)
{
    __disable_irq();    // disable interrupts during transmission!!!!
    MS5607_chip_select(altimeter);
    if (HAL_SPI_Transmit(SPI_BUS, tx, 1, TIMEOUT_VAL) ==  HAL_TIMEOUT) {}   //send 1 byte
    MS5607_chip_release(altimeter);
    __enable_irq();     // end transmission... created by mr. warburton
}


/** get_time_microseconds
 * helper function to track the counter on the timer
 * 
 * @param timer     <TIM_HandleTypeDef*>        valid timer object
 * @retval a 32 bit integer representing the time in microseconds
 * 
 */
int32_t get_time_microseconds(TIM_HandleTypeDef* timer) {
    return __HAL_TIM_GET_COUNTER(&timer);
}