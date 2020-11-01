/**
 * Header file for communicating with W25N01GV Flash Memory
 * Datasheet: https://www.winbond.com/resource-files/w25n01gv%20revl%20050918%20unsecured.pdf
 * 
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created July 20, 2020
 * Last edited October 24, 2020
 *
 *
 * ============================================================================
 * EXAMPLE CODE
 * ============================================================================
 * // Always used for initialization
 *
 * W25N01GV_Flash flash;
 * init_flash(&flash, &<spi_bus_name>, <GPIO_array>, <GPIO_pin>);
 *
 * ============================================================================
 *
 * // Checking if the SPI bus is working by flashing an LED
 *
 * while (is_flash_id_correct(&flash)) {
 *   HAL_GPIO_TogglePin(...);
 *   HAL_Delay(1000);
 * }
 *
 * ============================================================================
 *
 * // Scanning a new W25N01GV flash chip for bad memory blocks. You should
 * // only have to do this before writing to it for the very first time.
 * // If scan_bad_blocks() returns a nonzero value, there is at least 1
 * // corrupted memory block and you should consider using a different chip.
 *
 * uint16_t bad_blocks[1024];
 * uint16_t num_bad_blocks = scan_bad_blocks(&flash, bad_blocks);
 *
 * ============================================================================
 *
 * // Reading all of flash in 2KB chunks
 * // Note: the number of pages in flash is one greater than the maximum value
 * // of a uint16_t, so you either have to declare the page counter with at
 * // least 32bits or break out of the loop when the counter reaches
 * // W25N01GV_NUM_PAGES, or use some other control logic to avoid an infinite loop.
 *
 * uint32_t page = 0;
 * uint8_t read_buffer[2048];
 * reset_flash_read_pointer(&flash);
 *
 * while (page < W25N01GV_NUM_PAGES) {
 *   read_next_2KB_from_flash(&flash, read_buffer);
 *   // Data gets read into read_buffer, do something with it here
 * }
 *
 * ============================================================================
 *
 * // Writing an array of bytes to flash
 * // Note: if there isn't enough space to write the data, anything over
 * // capacity will get cut off and won't be written.
 *
 * // data is a uint8_t array, num_bytes is the size of data
 * write_to_flash(&flash, data, num_bytes);
 *
 * ============================================================================
 *
 * // Miscellaneous functions
 * TODO reorganize these functions
 *
 * // Erases all data on flash
 * erase_flash(&flash);
 *
 * // Returns the number of bytes available to write to
 * get_bytes_remaining(&flash)
 *
 * // Resets the flash chip to its power-on state
 * reset_flash(&flash);
 *
 * TODO: include delays in all README function descriptions
 */

#ifndef W25N01GV_H	// Begin header include protection
#define W25N01GV_H

#include "stm32f4xx_hal.h"

#ifdef HAL_SPI_MODULE_ENABLED	// Begin SPI include protection

// Number of pages that can be read from. See README and
// the above documentation for use when reading from flash.
#define W25N01GV_NUM_PAGES (uint32_t) 65536

/*
 * Struct to store data related to flash, including pins
 * and address counters. A pointer to a struct of this type
 * is passed to each flash function.
 */
typedef struct {
	SPI_HandleTypeDef *SPI_bus;   // SPI struct, specified by user
	GPIO_TypeDef *cs_base;        // Chip select GPIO base, specified by user
	uint16_t cs_pin;              // Chip select GPIO pin, specified by user

	uint16_t current_page;        // Tracking pages while writing
	uint16_t next_free_column;    // Tracking columns while writing

	uint16_t next_page_to_read;   // Tracking pages while reading

	// TODO add a HAL status code here
	// and have functions update that instead of returning codes
} W25N01GV_Flash;

/**
 * Value representing the status of the last read command. Error correction
 * algorithms are run internally on the flash chip, and the ECC1 and ECC0 bits
 * in the status register (SR3) display the output. These bits are read by the
 * function get_ECC_status(), which is called by read_next_2KB_from_flash().
 *
 * datasheet pg 20
 */
typedef enum {
	SUCCESS_NO_CORRECTIONS,    // ECC1 = 0, ECC0 = 0
	SUCCESS_WITH_CORRECTIONS,  // ECC1 = 0, ECC0 = 1
	ERROR_ONE_PAGE,            // ECC1 = 1, ECC0 = 0
	ERROR_MULTIPLE_PAGES       // ECC1 = 1, ECC0 = 1, only used in continuous read mode
} W25N01GV_ECC_Status;

