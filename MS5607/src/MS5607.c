/**
 * Code Implementation MS5607 Pressure Altimeter firmware library
 * Datasheet: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=srchrtrv&DocNm=MS5607-02BA03&DocType=Data+Sheet&DocLang=English
 *
 * From the datasheet: "The MS5607-02BA consists of a piezo-resistive sensor
 * and a sensor interface IC. The main function of the MS5607-02BA is to
 * convert the uncompensated analogue output voltage from the piezo-resistive
 * pressure sensor to a 24-bit digital value, as well as providing a 24-bit
 * digital value for the temperature of the sensor."
 * 
 * Operating Temperature range: -40 dg C to 85 dg C
 * Operating Pressure range: 10 mbar to 1200 mbar
 *
 * The altimeter supplies pressure and temperature, which is then converted to
 * altitude according to the 1976 U.S. Standard Atmosphere model.
 * 
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created May 3, 2020
 * Last edited August 12, 2020
 */

#include <stdint.h>
#include <math.h>
#include "MS5607.h"
#include "MS5607_altitude_conversion.h"

#ifdef HAL_SPI_MODULE_ENABLED	// begin SPI include protection

/** Command bytes 
 * Each command has a corresponding 1-byte sequence
 * 
 * The meaning of each bit is documented on the datasheet p.10:
 * Bit #    Bit name	 	Description
 * =======================================================
 * 0        PRM					PROM
 * 1        COV					Convert
 * 2        -						-
 * 3        Typ					?
 * 4        Ad2/Os2			PROM Address 2/Oversampling Rate 2
 * 5        Ad1/Os1			PROM Address 1/Oversampling Rate 1
 * 6        Ad0/Os0			PROM Address 0/Oversampling Rate 0
 * 7        Stop				Stop bit, always 0
 *
 * A note about PROM addresses:
 * Each of the 8 addresses stores a 16 bit value, shown here.
 *
 * Address 0:		Reserved for manufacturer
 * Address 1-6: Calibration constants
 * Address 7:		Serial code, CRC in last 4 bits
 *
 * To get the command for address i, 0 <= i <= 7, add 2*i
 * to MS5607_PROM_READ_BASE.
 */

// Initialization commands
#define MS5607_RESET          (uint8_t) 0x1E  // 0b00011110
#define MS5607_PROM_READ_BASE (uint8_t) 0xA0  // 0b1010---0

// ADC conversion commands to be run every loop
#define MS5607_D1_OSR_256     (uint8_t) 0x40  // 0b01000000
#define MS5607_D1_OSR_512     (uint8_t) 0x42  // 0b01000010
#define MS5607_D1_OSR_1024    (uint8_t) 0x44  // 0b01000100
#define MS5607_D1_OSR_2048    (uint8_t) 0x46  // 0b01000110
#define MS5607_D1_OSR_4096    (uint8_t) 0x48  // 0b01001000
#define MS5607_D2_OSR_256     (uint8_t) 0x50  // 0b01010000
#define MS5607_D2_OSR_512     (uint8_t) 0x52  // 0b01010010
#define MS5607_D2_OSR_1024    (uint8_t) 0x54  // 0b01010100
#define MS5607_D2_OSR_2048    (uint8_t) 0x56  // 0b01010110
#define MS5607_D2_OSR_4096    (uint8_t) 0x58  // 0b01011000

// ADC read command to be run every loop
#define MS5607_ADC_READ       (uint8_t) 0x00  // 0b00000000

/**
 * Maximum ADC conversion times, which vary depending on the oversampling rate
 * specified during initialization.
 */
#define MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_256    1
#define MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_512    2
#define MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_1024   3
#define MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_2048   5
#define MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_4096  10

// Chip is active low
#define MS5607_CS_ACTIVE    GPIO_PIN_RESET
#define MS5607_CS_INACTIVE  GPIO_PIN_SET

// Timeout for SPI
#define MS5607_TIMEOUT      1

// Operating ranges of sensor
#define MS5607_MIN_PRESSURE      1000  // 10 millibars
#define MS5607_MAX_PRESSURE    120000  // 1200 millibars
#define MS5607_MIN_TEMPERATURE  -4000  // -40 dg C
#define MS5607_MAX_TEMPERATURE   8500  // 85 dg C

/* Private function declarations */

static HAL_StatusTypeDef MS5607_write_cmd_to_SPI(MS5607_Altimeter *altimeter,
    uint8_t tx);
static HAL_StatusTypeDef MS5607_read_from_adc(MS5607_Altimeter *altimeter,
    uint32_t *value);

