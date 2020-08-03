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

#ifdef HAL_SPI_MODULE_ENABLED	// begin SPI include protection

typedef struct {
	SPI_HandleTypeDef *SPI_bus;   // SPI struct, specified by user
	GPIO_TypeDef *cs_base;        // Chip select GPIO base, specified by user
	uint16_t cs_pin;              // Chip select GPIO pin, specified by user

	uint16_t last_page_loaded;

	uint16_t current_page;
	uint16_t next_free_column;

} W25N01GV_Flash;

void init_flash(W25N01GV_Flash *flash, SPI_HandleTypeDef *SPI_bus_in,
		GPIO_TypeDef *cs_base_in, uint16_t cs_pin_in);

void unlock_flash(W25N01GV_Flash *flash);

void lock_flash(W25N01GV_Flash *flash);

void enable_ECC(W25N01GV_Flash *flash);
void disable_ECC(W25N01GV_Flash *flash);

void write_to_flash(W25N01GV_Flash *flash, uint8_t *data, uint16_t size);

void write_bytes(W25N01GV_Flash *flash, uint8_t *buffer, uint16_t buffer_size, uint16_t page_adr, uint16_t column_adr);

void read_bytes(W25N01GV_Flash *flash, uint8_t *buffer, uint16_t buffer_size,
		uint16_t page_num, uint16_t column_num);

void read_2KB_and_advance(W25N01GV_Flash *flash, uint8_t *buffer,
		uint16_t buffer_size, uint16_t *page_num);

uint8_t reset_flash(W25N01GV_Flash *flash);

void erase_chip(W25N01GV_Flash *flash);

#endif	// end SPI include protection
#endif	// end header include protection
