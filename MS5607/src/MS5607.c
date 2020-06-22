/** ms5607.c
 *
 * Header file for communicating with MS5607 Pressure Altimeter
 * datasheet: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5607-02BA03%7FB2%7Fpdf%7FEnglish%7FENG_DS_MS5607-02BA03_B2.pdf%7FCAT-BLPS0035
 *
 * From the datasheet: "The MS5607-02BA consists of a piezo-resistive sensor and a sensor interface IC.
 * The main function of the MS5607-02BA is to convert the uncompensated analogue output voltage from the piezo-resistive
 * pressure sensor to a 24-bit digital value, as well as providing a 24-bit digital value for the temperature of the sensor."
 * 
 * The altimeter supplies pressure and temperature, which the user must convert to altitude.
 * 
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created May 3, 2020
 * Last edited June 22, 2020
 *
 * Sorry it's a bit of a mess now (⋟﹏⋞)
 * I'll fix the comments after it's debugged
 *
 */

#include <stdint.h>
#include <math.h>
#include <MS5607.h>
//#include "MS5607_altitude_conversion.c"

/** SET_VALUE_BETWEEN_RANGE
 * Helper function to manually ensure a value is in the given range
 * 
 * @param value             the value to be checked
 * @param lower_bound       the minimum possible value
 * @param upper_bound       the maximum possible value
 * @requires: lower_bound <= upper_bound
 */
#define SET_VALUE_BETWEEN_RANGE(value, lower_bound, upper_bound) fmax((lower_bound), fmin((value), (upper_bound)))

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
 * _OSR_####_ is "oversampling rate", with commands for
 * each of the five settings: 256/512/1024/2048/4096
 * 
 */
#define MS5607_RESET_CMD                   0x1E    // 0b00011110
#define MS5607_CONVERT_D1_OSR_256_CMD      0x40    // 0b01000000
#define MS5607_CONVERT_D1_OSR_512_CMD      0x42    // 0b01000010
#define MS5607_CONVERT_D1_OSR_1024_CMD     0x44    // 0b01000100
#define MS5607_CONVERT_D1_OSR_2048_CMD     0x46    // 0b01000110
#define MS5607_CONVERT_D1_OSR_4096_CMD     0x48    // 0b01001000
#define MS5607_CONVERT_D2_OSR_256_CMD      0x50    // 0b01010000
#define MS5607_CONVERT_D2_OSR_512_CMD      0x52    // 0b01010010
#define MS5607_CONVERT_D2_OSR_1024_CMD     0x54    // 0b01010100
#define MS5607_CONVERT_D2_OSR_2048_CMD     0x56    // 0b01010110
#define MS5607_CONVERT_D2_OSR_4096_CMD     0x58    // 0b01011000
#define MS5607_ADC_READ_CMD                0x00    // 0b00000000

#define MS5607_PROM_READ_CMD_BASE          0xA0    // 0b1010---0
/*  Note on PROM_READ_CMD_BASE
 8 different commands, the first one is defined.
 Add 2*n to the base to get the remaining 7 commands, 1 <= n <= 7

 Command 0 is factory data and the setup
 Commands 1-6 are calibration coefficients
 Command 7 is the serial code and CRC
 */

#define MS5607_TIMEOUT	1

#define MS5607_CS_ACTIVE    GPIO_PIN_RESET  // chip is active low
#define MS5607_CS_INACTIVE  GPIO_PIN_SET

/**
 * Minimum and maxinum values for intermediate calculations
 * Used to manually enforce ranges on certain calculated values, specified by the datasheet
 */
#define MS5607_MIN_DT      -16776960
#define MS5607_MAX_DT       16777216
#define MS5607_MIN_OFF     -17179344900
#define MS5607_MAX_OFF      25769410560
#define MS5607_MIN_SENS    -8589672450
#define MS5607_MAX_SENS     12884705280

/** Reference temperature
 * Used in temperature and pressure calculations
 * 
 */
#define MS5607_REFERENCE_TEMPERATURE    2000      	// Celsius/100 (20 C)

/** Temperature compensation constants
 * Used to determine if the temperature is low enough to warrant compensation calculations
 * 
 */
#define MS5607_LOW_TEMPERATURE          2000        // Celsius/100 (20 C)
#define MS5607_VERY_LOW_TEMPERATURE    -1500        // Celsius/100 (-15 C)

/** Pressure and temperature ranges the device can handle
 * Not really used for anything ¯\_(ツ)_/¯
 * 
 */
#define MS5607_MINIMUM_PRESSURE         1000        // mbar/100    (10 mbar)
#define MS5607_MAXIMUM_PRESSURE         120000      // mbar/100    (1200 mbar)
#define MS5607_MINIMUM_TEMPERATURE     -4000        // Celsius/100 (-40 C)
#define MS5607_MAXIMUM_TEMPERATURE      8500        // Celsius/100 (85 C)

#define R_GAS_CONSTANT_AIR              287.05      // J/(kg*K)

/** Lookup table
 * Function mapping pressure to alitude
 * 
 */
