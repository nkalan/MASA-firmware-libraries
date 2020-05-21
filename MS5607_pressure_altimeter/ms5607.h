/** ms5607.h
 *
 * Header file for communicating with MS5607 Pressure Altimeter
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

#ifndef MS5607_H
#define MS5607_H

#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_gpio.h"
#include "Stm32f4xx_hal_tim.h"

//#define MS5607_PACK_16_TO_8(data) {data>>8, (data<<8)>>8}  // MSB SPI line
#define MS5607_CS_ACTIVE    GPIO_PIN_RESET  // chip is active low
#define MS5607_CS_INACTIVE  GPIO_PIN_SET

// Pressure and temperature ranges the device can handle
#define MS5607_MINIMUM_PRESSURE         = 1000      //mbar*100    (10 mbar)
#define MS5607_MAXIMUM_PRESSURE         = 120000    //mbar*100    (1200 mbar)
#define MS5607_MINIMUM_TEMPERATURE      = -4000     //Celsius*100 (-40 C)
#define MS5607_MAXIMUM_TEMPERATURE      = 8500      //Celsius*100 (85 C)
#define MS5607_REFERENCE_TEMPERATURE    = 2000      //Celsius*100 (20 C)


/** MS5607_OversamplingRatio
 * The MS5607 can operate at one of 5 oversampling ratios (OSR).
 * The commands are different for each, so one must be selected at initialization.
 * 
 */
typedef enum {
    OSR_256;
    OSR_512;
    OSR_1024;
    OSR_2048;
    OSR_4096;
} MS5607_OversamplingRatio;


/** MS5607
 * 
 */
typedef struct {
    MS5607_OversamplingRatio OSR;       // Oversampling ratio

    TIM_HandleTypeDef* timer;            // Timer

    SPI_HandlTypeDef* SPI_BUS;          // SPI

    GPIO_TypeDef* cs_base;              // Chip select GPIO base

    uint16_t cs_pin;                    // Chip select GPIO pin

    uint16_t[6] calibration_constants;            // Calibration constants for pressure and temperature
                                        /* {Presure sensitivity,
                                            Presure offset,
                                            Temperature coefficient of pressure sensitivity,
                                            Temperature coefficient of pressure offset,
                                            Reference temperature,
                                            Temperature coefficient of the temperature} */

    uint32_t time_of_last_conversion;   // 

    double last_recorded_pressure;
    
    double last_recorded_temperature;         // 
} MS5607_Altimeter;


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
        SPI_HandlTypeDef* SPI_BUS_in,GPIO_TypeDef* cs_base_in, uint16_t cs_pin_in);


/** read_pres_and_temp
 * reads pressure and temperature from the altimeter, then corrects them using the calibration constants
 * calculations are documented on datasheet p.8-9
 * 
 * @param altimeter     <MS5607_CalibrationConstants*>      altimeter object to store settings and constants
 * @param pres          <double*>                           pointer to double to store the pressure
 * @param temp          <double*>                           poniter to double to store the temperature
 * 
 */
void read_pres_and_temp(MS5607_CalibrationConstants altimeter, double* pres, double* temp);

#endif  /* end header include protection */