/* Public function definitions */

HAL_StatusTypeDef MS5607_altimeter_init(MS5607_Altimeter *altimeter,
    MS5607_OversamplingRate OSR_in, SPI_HandleTypeDef *SPI_bus_in,
    GPIO_TypeDef *cs_base_in, uint16_t cs_pin_in) {
	altimeter->OSR = OSR_in;
	altimeter->SPI_bus = SPI_bus_in;
	altimeter->cs_base = cs_base_in;
	altimeter->cs_pin = cs_pin_in;

	HAL_StatusTypeDef SPI_status_code = HAL_OK;

	// Reset the device PROM
	SPI_status_code |= MS5607_write_cmd_to_SPI(altimeter, MS5607_RESET);
	HAL_Delay(3); 	// Minimum of 2.88 ms to completely reset the chip

	// Send the 1 byte PROM read command, and the next 2 bytes clocked out
	// immediately after (MSB first) is the desired number. 8 times, once
	// for each constant.
	for (int i = 0; i < 8; i++) {
		/* Prepare SPI buffers */
		// The 8 ADC PROM read commands are found from MS5607_PROM_READ_BASE.
		// To get command i, 0 <= i <= 7, add 2*i to MS5607_PROM_READ_BASE.
		uint8_t tx[3] = { MS5607_PROM_READ_BASE + i * 2, 0, 0 }; // { command, unused, unused }
		uint8_t rx[3] = { 0, 0, 0 };  // { unused, empty, empty }

		// Transmit and receive over SPI
		__disable_irq();  // disable interrupt requests
		HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_ACTIVE); // select chip
		SPI_status_code |= HAL_SPI_TransmitReceive(altimeter->SPI_bus, tx, rx, 3,
		MS5607_TIMEOUT);
		HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin,
		    MS5607_CS_INACTIVE);  // release chip
		__enable_irq();  // re enable interrupt requests

		// Pack the 2 bytes received into 1 number and store it
		altimeter->constants[i] = ((uint16_t) rx[1] << 8) + rx[2];
	}

	return SPI_status_code;
}

HAL_StatusTypeDef MS5607_convert_pressure(MS5607_Altimeter *altimeter) {
	uint8_t conversion_cmd;

	// Select command for ADC to convert pressure
	switch (altimeter->OSR) {
		case OSR_256:
			conversion_cmd = MS5607_D1_OSR_256;
			break;
		case OSR_512:
			conversion_cmd = MS5607_D1_OSR_512;
			break;
		case OSR_1024:
			conversion_cmd = MS5607_D1_OSR_1024;
			break;
		case OSR_2048:
			conversion_cmd = MS5607_D1_OSR_2048;
			break;
		case OSR_4096:
			conversion_cmd = MS5607_D1_OSR_4096;
			break;
		default:
			// Select the command with the shortest conversion time by default
			conversion_cmd = MS5607_D1_OSR_256;
	}

	// Send the command and return the status code
	return MS5607_write_cmd_to_SPI(altimeter, conversion_cmd);
}

HAL_StatusTypeDef MS5607_read_raw_pressure(MS5607_Altimeter *altimeter) {
	return MS5607_read_from_adc(altimeter, &(altimeter->D1));
}

HAL_StatusTypeDef MS5607_convert_temperature(MS5607_Altimeter *altimeter) {
	uint8_t conversion_cmd;

	// Select command for ADC to convert pressure
	switch (altimeter->OSR) {
		case OSR_256:
			conversion_cmd = MS5607_D2_OSR_256;
			break;
		case OSR_512:
			conversion_cmd = MS5607_D2_OSR_512;
			break;
		case OSR_1024:
			conversion_cmd = MS5607_D2_OSR_1024;
			break;
		case OSR_2048:
			conversion_cmd = MS5607_D2_OSR_2048;
			break;
		case OSR_4096:
			conversion_cmd = MS5607_D2_OSR_4096;
			break;
		default:
			// Select the command with the shortest conversion time by default
			conversion_cmd = MS5607_D2_OSR_256;
	}

	// Send the command and return the status code
	return MS5607_write_cmd_to_SPI(altimeter, conversion_cmd);
}

HAL_StatusTypeDef MS5607_read_raw_temperature(MS5607_Altimeter *altimeter) {
	return MS5607_read_from_adc(altimeter, &(altimeter->D2));
}

