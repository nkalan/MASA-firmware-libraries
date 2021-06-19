/*
 * Write some quick information about the library here
 */

#include "MAX31856.h"


// Define SPI commands, status bits, registers, here
// Make sure to include the part number at the front of all #defines
#define MAX31856_CFG_REG_1_TC_TYPE_MASK (0x07)

// Datasheet pg 24-25, concatenate the 3 bytes and convert to real number
#define MAX31856_LNRZD_TC_TEMP_B2 (0x0C)
#define MAX31856_LNRZD_TC_TEMP_B1 (0x0D)
#define MAX31856_LNRZD_TC_TEMP_B0 (0x0E)

// Add any additional defines here as required


// Add any private helper functions here as required


void MAX31856_init_thermocouples(MAX31856_TC_Array* tcs) {
	// MAX31856 is K type default on power up
	// Iterate through all TCs in the array and switch them to T type

	// Make sure each thermocouple automatically converts data.
	// This might be the default, but check the datasheet.
	// We want to be able to read it by only using chip select and MISO,
	// and shouldn't need to transmit commands.
}

float MAX31856_read_thermocouple(MAX31856_TC_Array* tcs, uint8_t tc_index) {
	// Read the temperature value from a single thermocouple chip, specified by tc_index.
	// Then convert that number into a real temp in Celcius, then convert that to Kelvin and return it.
}