//static const int32_t[]
void MS5607_write_to_spi(MS5607_Altimeter *altimeter, uint8_t tx);
uint32_t MS5607_read_from_ADC(MS5607_Altimeter *altimeter);
void MS5607_chip_select(MS5607_Altimeter *altimeter);
void MS5607_chip_release(MS5607_Altimeter *altimeter);

void MS5607_first_initialization(MS5607_Altimeter *altimeter,
		MS5607_OversamplingRate OSR_in, SPI_HandleTypeDef *SPI_BUS_in,
		GPIO_TypeDef *cs_base_in, uint16_t cs_pin_in) {

	altimeter->OSR = OSR_in;
	altimeter->SPI_BUS = SPI_BUS_in;
	altimeter->cs_base = cs_base_in;
	altimeter->cs_pin = cs_pin_in;

	MS5607_write_to_spi(altimeter, MS5607_RESET_CMD);  //reset the device ROM
}

void MS5607_second_initialization(MS5607_Altimeter *altimeter) {

	for (int i = 0; i < 8; i++) {
		uint8_t tx[3] = { MS5607_PROM_READ_CMD_BASE + i * 2, 0, 0 };
		uint8_t rx[3] = { 0, 0, 0 };

		__disable_irq();
		MS5607_chip_select(altimeter);
		if (HAL_SPI_TransmitReceive(altimeter->SPI_BUS, tx, rx, 3, MS5607_TIMEOUT)
				== HAL_TIMEOUT) {
		}
		MS5607_chip_release(altimeter);
		__enable_irq();

		altimeter->constants[i] = rx[1] * pow(2, 8) + rx[2];
	}
}

void MS5607_first_conversion(MS5607_Altimeter *altimeter) {

	// Send the command to convert digital pressure
	//  the exact commands depends on the selected oversampling rate
	switch (altimeter->OSR) {
		case OSR_256:
			MS5607_write_to_spi(altimeter, MS5607_CONVERT_D1_OSR_256_CMD);
			break;
		case OSR_512:
			MS5607_write_to_spi(altimeter, MS5607_CONVERT_D1_OSR_512_CMD);
			break;
		case OSR_1024:
			MS5607_write_to_spi(altimeter, MS5607_CONVERT_D1_OSR_1024_CMD);
			break;
		case OSR_2048:
			MS5607_write_to_spi(altimeter, MS5607_CONVERT_D1_OSR_2048_CMD);
			break;
		case OSR_4096:
			MS5607_write_to_spi(altimeter, MS5607_CONVERT_D1_OSR_4096_CMD);
			break;
	}
}

void MS5607_second_conversion(MS5607_Altimeter *altimeter) {

	// Read in the digital pressure that was previously converted in first_conversion()
	altimeter->D1 = MS5607_read_from_ADC(altimeter);

	// Send the command to convert digital temperature
	//  the exact commands depends on the selected oversampling rate
	switch (altimeter->OSR) {
		case OSR_256:
			MS5607_write_to_spi(altimeter, MS5607_CONVERT_D2_OSR_256_CMD);
			break;
		case OSR_512:
			MS5607_write_to_spi(altimeter, MS5607_CONVERT_D2_OSR_512_CMD);
			break;
		case OSR_1024:
			MS5607_write_to_spi(altimeter, MS5607_CONVERT_D2_OSR_1024_CMD);
			break;
		case OSR_2048:
			MS5607_write_to_spi(altimeter, MS5607_CONVERT_D2_OSR_2048_CMD);
			break;
		case OSR_4096:
			MS5607_write_to_spi(altimeter, MS5607_CONVERT_D2_OSR_4096_CMD);
			break;
	}
}