uint8_t MS5607_get_adc_conversion_time(MS5607_Altimeter *altimeter) {
	// Each oversampling rate setting has a different ADC conversion time
	switch (altimeter->OSR) {
		case OSR_256:
			return MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_256;
			break;
		case OSR_512:
			return MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_512;
			break;
		case OSR_1024:
			return MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_1024;
			break;
		case OSR_2048:
			return MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_2048;
			break;
		case OSR_4096:
			return MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_4096;
			break;
		default:
			// Return the longest delay by default
			return MS5607_ADC_CONVERSION_TIME_MILLISECONDS_OSR_4096;
			break;
	}
}

void MS5607_calculate_pressure_and_temperature(MS5607_Altimeter *altimeter,
    int32_t *final_pressure, int32_t *final_temperature) {
	/**
	 * Calculations for finding actual temperature and pressure are documented
	 * on pages 8-9 of the datasheet.
	 *
	 * In the datasheet, Ci is the ith calibration constant stored in the
	 * MS5607_Altimeter struct.
	 *
	 * pow() is used when multiplying powers of 2, not bit shift (<< and >>)
	 * to avoid type errors and integer overflows.
	 *
	 * The intermediate variables dT, OFF, OFF2, and SENS2 have explicit casts
	 * to int64_t in their expressions to avoid integer overflow errors.
	 */

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

//TODO: finish lookup table generator
uint32_t MS5607_calculate_altitude(MS5607_Altimeter *altimeter) {
	/**
	 * Outputting pressure and temperature is incredibly useful for debugging,
	 * but the user probably never needs to see it.
	 *
	 * The pressure/temperature calculation is handled in a separate
	 * function in case the user ever wants to access those values
	 * directly [MS5607_calculate_pressure_and_temperature()]
	 */

	int32_t pressure, temperature;
	MS5607_calculate_pressure_and_temperature(altimeter, &pressure, &temperature);

	// Return -1 for pressures outside the operating range
	if (pressure < MS5607_MIN_PRESSURE || MS5607_MAX_PRESSURE < pressure)
		return -1;
	else
		// Minimum pressure is index 0
		return MS5607_pressure_to_altitude_lookup_table[pressure
		    - MS5607_MIN_PRESSURE];
}

/* Private function definitions */

/**
 * Private helper function to transmit 1 byte to the MS5607 over SPI.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 * @param tx            			<uint8_t>                   1 byte to write to the altimeter
 * @retval SPI status code
 */
static HAL_StatusTypeDef MS5607_write_cmd_to_SPI(MS5607_Altimeter *altimeter,
    uint8_t tx) {
	HAL_StatusTypeDef SPI_status_code;

	__disable_irq();	// disable interrupts
	HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_ACTIVE); // select chip
	SPI_status_code = HAL_SPI_Transmit(altimeter->SPI_bus, &tx, 1,
	    MS5607_TIMEOUT);
	HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_INACTIVE); // release chip
	__enable_irq();  // re enable interrupts

	return SPI_status_code;
}

/**
 * Private helper function to read from the MS5607's ADC.
 *
 * This function sends the 1 byte ADC read command, and the next 3 bytes
 * clocked out immediately after (MSB first) by the MS5607 is the desired
 * number, which is packed into a single uint32_t and stored in the
 * provided pointer.
 *
 * @param altimeter         	<MS5607_Altimeter*>         Struct to store altimeter settings and constants
 * @param value               <uint32_t*>                 Pointer to store the ADC's contents in
 * @retval SPI status code
 */
static HAL_StatusTypeDef MS5607_read_from_adc(MS5607_Altimeter *altimeter,
    uint32_t *value) {
	HAL_StatusTypeDef SPI_status_code;

	// Prepare SPI buffers
	uint8_t tx[4] = { MS5607_ADC_READ, 0, 0, 0 }; // { ADC read command, unused, unused, unused }
	uint8_t rx[4] = { 0, 0, 0, 0 };  // { unused, empty, empty, empty }

	// Transmit and receive over SPI
	__disable_irq();	// disable interrupts
	HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_ACTIVE); // select chip
	SPI_status_code = HAL_SPI_TransmitReceive(altimeter->SPI_bus, tx, rx, 4,
	    MS5607_TIMEOUT);
	HAL_GPIO_WritePin(altimeter->cs_base, altimeter->cs_pin, MS5607_CS_INACTIVE); // release chip
	__enable_irq();	 //re enable interrupts

	// Pack the 3 bytes read from the MS5607's ADC into 1 number.
	// first byte of rx is junk - don't return it
	*value = ((((uint32_t) rx[1] << 8) + (uint32_t) rx[2]) << 8)
	    + (uint32_t) rx[3];

	return SPI_status_code;
}

#endif	// end SPI include protection
