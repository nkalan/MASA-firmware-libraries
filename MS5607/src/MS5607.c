/**
 * Code Implementation for communicating with MS5607 Pressure Altimeter
 * Datasheet: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5607-02BA03%7FB2%7Fpdf%7FEnglish%7FENG_DS_MS5607-02BA03_B2.pdf%7FCAT-BLPS0035
 *
 * From the datasheet: "The MS5607-02BA consists of a piezo-resistive sensor and a sensor
 * interface IC. The main function of the MS5607-02BA is to convert the uncompensated
 * analogue output voltage from the piezo-resistive pressure sensor to a 24-bit digital
 * value, as well as providing a 24-bit digital value for the temperature of the sensor."
 * 
 * The altimeter supplies pressure and temperature, which the user must convert to altitude.
 * 
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created May 3, 2020
 * Last edited July 20, 2020
 */

#include <stdint.h>
#include <math.h>
#include <MS5607.h>
//#include "MS5607_altitude_conversion.c"

/** Command bytes 
 * Each command has a corresponding 1-byte sequence
 * 
 * The meaning of each bit is documented on the datasheet p.10:
 * Bit #    Bit name	 	Description
 * ================================================
 * 0        PRM					PROM
 * 1        COV					Convert
 * 2        -						-
 * 3        Typ					?
 * 4        Ad2/Os2			PROM Address2/Oversampling2
 * 5        Ad1/Os1			PROM Address1/Oversampling1
 * 6        Ad0/Os0			PROM Address0/Oversampling0
 * 7        Stop				Stop bit, always 0
 * ================================================
 *
 * A note about PROM addresses:
 * Each of the 8 addresses stores a value, shown here.
 *
 * Address 0:		Reserved for manufacturer
 * Address 1-6: Calibration constants
 * Address 7:		Serial code and CRC
 *
 * To get the command for address i, 0 <= i <= 7, add 2*i
 * to MS5607_PROM_READ_CMD_BASE.
 */
#define MS5607_RESET_CMD                   	0x1E    // 0b00011110
#define MS5607_ADC_CONVERT_BASE_CMD					0x40		// 0b01000000
#define MS5607_ADC_CONVERT_D1_CMD						0x00		// 0b00000000
#define MS5607_ADC_CONVERT_D2_CMD						0x10		// 0b00010000
#define MS5607_ADC_CONVERT_OSR_256_CMD			0x00		// 0b00000000
#define MS5607_ADC_CONVERT_OSR_512_CMD			0x02		// 0b00000010
#define MS5607_ADC_CONVERT_OSR_1024_CMD			0x04		// 0b00000100
#define MS5607_ADC_CONVERT_OSR_2048_CMD			0x06		// 0b00000110
#define MS5607_ADC_CONVERT_OSR_4096_CMD			0x08		// 0b00001000
#define MS5607_ADC_READ_CMD                	0x00    // 0b00000000
#define MS5607_PROM_READ_CMD_BASE          	0xA0    // 0b10100000

/**
 * ADC Conversion times vary depending on the oversampling rate specified
 * during initialization. If the ADC_READ command is given too early or the
 * CONVERT command is not given at all, the ADC will return 0.
 */
#define MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_256			1
#define MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_512    	2
#define MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_1024 		3
#define MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_2048   	5
#define MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_4096    10

#define MS5607_CS_ACTIVE    GPIO_PIN_RESET  // Chip is active low
#define MS5607_CS_INACTIVE  GPIO_PIN_SET

#define MS5607_TIMEOUT	1	 // Timeout for SPI

/**
 * Pressure and temperature ranges the MS5607 can handle
 */
#define MS5607_MINIMUM_PRESSURE         1000        // 0.01 mbar    (  10 mbar)
#define MS5607_MAXIMUM_PRESSURE         120000      // 0.01 mbar    (1200 mbar)
#define MS5607_MINIMUM_TEMPERATURE     -4000        // 0.01 Celsius (-40 C)
#define MS5607_MAXIMUM_TEMPERATURE      8500        // 0.01 Celsius ( 85 C)

/** Lookup table
 * Function mapping pressure to altitude
 * 
 */
//static const int32_t[]
/**
 * Helper function to set the MS5607's chip select pin active
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 */
void MS5607_chip_select(MS5607_Altimeter *altimeter) {
	HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_ACTIVE);
}

/**
 * Helper function to set the MS5607's chip select pin inactive
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 */
void MS5607_chip_release(MS5607_Altimeter *altimeter) {
	HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_INACTIVE);
}

/**
 * Helper function to transmit 1 byte to the MS5607 over SPI.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 * @param tx            			<uint8_t>                   1 byte to write to the altimeter
 */
void MS5607_write_cmd_to_spi(MS5607_Altimeter *altimeter, uint8_t tx) {
	__disable_irq();	// disable interrupts
	MS5607_chip_select(altimeter);
	if (HAL_SPI_Transmit(altimeter->SPI_BUS, &tx, 1, MS5607_TIMEOUT)
			== HAL_TIMEOUT) {
	}
	MS5607_chip_release(altimeter);
	__enable_irq();  // re enable interrupts
}