/**
 * Initializes the flash memory chip with SPI and pin information,
 * sets parameters to an initial state, and enables the onboard
 * ECC and buffer read mode.
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param SPI_bus_in <SPI_HandleTypeDef*> Struct used for SPI communication
 * @param cs_base    <GPIO_TypeDef*>      GPIO pin array the chip select pin is on
 * @param cs_pin     <uint16_t>           GPIO pin connected to flash chip select
 */
void init_flash(W25N01GV_Flash *flash, SPI_HandleTypeDef *SPI_bus_in,
		GPIO_TypeDef *cs_base_in, uint16_t cs_pin_in);

/**
 * Check that the device's JEDEC ID matches the one listed in the datasheet.
 * Use this function to check if the flash and the SPI bus is functioning.
 *
 * Reads and checks the manufacturer ID and the device ID.
 * datasheet pg 27
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval 1 if it read the ID back correctly, 0 if it didn't
 */
uint8_t is_flash_ID_correct(W25N01GV_Flash *flash);

/**
 * Resets the flash chip to it's power-on state. If the device is busy when
 * this function is called, it will first wait for the device to finish its
 * current operation before resetting.
 *
 * Causes a typical delay of 5 microseconds and a max delay of 500 microseconds
 *
 * datasheet pg 26
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval SPI status code
 */
HAL_StatusTypeDef reset_flash(W25N01GV_Flash *flash);

/**
 * Erase the entire flash memory chip. Erasing means setting every byte to 0xFF.
 * This function also resets the address counters in the W25N01GV_Flash struct.
 *
 * WARNING: This function will erase all data, and causes a substantial delay
 * on the order of 2-10 seconds. Only use it if you're absolutely sure.
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval 0 if there's no problems, nonzero int if at least one block failed to erase
 */
uint8_t erase_flash(W25N01GV_Flash *flash);

/**
 * Writes data from an array to the W25N01GV flash memory chip.
 * It automatically tracks the address of data it writes; no address
 * management is required by the user. If it runs out of space, it
 * will stop writing data and do nothing.
 *
 * Note: It can only write data to memory locations that were previously
 * erased, so make sure to call erase_flash once before you start writing.
 *
 * TODO: test more
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param data       <uint8_t*>           Array of data to write to flash
 * @param num_bytes  <uint32_t>           Number of bytes to write to flash
 * @retval 0 if it wrote successfully, nonzero int if something went wrong
 */
uint8_t write_to_flash(W25N01GV_Flash *flash, uint8_t *data, uint32_t size);

/**
 * To be used before calling read_next_2KB_from_flash().
 *
 * Sets the page counter in the W25N01GV_Flash struct to the first page
 * for reading. Use this first, then call read_next_2KB_from_flash().
 * See README for sample code.
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 */
void reset_flash_read_pointer(W25N01GV_Flash *flash);

/**
 * Reads a 2KB page into the supplied buffer, then increments a counter so it
 * will output the next page the next time this function is called.
 *
 * To read out the entire memory array, call reset_read_pointer(), then call
 * this function up to W25N01GV_NUM_PAGES times. See README for sample code.
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param buffer     <uint8_t*>           Buffer to hold 2048 bytes of data
 * @retval SPI status code
 */
W25N01GV_ECC_Status read_next_2KB_from_flash(W25N01GV_Flash *flash,
		uint8_t *buffer);

/**
 * Returns the number of bytes remaining in the flash memory array that are
 * available to write to.
 *
 * Uses the address counters in the flash struct to calcluate how much space
 * is currently taken up, then subtracts that from the total available space.
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval Number of free bytes remaining in the flash chip to write to
 */
uint32_t get_bytes_remaining(W25N01GV_Flash *flash);

/**
 * Scan flash for bad memory blocks before writing to it for the first time.
 *
 * Reads the first byte of each block and checks for a bad block marking.
 * Out of the factory, all bytes are set to 0xFF except for the first byte
 * of each bad block. This function looks for those bytes, records the address
 * of any bad blocks found into the bad_blocks array, and returns the number of
 * bad blocks it found.
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param bad_blocks <uint16_t*>          An array of size 1024 containing the address of each bad block.
 * 	The return value tells how many of the first N indices of this array are used.
 * @retval The total number of bad blocks found
 */
uint16_t scan_bad_blocks(W25N01GV_Flash *flash, uint16_t *bad_blocks);

#endif	// end SPI include protection
#endif	// end header include protection