void MS5607_calculate_pressure_and_temperature(MS5607_Altimeter *altimeter,
		float *final_pressure, float *final_temperature) {

	// Read in the digital temperature that was previously converted in first_conversion()
	altimeter->D2 = MS5607_read_from_ADC(altimeter);

	// At this point, the digital pressure and temperature values (D1 & D2) must be converted
	//  into the actual pressure and temperature through calculations specified in the datasheet

	// TEMPERATURE CALCULATION

	// Difference between actual and reference temperature
	// dT = D2 - C5*(2^8)
	int32_t dT = altimeter->D2 - (altimeter->constants[5] * pow(2, 8));
	dT = SET_VALUE_BETWEEN_RANGE(dT, MS5607_MIN_DT, MS5607_MAX_DT);

	// Actual temperature (last 2 digits in base 10 are decimals; units are Celsius*100)
	// TEMP = 2000 + dT * (C6 / 2^23)
	int32_t TEMP = MS5607_REFERENCE_TEMPERATURE
			+ dT * (altimeter->constants[6] * pow(2, -23));

	// Offset at actual temperature
	// OFF = (C2 * 2^17) + (C4 * dT) / 2^6
	int64_t OFF = (altimeter->constants[2] * pow(2, 17))
			+ dT * (altimeter->constants[4] * pow(2, -6));
	OFF = SET_VALUE_BETWEEN_RANGE(OFF, MS5607_MIN_OFF, MS5607_MAX_OFF);

	// Sensitivity at actual temperature
	// SENS = (C1 * 2^16) + (C3 * dT) / 2^7
	int64_t SENS = (altimeter->constants[1] * pow(2, 16))
			+ dT * (altimeter->constants[3] * pow(2, -7));
	SENS = SET_VALUE_BETWEEN_RANGE(SENS, MS5607_MIN_SENS, MS5607_MAX_SENS);

	// TEMPERATURE COMPENSATION CALCULATIONS
	//  for low temperatures

	int32_t T2 = 0;
	int32_t OFF2 = 0;
	int32_t SENS2 = 0;

	// if temperature is low (TEMP < 20 C), compensate
	if (TEMP < MS5607_LOW_TEMPERATURE) {
		T2 = (dT * dT) >> 31;
		OFF2 = 61 * pow(TEMP - MS5607_REFERENCE_TEMPERATURE, 2) * pow(2, -4);
		SENS2 = 2 * pow(TEMP - MS5607_REFERENCE_TEMPERATURE, 2);
	}
	// if temperature is very low (TEMP < -15 C), compensate again
	if (TEMP < MS5607_VERY_LOW_TEMPERATURE) {
		OFF2 += 15 * pow(TEMP - MS5607_VERY_LOW_TEMPERATURE, 2);
		SENS2 += 8 * pow(TEMP - MS5607_VERY_LOW_TEMPERATURE, 2);
	}

	TEMP -= T2;
	OFF -= OFF2;
	SENS -= SENS2;

	// PRESSURE CALCULATION

	// Temperature compensated pressure (units are mbar/100)
	// P = (D1*SENS/(2^21) - OFF) / (2^15)
	int32_t P = (altimeter->D1 * (SENS * pow(2, -21)) - OFF) * pow(2, -15);

	// CONVERTING INTO NORMAL UNITS
	// The numbers represented by P and TEMP are 100 times the actual value
	//  i.e. 2000 = 20 C, 110002 = 1100.02 mbar

	*final_pressure = (float) P / 100.0;
	*final_temperature = (float) TEMP / 100.0;

	// ALTITUDE CALCULATION

}

/**
 * Helper function to transmit a 1 byte message to MS5607 altimeter over SPI
 *
 * @param altimeter     <MS5607_Altimeter*>                 altimeter object to store settings and constants
 * @param tx            <uint8_t*>                          value to write to the MS5607
 *
 */
void MS5607_write_to_spi(MS5607_Altimeter *altimeter, uint8_t tx) {
	// disable inturrupts during transmission and select the chip
	__disable_irq();
	MS5607_chip_select(altimeter);

	// send a 1 byte command
	if (HAL_SPI_Transmit(altimeter->SPI_BUS, &tx, 1, MS5607_TIMEOUT)
			== HAL_TIMEOUT) {
	}

	// re-enable inturrupts after transmission and deselect the chip
	MS5607_chip_release(altimeter);
	__enable_irq();
}

/**
 * Helper function to read the value last converted by the altimeter's ADC over SPI
 *
 * @param altimeter     <MS5607_Altimeter*>      altimeter object to store settings and constants
 *
 */
uint32_t MS5607_read_from_ADC(MS5607_Altimeter *altimeter) {

	// prepare buffer for the data to be written (ADC_READ_CMD)
	uint8_t tx[4] = { MS5607_ADC_READ_CMD, 0, 0, 0 }; // {ADC_READ_COMMAND, [not used], [not used], [not used]}
	// prepare buffer to receive the 24 bits of data (stored in the last 3 indeces of the rx array)
	// these 8 bit integers must be later converted to a 32 bit integer
	uint8_t rx[4] = { 0, 0, 0, 0 }; // {[not used], [uninitialized], [uninitialized], [uninitialized]}

	// disable inturrupts during transmission and select the chip
	__disable_irq();
	MS5607_chip_select(altimeter);

	// transmit and receive the data - 4 bytes
	if (HAL_SPI_TransmitReceive(altimeter->SPI_BUS, tx, rx, 4, MS5607_TIMEOUT)
			== HAL_TIMEOUT) {
	}

	// re-enable inturrupts after transmission and deselect the chip
	MS5607_chip_release(altimeter);
	__enable_irq();

	// rx_read_results now contains the desired data as separate 8 bit values - convert them into a 32 bit integer
	// add bits starting with MSB (most significant bit) and bit shift left by 8; repeat.
	// first byte is junk - don't return it
	//return rx[1] * pow(2, 16) + rx[2] * pow(2, 8) + rx[3];
	return ((((uint32_t) rx[1] << 8) + (uint32_t) rx[2]) << 8) + (uint32_t) rx[3];
}

/**
 * Helper function to set the CS pin active
 *
 * @param altimeter     <MS5607_Altimeter*>      altimeter object to store settings and constants
 *
 */
void MS5607_chip_select(MS5607_Altimeter *altimeter) {
	HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_ACTIVE);
}

/**
 * Helper function to set the CS pin inactive
 *
 * @param altimeter     <MS5607_Altimeter*>      altimeter object to store settings and constants
 *
 */
void MS5607_chip_release(MS5607_Altimeter *altimeter) {
	HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_INACTIVE);
}