/**
 * Helper function to read from the MS5607's ADC
 *
 * This function sends the 1 byte ADC read command, and the next 3 bytes
 * clocked out immediately after (MSB first) by the MS5607 is the desired
 * number, which is packed into a single uint32_t and returned.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 * @retval The 24 bit value stored in the MS5607's ADC
 */
uint32_t MS5607_read_from_ADC(MS5607_Altimeter *altimeter) {

	// Prepare SPI buffers
	uint8_t tx[4] = { MS5607_ADC_READ_CMD, 0, 0, 0 }; // { ADC read command, unused, unused, unused }
	uint8_t rx[4] = { 0, 0, 0, 0 };  // { unused, empty, empty, empty }

	// Transmit and receive over SPI
	__disable_irq();	// disable interrupts
	MS5607_chip_select(altimeter);
	if (HAL_SPI_TransmitReceive(altimeter->SPI_BUS, tx, rx, 4, MS5607_TIMEOUT)
			== HAL_TIMEOUT) {
	}
	MS5607_chip_release(altimeter);
	__enable_irq();	 //re enable interrupts

	// Pack the 3 bytes read from the MS5607's ADC into 1 number
	// first byte of rx is junk - don't return it
	return ((((uint32_t) rx[1] << 8) + (uint32_t) rx[2]) << 8) + (uint32_t) rx[3];
}

/**
 * Helper function to help generate the correct ADC conversion command, based
 * on the oversampling rate.
 *
 * The output of this function is to be added to MS5607_ADC_CONVERT_BASE_CMD,
 * and to either MS5607_ADC_CONVERT_D1_CMD or MS5607_ADC_CONVERT_D2_CMD
 * to generate a complete ADC conversion command.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 * @retval An 8bit number representing the bits in the ADC conversion command that depend on the OSR
 */
uint8_t MS5607_get_ADC_conversion_cmd_OSR(MS5607_Altimeter* altimeter) {
	switch (altimeter->OSR) {
		case OSR_256:
			return MS5607_ADC_CONVERT_OSR_256_CMD;
			break;
		case OSR_512:
			return MS5607_ADC_CONVERT_OSR_512_CMD;
			break;
		case OSR_1024:
			return MS5607_ADC_CONVERT_OSR_1024_CMD;
			break;
		case OSR_2048:
			return MS5607_ADC_CONVERT_OSR_2048_CMD;
			break;
		case OSR_4096:
			return MS5607_ADC_CONVERT_OSR_4096_CMD;
			break;
		default:
			// Use the OSR with the shortest conversion time by default
			return MS5607_ADC_CONVERT_OSR_256_CMD;
			break;
	}
}

/**
 * Helper function to find the time in milliseconds required for the
 * MS5607's ADC to finish converting data, based on the oversampling rate.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 * @retval The time in milliseconds required for the ADC to convert data
 */
uint8_t MS5607_get_ADC_conversion_time_OSR(MS5607_Altimeter* altimeter) {
	switch (altimeter->OSR) {
		case OSR_256:
			return MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_256;
			break;
		case OSR_512:
			return MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_512;
			break;
		case OSR_1024:
			return MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_1024;
			break;
		case OSR_2048:
			return MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_2048;
			break;
		case OSR_4096:
			return MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_4096;
			break;
		default:
			// Return the longest delay by default
			return MS5607_ADC_CONVERSION_DELAY_MILLISECONDS_OSR_4096;
			break;
	}
}

void MS5607_first_initialization(MS5607_Altimeter *altimeter,
		MS5607_OversamplingRate OSR_in, SPI_HandleTypeDef *SPI_BUS_in,
		GPIO_TypeDef *cs_base_in, uint16_t cs_pin_in) {
	altimeter->OSR = OSR_in;
	altimeter->SPI_BUS = SPI_BUS_in;
	altimeter->cs_base = cs_base_in;
	altimeter->cs_pin = cs_pin_in;

	MS5607_write_cmd_to_spi(altimeter, MS5607_RESET_CMD);  //reset the device PROM
}

void MS5607_second_initialization(MS5607_Altimeter *altimeter) {
	/**
	 * The 8 ADC PROM read commands are found from MS5607_PROM_READ_CMD_BASE.
	 * To get command i, 0 <= i <= 7, add 2*i to MS5607_PROM_READ_CMD_BASE.
	 *
	 * This function sends the 1 byte PROM read command, and the next 2 bytes
	 * clocked out immediately after (MSB first) by the MS5607 is the desired
	 * number, which is packed into a single uint16_t and returned. This is done
	 * a total of 8 times, once for each calibration constant.
	 */

	for (int i = 0; i < 8; i++) {
		// Prepare SPI buffers
		uint8_t tx[3] = { MS5607_PROM_READ_CMD_BASE + i * 2, 0, 0 };// { command, unused, unused }
		uint8_t rx[3] = { 0, 0, 0 };  // { unused, empty, empty }

		// Transmit and receive over SPI
		__disable_irq();	// disable interrupt requests
		MS5607_chip_select(altimeter);
		if (HAL_SPI_TransmitReceive(altimeter->SPI_BUS, tx, rx, 3, MS5607_TIMEOUT)
				== HAL_TIMEOUT) {
		}
		MS5607_chip_release(altimeter);
		__enable_irq();  // re enable interrupt requests

		// Pack the 2 bytes received into 1 number and store it
		altimeter->constants[i] = ((uint16_t) rx[1] << 8) + rx[2];
	}
}

