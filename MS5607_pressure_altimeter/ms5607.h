/** ms5607.h
 *
 * Header file for communicating with MS5607 Pressure Altimeter
 * datasheet: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5607-02BA03%7FB2%7Fpdf%7FEnglish%7FENG_DS_MS5607-02BA03_B2.pdf%7FCAT-BLPS0035
 *
 * From the datasheet: "The MS5607-02BA consists of a piezo-resistive sensor and a sensor interface IC.
 * The main function of the MS5607-02BA is to convert the uncompensated analogue output voltage from the piezo-resistive
 * pressure sensor to a 24-bit digital value, as well as providing a 24-bit digital value for the temperature of the sensor."
 * 
 * 
 * 
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created May 3, 2020
 * Last edited June 3, 2020
 */

#ifndef MS5607_H
#define MS5607_H

#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_gpio.h"

#define MS5607_CS_ACTIVE    GPIO_PIN_RESET  // chip is active low
#define MS5607_CS_INACTIVE  GPIO_PIN_SET

/** ADC Maximum Conversion times
 * The altimeter's onboard ADC must wait a certain amount of time
 * after receiving the CONVERT command before data can be read with
 * the ADC_READ command. If the ADC_READ command is given too early or the
 * CONVERT command is not given, the ADC will return a 0 instead of data.
 * 
 */
#define ADC_CONVERSION_TIME_OSR_256_MICROSECONDS   9040    //units in microseconds
#define ADC_CONVERSION_TIME_OSR_512_MICROSECONDS   4540
#define ADC_CONVERSION_TIME_OSR_1024_MICROSECONDS  2280
#define ADC_CONVERSION_TIME_OSR_2048_MICROSECONDS  1170
#define ADC_CONVERSION_TIME_OSR_4096_MICROSECONDS   600

/** Altimeter memory reset delay
 * Delay required after sending the memory reset command
 * 
 */
#define RESET_LOAD_TIME_MICROSECONDS      2880

/** MS5607_OversamplingRate
 * The MS5607 can operate at one of 5 oversampling rates (OSR).
 * The commands are different for each, so one must be selected at initialization.
 * 
 */
typedef enum {
    OSR_256;
    OSR_512;
    OSR_1024;
    OSR_2048;
    OSR_4096;
} MS5607_OversamplineRate;


/** MS5607
 * 
 */
typedef struct {
    MS5607_OversamplingRate OSR;       // Oversampling rate, specified by user

    SPI_HandlTypeDef* SPI_BUS;          // SPI struct, specified by user

    GPIO_TypeDef* cs_base;              // Chip select GPIO base, specified by user

    uint16_t cs_pin;                    // Chip select GPIO pin, specified by user

    uint16_t[6] calibration_constants;  // Calibration constants to be read in during second_initialization()
                                        /* {Presure sensitivity,
                                            Presure offset,
                                            Temperature coefficient of pressure sensitivity,
                                            Temperature coefficient of pressure offset,
                                            Reference temperature,
                                            Temperature coefficient of the temperature} */

    int32_t D1;                         // Uncompensated pressure to be read in from the altimeter ADC

    int32_t D2;                         // Uncompensated temperature to be read in from the altimeter ADC
} MS5607_Altimeter;


/**
 * This function must be run once after starting up the microcontroller.
 * It stores the settings and configuration into the MS5607_Altimeter struct
 *  and initializes the device by resetting its memory from an unknown state.
 * YOU MUST WAIT 2.88 MILLISECONDS BEFORE RUNNING SECOND_INITIALIZATION()
 * 
 * @param altimeter         <MS5607_Altimeter*>             altimeter object to store settings and constants
 * @param OSR_setting_in    <MS5607_OversamplingRate>       OSR setting for the chip to operate at
 * @param SPI_BUS           <SPI_HandleTypeDef*>            SPI object altimeter is on
 * @param cs_base           <GPIO_TypeDef*>                 GPIO pin array chip select pin is on
 * @param cs_pin            <uint16_t>                      GPIO pin connected to altimeter chip select
 * 
 */
void first_initialization(MS5607_Altimeter* altimeter, MS5607_OversamplingRate OSR_setting_in,
        SPI_HandlTypeDef* SPI_BUS_in,GPIO_TypeDef* cs_base_in, uint16_t cs_pin_in);


/**
 * THIS FUNCTION MUST BE RUN AT LEAST 2.88 MILLISECONDS AFTER RUNNING FIRST_INITIALIZATION()
 * It reads factory-calibrated constants from the device and stores them in the altimeter struct.
 * 
 * @param altimeter         <MS5607_Altimeter*>             altimeter object to store settings and constants
 * 
 */
void second_initialization(MS5607_Altimeter* altimeter);


/**
 * Commands the device's ADC to convert the digital pressure data.
 * This function does not require any delay before using.
 * REQUIRES A DELAY AFTER RUNNING - exact time depends on the user-specified oversampling rate (OSR)
 * 
 * @param altimeter         <MS5607_Altimeter*>             altimeter object to store settings and constants
 * 
 */
void first_conversion(MS5607_Altimeter* altimeter);


/**
 * REQUIRES A DELAY BEFORE RUNNING - exact time depends on the user-specified oversampling rate (OSR)
 * Reads the digital pressure from the device's ADC and sends the command to convert digital pressure.
 * REQUIRES A DELAY AFTER RUNNING - exact time depends on the user-specified oversampling rate (OSR)
 * 
 * @param altimeter         <MS5607_Altimeter*>             altimeter object to store settings and constants
 * 
 */
void second_conversion(MS5607_Altimeter* altimeter);


/**
 * REQUIRES A DELAY BEFORE RUNNING - exact time depends on the user-specified oversampling rate (OSR)
 * Reads the digital temperature from the device's ADC and calculates actual pressure and temperature,
 * then returns the altitude above mean sea level (AMSL) in meters
 * 
 * @param altimeter         <MS5607_Altimeter*>             altimeter object to store settings and constants
 * @retval                  <int32_t>                       altitude above mean sea level in meters
 * 
 */
int32_t calculate_altitude(MS5607_Altimeter* altimeter);

#endif  /* end header include protection */