/**
 * Header file for communicating with W25N01GV Flash Memory
 * Datasheet: https://www.winbond.com/resource-files/w25n01gv%20revl%20050918%20unsecured.pdf
 * 
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created July 20, 2020
 * Last edited September 12, 2020
 */

#ifndef W25N01GV_H	// begin header include protection
#define W25N01GV_H

#include "stm32f4xx_hal.h"

#ifdef HAL_SPI_MODULE_ENABLED	// begin SPI include protection

typedef struct {
	SPI_HandleTypeDef *SPI_bus;   // SPI struct, specified by user
	GPIO_TypeDef *cs_base;        // Chip select GPIO base, specified by user
	uint16_t cs_pin;              // Chip select GPIO pin, specified by user

	uint16_t current_page;
	uint16_t next_free_column;

	uint16_t next_page_to_read;

} W25N01GV_Flash;

/**
 * Value representing the status of the last read command. Error correction
 * algorithms are run internally on the flash chip, and the ECC1 and ECC0 bits
 * in the status register (SR3) display the output. These bits are read by the
 * function get_ECC_status().
 *
 * TODO: move the documentation back to the .c file
 *
 * datasheet pg 20
 */
typedef enum {
	SUCCESS_NO_CORRECTIONS,    // ECC1 = 0, ECC0 = 0
	SUCCESS_WITH_CORRECTIONS,  // ECC1 = 0, ECC0 = 1
	ERROR_ONE_PAGE,            // ECC1 = 1, ECC0 = 0
	ERROR_MULTIPLE_PAGES       // ECC1 = 1, ECC0 = 1, only used in continuous read mode
} W25N01GV_ECC_Status;

void init_flash(W25N01GV_Flash *flash, SPI_HandleTypeDef *SPI_bus_in,
		GPIO_TypeDef *cs_base_in, uint16_t cs_pin_in);

HAL_StatusTypeDef write_to_flash(W25N01GV_Flash *flash, uint8_t *data, uint32_t size);

void read_address_pointer(W25N01GV_Flash *flash);

void set_address_pointer(W25N01GV_Flash *flash, uint16_t page_adr, uint16_t column_adr);

void reset_flash_read_pointer(W25N01GV_Flash *flash);

HAL_StatusTypeDef read_next_2KB_from_flash(W25N01GV_Flash *flash, uint8_t *buffer);

HAL_StatusTypeDef reset_flash(W25N01GV_Flash *flash);

uint8_t read_status_register(W25N01GV_Flash *flash, uint8_t register_adr);

//W25N01GV_ECC_Status get_ECC_status(W25N01GV_Flash *flash); TODO maybe call this in the read function?

//uint8_t get_write_failure_status(W25N01GV_Flash *flash);

uint8_t erase_flash(W25N01GV_Flash *flash);

void unlock_flash(W25N01GV_Flash *flash);

void lock_flash(W25N01GV_Flash *flash);

uint8_t flash_ID_is_correct(W25N01GV_Flash *flash);

uint8_t write_bytes_to_page(W25N01GV_Flash *flash, uint8_t *buffer,
    uint16_t buffer_size, uint16_t page_adr, uint16_t column_adr);

W25N01GV_ECC_Status read_bytes_from_page(W25N01GV_Flash *flash, uint8_t *buffer,
		uint16_t buffer_size, uint16_t page_num, uint16_t column_num);

uint8_t BBM_look_up_table_is_full(W25N01GV_Flash *flash);

void read_BBM_look_up_table(W25N01GV_Flash *flash, uint16_t *logical_block_addresses, uint16_t *physical_block_addresses);

#endif	// end SPI include protection
#endif	// end header include protection