void MS5607_first_conversion(MS5607_Altimeter *altimeter) {
	// Send command for ADC to convert temperature
	uint8_t ADC_conversion_cmd = MS5607_ADC_CONVERT_BASE_CMD
			| MS5607_ADC_CONVERT_D1_CMD
			| MS5607_get_ADC_conversion_cmd_OSR(altimeter);	 // bitwise-OR to combine bits, '+' does the same thing
	MS5607_write_cmd_to_spi(altimeter, ADC_conversion_cmd);
}

uint8_t MS5607_first_conversion_delay(MS5607_Altimeter *altimeter) {
	return MS5607_get_ADC_conversion_time_OSR(altimeter);
}

void MS5607_second_conversion(MS5607_Altimeter *altimeter) {
	// Read in digital pressure that should have been previously converted
	altimeter->D1 = MS5607_read_from_ADC(altimeter);

	// Send command for ADC to convert temperature
	uint8_t ADC_conversion_cmd = MS5607_ADC_CONVERT_BASE_CMD
			| MS5607_ADC_CONVERT_D2_CMD
			| MS5607_get_ADC_conversion_cmd_OSR(altimeter);	 // bitwise-OR to combine bits, '+' does the same thing
	MS5607_write_cmd_to_spi(altimeter, ADC_conversion_cmd);
}

uint8_t MS5607_second_conversion_delay(MS5607_Altimeter *altimeter) {
	return MS5607_get_ADC_conversion_time_OSR(altimeter);
}

void MS5607_calculate_pressure_and_temperature(MS5607_Altimeter *altimeter,
		int32_t *final_pressure, int32_t *final_temperature) {
	/**
	 * Calculations for finding actual temperature and pressure are documented
	 * on pages 8-9 of the datasheet.
	 *
	 * Ci is the ith calibration constant stored in the MS5607_Altimeter struct.
	 *
	 * pow() is used when multiplying powers of 2, not bit shift (<< and >>)
	 * to avoid type errors and integer overflows.
	 *
	 * The intermediate variables dT, OFF, OFF2, and SENS2 have explicit casts
	 * to int64_t in their expressions to avoid integer overflow errors
	 */

	// Read in digital temperature that should have been previously converted
	altimeter->D2 = MS5607_read_from_ADC(altimeter);

	// TEMPERATURE CALCULATION
	// Difference between actual and reference temperature
	int64_t dT = altimeter->D2 - (altimeter->constants[5] * pow(2, 8));

	// Actual temperature (0.01 C resolution, 2000 = 20 C, -1550 = -15.5 C, etc)
	int32_t TEMP = 2000 + dT * altimeter->constants[6] * pow(2, -23);

	// Offset at actual temperature
	int64_t OFF = (int64_t) (altimeter->constants[2] * pow(2, 17))
			+ dT * altimeter->constants[4] * pow(2, -6);

	// Sensitivity at actual temperature
	int64_t SENS = altimeter->constants[1] * pow(2, 16)
			+ dT * altimeter->constants[3] * pow(2, -7);

	// COMPENSATION FOR LOW TEMPERATURES
	// Prepare temperature calibration values
	int64_t T2 = 0;
	int64_t OFF2 = 0;
	int64_t SENS2 = 0;

	// If temperature is low (TEMP < 20 C), compensate
	if (TEMP < 2000) {
		T2 = pow(dT, 2) * pow(2, -31);
		OFF2 = 61 * (int64_t) pow(TEMP - 2000, 2) * pow(2, -4);
		SENS2 = 2 * pow(TEMP - 2000, 2);

		// If temperature is very low (TEMP < -15 C) compensate again
		if (TEMP < -1500) {
			OFF2 += 15 * (int64_t) pow(TEMP + 1500, 2);
			SENS2 += 8 * (int64_t) pow(TEMP + 1500, 2);
		}
	}

	TEMP -= T2;
	OFF -= OFF2;
	SENS -= SENS2;

	// PRESSURE CALCULATION
	// Temperature compensated pressure (0.01 mbar, 110002 = 1100.02 mbar, etc)
	int32_t P = (altimeter->D1 * SENS * pow(2, -21) - OFF) * pow(2, -15);

	*final_pressure = P;
	*final_temperature = TEMP;
}
