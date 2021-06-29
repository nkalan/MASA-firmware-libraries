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
// Datasheet pg 20, stuff related to TC TYPE[3:0]
#define MAX31856_CR1_REG_Read (0x01)
#define MAX31856_CR1_REG_Write (0x81)
#define MAX31856_TCTYPE_T (0b0111) // should I #define here?

// Add any private helper functions here as required


void MAX31856_init_thermocouples(MAX31856_TC_Array* tcs) {
	// MAX31856 is K type default on power up
	// Iterate through all TCs in the array and switch them to T type

	// Make sure each thermocouple automatically converts data.
	// This might be the default, but check the datasheet.
	// We want to be able to read it by only using chip select and MISO,
	// and shouldn't need to transmit commands.
  
  for (uint8_t i = 0; i < tcs->num_tcs; i++) {
    // Read the type (default should be K) in CR1_REG
    uint8_t reg_addr[1] = {MAX31856_CR1_REG_Read};
    uint8_t type[1] = {0};
    tcs->chip_select(i)
    HAL_SPI_Transmit(tcs->SPI_bus, (uint8_t *)reg_addr, 1, 1)
    HAL_SPI_Receive(tcs->SPI_bus, (uint8_t *)type, 1, 1); //interrupt?
    HAL_GPIO_WritePin(1);
    
    // Switch to type T
    type[0] &= 0xF0; // mask off bottom 4 bits
    type[0] |= (uint8_t) MAX31856_TCTYPE_T & 0x0F;
    
    // Write the register
    uint8_t tx[2] = {MAX31856_CR1_REG_Write, type[0]};
    tcs->chip_select(i)
    HAL_SPI_Transmit(tcs->SPI_bus, (uint8_t *)tx, 2, 1);
    HAL_GPIO_WritePin(1);
  }
}

float MAX31856_read_thermocouple(MAX31856_TC_Array* tcs, uint8_t tc_index) {
	// Read the temperature value from a single thermocouple chip, specified by tc_index.
	// Then convert that number into a real temp in Celcius, then convert that to Kelvin and return it.
  
  uint8_t reg_addr[1] = {MAX31856_LNRZD_TC_TEMP_B0}
  uint8_t rx[3] = { 0, 0, 0 };
  int24_t temp24;
  float real_temp_c;
  
  // Write into rx
  tcs->chip_select(i)
  HAL_SPI_Transmit(tcs->SPI_bus, (uint8_t *)reg_addr, 1, 1)
  HAL_SPI_Receive(tcs->SPI_bus, (uint8_t *)rx, 3, 1); //DRDY?
  HAL_GPIO_WritePin(1);
  
  // Convert rx into real_temp
  temp24 = rx[0] << 16 | rx[1] << 8 | rx[2]
  temp24 >>= 5
  real_temp_c = temp24 / 128
  
  // Convert from Celsius to Kelvin and return
  return real_temp + 273.15;
}
