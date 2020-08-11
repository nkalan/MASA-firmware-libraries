/**
 * Header file for MS5607 Pressure Altimeter firmware library
 * Datasheet: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=srchrtrv&DocNm=MS5607-02BA03&DocType=Data+Sheet&DocLang=English
 *
 * From the datasheet: "The MS5607-02BA consists of a piezo-resistive sensor
 * and a sensor interface IC. The main function of the MS5607-02BA is to
 * convert the uncompensated analogue output voltage from the piezo-resistive
 * pressure sensor to a 24-bit digital value, as well as providing a 24-bit
 * digital value for the temperature of the sensor."
 * 
 * Operating pressure range: 10 mbar to 1200 mbar
 * Operating temperature range: -40 dg C to 85 dg C
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
 *			int MS5607_calculate_altitude(&altimeter);
 * 		}
 *
 * For debugging, the function MS5607_calculate_pressure_and_temperature()
 * can be used to directly access pressure and temperature. The following
 * lines should be substituted or added in the same general location in the
 * code where MS5607_calculate_altitude() is called.
 *
 * 		int32_t pressure, temperature;
 * 		MS5607_calculate_pressure_and_temperature(&altimeter, &pressure, &temperature)
 *
 * A pointer to the MS5607_Altimeter struct must be passed to each function
 * along with any other necessary parameters. The delays should be implemented
 * with STM32 Timers.
 *
 * =================================================================
 * SPI SETTINGS
 * =================================================================
 * The maximum SPI clock frequency supported by the MS5607 is 20 MHz.
 * For extra caution, setting it to 15 MHz or below is acceptable.
 *
 * The MS5607 can operate in SPI mode 0 (clock idle low, rising edge)
 * or SPI mode 3 (clock idle high, falling edge).
 *
 * These can be configured in STM32CubeMX or STM32CubeIDE under SPI settings.
 *
 *  Frame Format: Motorola
 *  Data Size: 8 bits
 *  First Bit: MSB First
 *  Clock Polarity: Low
 *  Clock Phase: 1 Edge
 *
 * =================================================================
 *
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created May 3, 2020
 * Last edited August 11, 2020
 */

#ifndef MS5607_H	// begin header include protection
#define MS5607_H

#include "stm32f4xx_hal.h"

#ifdef HAL_SPI_MODULE_ENABLED	// begin SPI include protection

/**
 * MS5607_OversamplingRate
 *
 * The MS5607 can operate at one of 5 over sampling rates (OSR).
 * The code is slightly different for each one, so one must be
 * selected at initialization.
 * 
 * Higher OSR -> higher resolution, lower frequency
 * Lower OSR -> lower resolution, higher frequency
 *
 * datasheet pg. 3-4
 */
typedef enum {
	OSR_256, OSR_512, OSR_1024, OSR_2048, OSR_4096
} MS5607_OversamplingRate;

/* MS5607_Altimeter struct to store pin, SPI, and oversampling rate settings. */
typedef struct {
	MS5607_OversamplingRate OSR;  // Oversampling rate for pressure/temperature readings, specified by user
	SPI_HandleTypeDef *SPI_bus;   // SPI struct, specified by user
	GPIO_TypeDef *cs_base;        // Chip select GPIO base, specified by user
	uint16_t cs_pin;              // Chip select GPIO pin, specified by user
	uint16_t constants[8]; 				// Calibration constants to be read in during second_initialization()
	int32_t D1; 									// Uncompensated digital pressure to be read in from the altimeter ADC
	int32_t D2; 									// Uncompensated digital temperature to be read in from the altimeter ADC
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
 * @param SPI_bus           	<SPI_HandleTypeDef*>        SPI struct used for communication
 * @param cs_base           	<GPIO_TypeDef*>             GPIO pin array the chip select pin is on
 * @param cs_pin            	<uint16_t>                  GPIO pin connected to altimeter chip select
 */
void MS5607_altimeter_init(MS5607_Altimeter *altimeter,
    MS5607_OversamplingRate OSR_in, SPI_HandleTypeDef *SPI_bus_in,
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
 * a delay specified by MS5607_get_adc_conversion_time. The ADC will return
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
 * Debugging function to read pressure and temperature directly.
 * Reads digital temperature from the altimeter's ADC and calculates
 * actual temperature and pressure. Calculations are documented on
 * page 8-9 of the datasheet.
 *
 * First, temperature and several other intermediate variables are calculated
 * according to a linear calibration curve, using the calibration constants
 * read during initialization. If the temperature is below 20 C, the variables
 * are adjusted to a quadratic calibration curve. Finally, pressure is
 * calculated, and both pressure and temperature are stored in pointers.
 *
 * Pressure is in units of 0.01*mbar, so 110002 = 1100.02 mbar,
 * Temperature is in units of 0.01*C, so 2000 = 20 C, -1550 = -15.5 C
 *
 * Note: This function is for debugging only. It doesn't need to be run by the
 * user to find altitude; MS5607_calculate_altitude() calls this function.
 * It's a public function in case it needs to be used for debugging. If it's
 * used, it should be substituted or added wherever MS5607_calculate_altitude()
 * is called.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 * @param final_pressure			<int32_t*>									int32_t pointer to store final pressure value
 * @param final_temperature		<int32_t*>									int32_t pointer to store final temperature value
 */
void MS5607_calculate_pressure_and_temperature(MS5607_Altimeter *altimeter,
    int32_t *final_pressure, int32_t *final_temperature);

/**
 * Calculates altitude above mean sea level (AMSL) in meters, according to the
 * 1976 US standard atmosphere model. It uses a lookup table array generated
 * by a MATLAB script to find the correct altitude.
 *
 * This function must be run after MS5607_second_conversion() followed by
 * a delay specified by MS5607_get_adc_conversion_time. The output will
 * be incorrect if the delay is not long enough.
 *
 * TODO: Figure out ranges for pressure
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 */
uint32_t MS5607_calculate_altitude(MS5607_Altimeter *altimeter);

#endif 	// end SPI include protection
#endif  // end header include protection
