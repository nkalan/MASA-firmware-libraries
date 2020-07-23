/**
 * Header file for communicating with MS5607 Pressure Altimeter
 * Datasheet: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5607-02BA03%7FB2%7Fpdf%7FEnglish%7FENG_DS_MS5607-02BA03_B2.pdf%7FCAT-BLPS0035
 *
 * From the datasheet: "The MS5607-02BA consists of a piezo-resistive sensor
 * and a sensor interface IC. The main function of the MS5607-02BA is to
 * convert the uncompensated analogue output voltage from the piezo-resistive
 * pressure sensor to a 24-bit digital value, as well as providing a 24-bit
 * digital value for the temperature of the sensor."
 * 
 * The altimeter supplies pressure and temperature, which is then converted to
 * altitude according to the 1976 U.S. Standard Atmosphere model.
 * 
 * =================================================================
 * EXAMPLE CODE & IMPLEMENTATION NOTES
 * =================================================================
 * Create an MS5607_Altimeter struct in the main code, and call
 * these functions the following order, with the specified delays.
 *
 * 		MS5607_Altimeter altimeter;
 * 		MS5607_initialization(&altimeter, ...)
 *
 * 		while(1) {
 * 			MS5607_first_conversion(&altimeter)
 * 			delay MS5607_get_adc_conversion_time(&altimeter)
 * 			MS5607_second_conversion(&altimeter)
 * 			delay MS5607_get_adc_conversion_time(&altimeter)
 *
 * 			int32_t pressure, temperature;
 * 			MS5607_calculate_pressure_and_temperature(&altimeter, &pressure, &temperature)
 * 		}
 *
 * The pointer to an MS5607_Altimeter struct must be passed to each function
 * along with any other necessary parameters. For
 * MS5607_calculate_pressure_and_temperature, 2 int32_t pointers
 * are passed to store the final pressure and temperature.
 *
 * The delays should be implemented with STM32 Timers.
 *
 * =================================================================
 * SPI SETTINGS
 * =================================================================
 * The maximum SPI clock frequency supported by the MS5607 is 20 MHz.
 * For extra caution, setting it to 15 MHz or below is acceptable.
 *
 * The MS5607 can operate in SPI mode 0 (clock idle low, rising edge)
 * or SPI mode 3 (clock idle high, falling edge)
 *
 * These can be configured in STM32CubeMX or STM32CubeIDE under SPI settings.
 * =================================================================
 *
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created May 3, 2020
 * Last edited July 21, 2020
 */

#ifndef MS5607_H	// begin header include protection
#define MS5607_H

#include "stm32f4xx_hal.h"

/**
 * MS5607_OversamplingRate
 *
 * The MS5607 can operate at one of 5 over sampling rates (OSR).
 * The code is slightly different for each one, so one must be
 * selected at initialization.
 * 
 * Higher OSR -> higher resolution, lower frequency
 * Lower OSR -> lower resolution, higher frequency
 */
typedef enum {
	OSR_256, OSR_512, OSR_1024, OSR_2048, OSR_4096
} MS5607_OversamplingRate;

/**
 * MS5607_Altimeter struct to store pin, SPI, and oversampling rate settings.
 */
typedef struct {
	MS5607_OversamplingRate OSR; 	// Oversampling rate for pressure/temperature readings, specified by user
	SPI_HandleTypeDef *SPI_BUS;   // SPI struct, specified by user
	GPIO_TypeDef *cs_base;        // Chip select GPIO base, specified by user
	uint16_t cs_pin;              // Chip select GPIO pin, specified by user
	uint16_t constants[8]; 				// Calibration constants to be read in during second_initialization()
	int32_t D1;     							// Uncompensated digital pressure to be read in from the altimeter ADC
	int32_t D2;  									// Uncompensated digital temperature to be read in from the altimeter ADC
} MS5607_Altimeter;

/**
 * Initializes an MS5607_Altimeter struct with pin, SPI, and oversampling
 * rate settings, resets its memory from an unknown state, then reads in the 8
 * calibration constants from the MS5607's PROM over SPI.
 *
 * This function includes a 3 ms delay.
 *
 * This function must be run once after starting up the microcontroller, and
 * before running all other functions.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 * @param OSR_in					   	<MS5607_OversamplingRate>		OSR setting for pressure/temperature readings
 * @param SPI_BUS           	<SPI_HandleTypeDef*>        SPI struct used for communication
 * @param cs_base           	<GPIO_TypeDef*>             GPIO pin array the chip select pin is on
 * @param cs_pin            	<uint16_t>                  GPIO pin connected to altimeter chip select
 */
void MS5607_initialization(MS5607_Altimeter *altimeter,
		MS5607_OversamplingRate OSR_in, SPI_HandleTypeDef *SPI_BUS_in,
		GPIO_TypeDef *cs_base_in, uint16_t cs_pin_in);

/**
 * Commands the MS5607's ADC to convert the digital pressure data.
 *
 * This function does not require any delay before running. However, it
 * requires a delay afterwards, and the ADC will return 0 if you try to
 * read from the ADC before the conversion is finished.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 */
void MS5607_first_conversion(MS5607_Altimeter *altimeter);

/**
 * Reads digital pressure from the altimeter's ADC and commands the MS5607's
 * ADC to convert the digital temperature data.
 *
 * This function must be run after MS5607_first_conversion() followed by
 * a delay specified by MS5607_first_conversion_delay(). The ADC will return
 * 0 if you try to read from the ADC before the conversion is finished.
 * 
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 */
void MS5607_second_conversion(MS5607_Altimeter *altimeter);

/**
 * Returns the maximum ADC conversion time in milliseconds, specified on
 * page 3 of the datasheet. This time depends on the oversampling rate,
 * selected during initialization.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 * @retval The ADC conversion time in ms
 */
uint8_t MS5607_get_adc_conversion_time(MS5607_Altimeter *altimeter);

/**
 * Reads digital temperature from the altimeter's ADC and calculates
 * actual temperature and pressure. Calculations are documented on
 * page 8-9 of the datasheet.
 *
 * This function must be run after MS5607_second_conversion() followed by
 * a delay specified by MS5607_second_conversion_delay(). The ADC will return
 * 0 if you try to read from the ADC before the conversion is finished.
 * 
 * First, temperature and several other intermediate variables are calculated
 * according to a linear calibration curve, using the calibration constants
 * read during initialization. If the temperature is below 20 C, the variables
 * are adjusted to a quadratic calibration curve. Finally, pressure is
 * calculated, and both pressure and temperature are stored in pointers.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 * @param final_pressure			<int32_t*>									int32_t pointer to store final pressure value
 * @param final_temperature		<int32_t*>									int32_t pointer to store final temperature value
 */
void MS5607_calculate_pressure_and_temperature(MS5607_Altimeter *altimeter,
		int32_t *final_pressure, int32_t *final_temperature);

#endif  // end header include protection
