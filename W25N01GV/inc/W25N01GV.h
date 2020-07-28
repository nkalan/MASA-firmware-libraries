/**
 * Header file for communicating with W25N01GV Flash Memory
 * Datasheet: https://www.winbond.com/resource-files/w25n01gv%20revl%20050918%20unsecured.pdf
 * 
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created July 20, 2020
 * Last edited July 27, 2020
 */

#ifndef W25N01GV_H	// begin header include protection
#define W25N01GV_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

#ifdef HAL_SPI_MODULE_ENABLED	// begin SPI include protection

/**
 * Continuous read mode:
 */
typedef enum {
	CONTINUOUS, BUFFER
} W25N01GV_ReadMode;

typedef struct {
	SPI_HandleTypeDef *SPI_bus;   // SPI struct, specified by user
	GPIO_TypeDef *cs_base;        // Chip select GPIO base, specified by user
	uint16_t cs_pin;              // Chip select GPIO pin, specified by user
	W25N01GV_ReadMode;						// Default is BUFFER for the W25N01GVZEIG model


} W25N01GV_Flash;

void W25N01GV_init_flash(W25N01GV_Flash *flash, SPI_HandleTypeDef *SPI_bus_in,
		GPIO_TypeDef *cs_base_in, /*GPIO_TypeDef *wp_base_in,
		GPIO_TypeDef *hold_base_in,*/ uint16_t cs_pin_in/*, uint16_t wp_pin_in,
		uint16_t hold_pin_in*/);


// Reads data from memory to buffer
void W25N01GV_page_read_memory(W25N01GV_Flash *flash, uint16_t column_addr,
		uint8_t *data_buf, uint16_t buf_size);

// Writes data to memory from buffer
void W25N01GV_page_load_memory(W25N01GV_Flash *flash, uint16_t column_addr,
		uint8_t *data_buf, uint16_t buf_size);

// Reads status of selected status registar
uint8_t W25N01GV_read_status_data(W25N01GV_Flash *flash, uint8_t status_cmd,
		uint8_t status_addr);

// Writes status to selected status registar
void W25N01GV_write_status_data(W25N01GV_Flash *flash, uint8_t status_cmd,
		uint8_t status_addr, uint8_t status_data);

// Erases whole device memory
void W25N01GV_erase_chip(W25N01GV_Flash *flash);

// Resets device
void W25N01GV_software_reset(W25N01GV_Flash *flash);

#endif	// end SPI include protection
#endif	// end header include protection
