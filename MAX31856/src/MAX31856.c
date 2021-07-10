/** MAX31856.c
 * This is MASA's library for the MAX31856 thermocouple library
 * Datasheet: https://datasheets.maximintegrated.com/en/ds/MAX31856.pdf
 *
 * Samantha Liu (samzliu@umich.edu)
 * Michigan Aeronautical Science Association
 * Created June 19, 2021
 * Last edited: July 8 2021
 */

#include "MAX31856.h"


// Define SPI commands, status bits, registers, here
// Make sure to include the part number at the front of all #defines
#define MAX31856_CFG_REG_1_TC_TYPE_MASK (0x07)

// Datasheet pg 24-25, concatenate the 3 bytes and convert to real number
#define MAX31856_LNRZD_TC_TEMP_B2 (0x0C) //High
#define MAX31856_LNRZD_TC_TEMP_B1 (0x0D)
#define MAX31856_LNRZD_TC_TEMP_B0 (0x0E)

// Add any additional defines here as required
// Datasheet pg 20, stuff related to TC TYPE[3:0]
#define MAX31856_CR1_REG_Read (0x01)
#define MAX31856_CR1_REG_Write (0x81)
#define MAX31856_TCTYPE_T ((uint8_t) 0b0111)

//Datasheet pg 19, Configuration 0 Register for changing conversion mode
#define MAX31856_CR0_REG_Read (0x00)
#define MAX31856_CR0_REG_Write (0x80)

#define MAX31856_TIMEOUT (0x01)
// Add any private helper functions here as required


void MAX31856_init_thermocouples(MAX31856_TC_Array* tcs) {
	// MAX31856 is K type default on power up
	// Iterate through all TCs in the array and switch them to T type

	// Make sure each thermocouple automatically converts data.
	// This might be the default, but check the datasheet.
	// We want to be able to read it by only using chip select and MISO,
	// and shouldn't need to transmit commands.

	uint8_t reg_addr[1];
	uint8_t rx[1];
	uint8_t tx[2];
	uint8_t check[1];
  
  for (uint8_t i = 0; i < tcs->num_tcs; i++) {
    // Read the type (default should be K) in CR1_REG
	reg_addr[0] = MAX31856_CR1_REG_Read;
	rx[0] = 0;
    __disable_irq();
    (*tcs->chip_select)(i);
    HAL_SPI_Transmit(tcs->SPI_bus, (uint8_t *)reg_addr, 1, MAX31856_TIMEOUT);
    HAL_SPI_Receive(tcs->SPI_bus, (uint8_t *)rx, 1, MAX31856_TIMEOUT); //interrupt?
    (*tcs->chip_release)(i);
    __enable_irq();

    // Switch to type T
    rx[0] &= 0xF0; // mask off bottom 4 bits, clear bits 3:0
    rx[0] |= MAX31856_TCTYPE_T;
    
    // Write the register
    tx[0] = MAX31856_CR1_REG_Write;
    tx[1] = rx[0];
    __disable_irq();
    (*tcs->chip_select)(i);
    HAL_SPI_Transmit(tcs->SPI_bus, (uint8_t *)tx, 2, MAX31856_TIMEOUT);
    (*tcs->chip_release)(i);
    __enable_irq();

    // Checking the register
    check[0] = 0;
    __disable_irq();
    (*tcs->chip_select)(i);
    HAL_SPI_Transmit(tcs->SPI_bus, (uint8_t *)reg_addr, 1, MAX31856_TIMEOUT);
    HAL_SPI_Receive(tcs->SPI_bus, (uint8_t *)check, 2, MAX31856_TIMEOUT);
    (*tcs->chip_release)(i);
    __enable_irq();


    // Change to automatic conversion every 100 ms (datasheet p19)
	reg_addr[0] = MAX31856_CR0_REG_Read;
    rx[0] = 0;
    __disable_irq();
    (*tcs->chip_select)(i);
    HAL_SPI_Transmit(tcs->SPI_bus, (uint8_t *)reg_addr, 1, MAX31856_TIMEOUT);
    HAL_SPI_Receive(tcs->SPI_bus, (uint8_t *)rx, 1, MAX31856_TIMEOUT); //interrupt?
    (*tcs->chip_release)(i);
    __enable_irq();

    // Change bit 7 to high, Automatic Conversion mode
    rx[0] |= 0b10000000;

    // Write the register CR0
    tx[0] = MAX31856_CR0_REG_Write;
    tx[1] = rx[0];
    __disable_irq();
    (*tcs->chip_select)(i);
    HAL_SPI_Transmit(tcs->SPI_bus, (uint8_t *)tx, 2, MAX31856_TIMEOUT);
    (*tcs->chip_release)(i);
    __enable_irq();
  }
}

float MAX31856_read_thermocouple(MAX31856_TC_Array* tcs, uint8_t tc_index) {
	// Read the temperature value from a single thermocouple chip, specified by tc_index.
	// Then convert that number into a real temp in Celcius, then convert that to Kelvin and return it.
  
  uint8_t reg_addr[1] = {MAX31856_LNRZD_TC_TEMP_B2};
  uint8_t rx[3] = { 0, 0, 0 };
  uint32_t temp32;
  float real_temp_c;
  
  // Write into rx
  __disable_irq();
  (*tcs->chip_select)(tc_index);
  HAL_SPI_Transmit(tcs->SPI_bus, (uint8_t *)reg_addr, 1, MAX31856_TIMEOUT);
  HAL_SPI_Receive(tcs->SPI_bus, (uint8_t *)rx, 3, MAX31856_TIMEOUT); //DRDY?
  (*tcs->chip_release)(tc_index);
  __enable_irq();
  
  // Convert rx into real_temp
  temp32 = 0b00000000| rx[0] << 16 | rx[1] << 8 | rx[2];
  if (temp32 & 0x800000) { //if first bit is 1
      temp32 |= 0xFF000000; // fix sign
  }
  temp32 >>= 5;
  real_temp_c = temp32 / 128.0;
  
  // Convert from Celsius to Kelvin and return
  return real_temp_c + 273.15;
}
