/**
 * Code implementation for communicating with W25N01GV Flash Memory
 * Datasheet: https://www.winbond.com/resource-files/w25n01gv%20revl%20050918%20unsecured.pdf
 * 
 * The W25N01GV 1G-bit memory array is organized into 65,536 programmable pages
 * of 2,048-bytes each. The entire page can be programmed at one time using the
 * data from the 2,048-Byte internal buffer. Pages can be erased in groups of
 * 64 (128KB block erase). The W25N01GV has 1,024 erasable blocks.
 *
 * The W25N01GV supports the standard Serial Peripheral Interface (SPI),
 * Dual/Quad I/O SPI: Serial Clock, Chip Select, Serial Data I/O0 (DI),
 * I/O1 (DO), I/O2 (/WP), and I/O3 (/HOLD). SPI clock frequencies of up to
 * 104MHz are supported allowing equivalent clock rates of 208MHz (104MHz x 2)
 * for Dual I/O and 416MHz (104MHz x 4) for Quad I/O when using the Fast Read
 * Dual/Quad I/O instructions.
 *
 * The W25N01GV provides a new Continuous Read Mode that allows for efficient
 * access to the entire memory array with a single Read command. This feature
 * is ideal for code shadowing applications.
 *
 * Additionally, the device supports JEDEC standard manufacturer and device ID,
 * one 2,048-Byte Unique ID page, one 2,048-Byte parameter page and ten
 * 2,048-Byte OTP pages. To provide better NAND flash memory manageability,
 * user configurable internal ECC, bad block management are also available.
 *
 * ===============================================================================
 * Memory architecture described on datasheet pg 11
 * Device operational flow described on datasheet pg 12
 *
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created July 20, 2020
 * Last edited August 2, 2020
 */

//#include "W25N01GV.h"
#ifdef HAL_SPI_MODULE_ENABLED  // begin SPI include protection

const uint8_t	W25N01GV_MANUFACTURER_ID                = 0xEF;
const uint16_t W25N01GV_DEVICE_ID                     = 0xAA21;

const uint8_t W25N01GV_CS_ACTIVE                      = GPIO_PIN_RESET;  // Chip is active low
const uint8_t W25N01GV_CS_INACTIVE                    = GPIO_PIN_SET;

const uint8_t SPI_TIMEOUT                             = 0xFF;

const uint32_t TOTAL_NUM_PAGES                        = 65536;
const uint16_t PAGE_MAIN_ARRAY_NUM_BYTES              = 2048;
const uint16_t PAGE_SPARE_ARRAY_NUM_BYTES             = 64;
const uint16_t PAGE_TOTAL_NUM_BYTES                   = 2112;  // main array + spare array

const uint16_t PAGES_PER_BLOCK                        = 64;
const uint16_t TOTAL_NUM_BLOCKS                       = 1024;  // 1024 blocks with 64 pages each = 65536 pages

/* Commands */
// Summary of commands and usage on datasheet pg 23-25
const uint8_t DEVICE_RESET                            = 0xFF;
const uint8_t READ_JEDEC_ID                           = 0x9F;
const uint8_t READ_STATUS_REGISTER                    = 0x0F;  // 0x05 also works
const uint8_t WRITE_STATUS_REGISTER                   = 0x1F;  // 0x01 also works
const uint8_t WRITE_ENABLE                            = 0x06;
const uint8_t WRITE_DISABLE                           = 0x04;
const uint8_t BAD_BLOCK_MANAGEMENT                    = 0xA1;
const uint8_t READ_BBM_LOOK_UP_TABLE                  = 0xA5;
const uint8_t LAST_ECC_FAILURE_PAGE_ADDRESS           = 0xA9;
const uint8_t BLOCK_ERASE_128KB                       = 0xD8;
const uint8_t LOAD_PROGRAM_DATA                       = 0x02;
const uint8_t RANDOM_LOAD_PROGRAM_DATA                = 0x84;
const uint8_t QUAD_LOAD_PROGRAM_DATA                  = 0x32;  // not used
const uint8_t QUAD_RANDOM_LOAD_PROGRAM_DATA           = 0x34;  // not used
const uint8_t PROGRAM_EXECUTE                         = 0x10;
const uint8_t PAGE_DATA_READ                          = 0x13;
const uint8_t READ_DATA                               = 0x03;
const uint8_t FAST_READ                               = 0x0B;
const uint8_t FAST_READ_4BYTE_ADDRESS                 = 0x0C;
const uint8_t FAST_READ_DUAL_OUTPUT                   = 0x3B;  // not used
const uint8_t FAST_READ_DUAL_OUTPUT_4BYTE_ADDRESS     = 0x3C;  // not used
const uint8_t FAST_READ_QUAD_OUTPUT                   = 0x6B;  // not used
const uint8_t FAST_READ_QUAD_OUTPUT_4BYTE_ADDRESS     = 0x6C;  // not used
const uint8_t FAST_READ_DUAL_IO                       = 0xBB;  // not used
const uint8_t FAST_READ_DUAL_IO_4BYTE_ADDRESS         = 0xBC;  // not used
const uint8_t FAST_READ_QUAD_IO                       = 0xEB;  // not used
const uint8_t FAST_READ_QUAD_IO_4BYTE_ADDRESS         = 0xEC;  // not used

/* Status Register addressses */
const uint8_t SR1_PROTECTION_REGISTER_ADDRESS         = 0xA0;  // Listed as 0xAx in the datasheet
const uint8_t SR2_CONFIGURATION_REGISTER_ADDRESS      = 0xB0;  // Listed as 0xBx in the datasheet
const uint8_t SR3_STATUS_REGISTER_ADDRESS             = 0xC0;  // Listed as 0xBx in the datasheet

/* Status Register bits */
// Protection register - datasheet pg 15
const uint8_t SR1_BLOCK_PROTECT_BP3                   = 0x40;  // 0b01000000
const uint8_t SR1_BLOCK_PROTECT_BP2                   = 0x20;  // 0b00100000
const uint8_t SR1_BLOCK_PROTECT_BP1                   = 0x10;  // 0b00010000
const uint8_t SR1_BLOCK_PROTECT_BP0                   = 0x08;  // 0b00001000
const uint8_t SR1_BLOCK_PROTECT_TB                    = 0x04;  // 0b00000100
const uint8_t SR1_WRITE_PROTECT_ENABLE                = 0x02;  // 0b00000010
const uint8_t SR1_STATUS_REGISTER_PROTECT_SRP1        = 0x00;  // 0b10000000
const uint8_t SR1_STATUS_REGISTER_PROTECT_SRP0        = 0x01;  // 0b00000001

// Configuration register - datasheet pg 17
const uint8_t SR2_ONE_TIME_PROGRAM_LOCK               = 0x80;  // 0b10000000
const uint8_t SR2_ENTER_TOP_ACCESS_MODE               = 0x40;  // 0b01000000
const uint8_t SR2_STATUS_REGISTER_1_LOCK              = 0x20;  // 0b00100000
const uint8_t SR2_ECC_ENABLE                          = 0x10;  // 0b00010000
const uint8_t SR2_BUFFER_READ_MODE                    = 0x08;  // 0b00001000

// Status register - datasheet pg 19
const uint8_t SR3_BBM_LOOK_UP_TABLE_FULL              = 0x40;  // 0b01000000
const uint8_t SR3_ECC_STATUS_SUCCESS                  = 0x00;  // 0b00000000
const uint8_t SR3_ECC_STATUS_SUCCESS_WITH_CORRECTIONS = 0x10;  // 0b00010000
const uint8_t SR3_ECC_STATUS_ERROR_ONE_PAGE           = 0x20;  // 0b00100000
const uint8_t SR3_ECC_STATUS_ERROR_MULTIPLE_PAGES     = 0x30;  // 0b00110000
const uint8_t SR3_PROGRAM_FAILURE                     = 0x08;  // 0b00001000
const uint8_t SR3_ERASE_FAILURE                       = 0x04;  // 0b00000100
const uint8_t SR3_WRITE_ENABLE_LATCH                  = 0x02;  // 0b00000010
const uint8_t SR3_OPERATION_IN_PROGRESS               = 0x01;  // 0b00000001

/* Unique ID / Parameter / OPT Page Addresses */
//datasheet pg 52-53
const uint8_t UNIQUE_ID_PAGE_ADDRESS                  = 0x00;  // not used
const uint8_t PARAMETER_PAGE_ADDRESS                  = 0x01;  // not used
const uint8_t OTP_PAGE_0                              = 0x02;  // not used
// ^^ Add i to get OTP page i, 0 <= i <= 9

/* Private functions */

/**
 * Converts a uint16_t number into an array literal of 2 uint8_t numbers.
 *
 * @param num <uint16_t> A 16 bit number to be converted to an array of 2 8 bit numbers.
 * @retval An array literal of 2 uint8_t numbers, with the first entry
 * 	representing the first 8 bits	of the input, and the second entry
 * 	representing the last 8 bits of the input.
 */
#define UNPACK_UINT16_TO_2_BYTES(num)		{(uint8_t) (((num) & 0xFF00) >> 8), (uint8_t) ((num) & 0x00FF)}

/**
 * Converts an array of 2 uint8_t numbers into 1 uint16_t number.
 *
 * @param bytes <uint8_t*> An array of at least 2 uint8_t's
 * @retval A uint16_t formed by combining the 2 uint8_t's in the input array,
 * 	with the first 8 bit representing the first array entry, and the last 8
 * 	bits representing the second array entry.
 */
#define PACK_2_BYTES_TO_UINT16(bytes)		((((uint16_t) *(bytes)) << 8) + *((bytes)+1))

/**
 * Sets the device's chip select pin active.
 */
void chip_select(W25N01GV_Flash *flash) {
	HAL_GPIO_WritePin(flash->cs_base, flash->cs_pin, W25N01GV_CS_ACTIVE);
}

/**
 * Sets the device's chip select pin inactive.
 */
void chip_release(W25N01GV_Flash *flash) {
	HAL_GPIO_WritePin(flash->cs_base, flash->cs_pin, W25N01GV_CS_INACTIVE);
}

/**
 * Transmit 1 or more bytes to the device via SPI.
 *
 * @param tx <uint8_t*> Data buffer to transmit
 * @param size <uint16_t> Number of bytes to transmit
 */
void spi_transmit(W25N01GV_Flash *flash, uint8_t *tx, uint16_t size) {
	__disable_irq();
	chip_select(flash);
	HAL_SPI_Transmit(flash->SPI_bus, tx, size, SPI_TIMEOUT);
	chip_release(flash);
	__enable_irq();
}

/**
 * Receive 1 or more bytes from the device via SPI.
 * I don't think this has any practical use at all.
 * I don't know why I made it lol
 *
 * @param rx <uint8_t*> Data buffer for receiving
 * @param size <uint16_t> Number of bytes to receive
 */
void spi_receive(W25N01GV_Flash *flash, uint8_t *rx, uint16_t size) {
	__disable_irq();
	chip_select(flash);
	HAL_SPI_Receive(flash->SPI_bus, rx, size, SPI_TIMEOUT);
	chip_release(flash);
	__enable_irq();
}

/**
 * Transmit 1 or more bytes and receive 1 or more bytes to/from the device via SPI
 *
 * @param tx <uint8_t*> Data buffer to transmit
 * @param tx_size <uint16_t> Number of bytes to transmit
 * @param rx <uint8_t*> Buffer to receive data
 * @param rx_size <uint16_t> Number of bytes to receive
 */
void spi_transmit_receive(W25N01GV_Flash *flash, uint8_t *tx, uint16_t tx_size,
		uint8_t *rx, uint16_t rx_size) {
	__disable_irq();
	chip_select(flash);
	HAL_SPI_Transmit(flash->SPI_bus, tx, tx_size, SPI_TIMEOUT);
	HAL_SPI_Receive(flash->SPI_bus, rx, rx_size, SPI_TIMEOUT);
	chip_release(flash);
	__enable_irq();
}

/**
 * Sends a 1 byte command over SPI to the flash device.
 * Receives nothing.
 *
 * @param cmd <uint8_t> 1 byte command to be sent over SPI
 */
void send_cmd(W25N01GV_Flash *flash, uint8_t cmd) {
	uint8_t tx[1] = {cmd};
	spi_transmit(flash, tx, 1);
}

/**
 * The Read Status Register instruction may be used at any time, even while
 * a Program or Erase cycle is in progress.
 *
 * The status registers available to read are:
 * Protection Register (SR1)				Address: SR1_PROTECTION_REGISTER_ADDRESS (0xA0)
 * Configuration Register (SR2)			Address: SR2_CONFIGURATION_REGISTER_ADDRESS (0xB0)
 * Status Register (SR3)						Address: SR3_STATUS_REGISTER_ADDRESS (0xC0)
 *
 * @param register_adr <uint8_t> The address of the status register to be read
 * @retval The contents of the 8 bit status register specified by the user
 *
 * datasheet pg 28
 */
uint8_t read_status_register(W25N01GV_Flash *flash, uint8_t register_adr) {
	uint8_t tx[2] = {READ_STATUS_REGISTER, register_adr};
	uint8_t rx[1];

	spi_transmit_receive(flash, tx, 2, rx, 1);

	return *rx;
}

/**
 * Checks to see if flash is busy. While it's busy, all commands will be
 * ignored except for Read Status Register and Read JEDEC ID.
 *
 * @retval 1 if the device is busy, 0 if it's not.
 */
uint8_t flash_is_busy(W25N01GV_Flash *flash) {
	uint8_t status_register = read_status_register(flash, SR3_STATUS_REGISTER_ADDRESS);
	return status_register & SR3_OPERATION_IN_PROGRESS;
}

/**
 * Writes 1 byte to the selected status register. Takes at least 50 ns to complete.
 *
 * The status registers available to write are:
 * Protection Register (SR1)				Address: SR1_PROTECTION_REGISTER_ADDRESS (0xA0)
 * Configuration Register (SR2)			Address: SR2_CONFIGURATION_REGISTER_ADDRESS (0xB0)
 *
 * @param register_adr <uint8_t> The address of the status register to be written to
 * @param register_write_val <uint8_t> The 8bit value to write to the status register
 */
void write_status_register(W25N01GV_Flash *flash, uint8_t register_adr,
		uint8_t register_write_val) {
	uint8_t tx[3] = {WRITE_STATUS_REGISTER, register_adr, register_write_val};

	spi_transmit(flash, tx, 3);

	while (flash_is_busy(flash));	//wait for writing to finish
}


/**
 * Checks to see if all 20 entries in the Bad Block Management look up table
 * are full.
 *
 * @retval 1 if the LUT is full, 0 if not
 */
uint8_t BBM_look_up_table_is_full(W25N01GV_Flash *flash) {
	uint8_t status_register = read_status_register(flash, SR3_STATUS_REGISTER_ADDRESS);
	return status_register & SR3_BBM_LOOK_UP_TABLE_FULL;
}

/**
 * Read the bad block management look up table. The manufacturer marks some of
 * the bad memory blocks and writes them into a look up table. Honestly idk how
 * it works or what it does exactly, but the datasheet said to scan all blocks
 * before writing or erasing so you don't delete this information.
 *
 * @param logical_block_addresses <uint16_t*> Pointer to array of 20 uint16_t's to store the table's LBAs
 * @param physical_block_addresses <uint16_t*> Pointer to array of 20 uint16_t's to store the table's PBAs
 */
void read_BBM_look_up_table(W25N01GV_Flash *flash, uint16_t *logical_block_addresses, uint16_t *physical_block_addresses) {
	uint8_t tx[2] = {READ_BBM_LOOK_UP_TABLE, 0};	// 2nd byte is unused
	uint8_t rx[80];

	spi_transmit_receive(flash, tx, 2, rx, 80);

	// format the received bytes into the user-supplied arrays
	for (int i = 0; i < 20; i++) {
		logical_block_addresses[i] = PACK_2_BYTES_TO_UINT16(rx+(2*i));
		physical_block_addresses[i] = PACK_2_BYTES_TO_UINT16(rx+(2*i)+1);
	}
}

/**
 * Loads a page specified by the user into the device's buffer.
 * Load process takes 25 microseconds if ECC is disabled, and 60 microseconds if enabled.
 * The device will be in a BUSY state and ignore most commands until loading finished.
 *
 * datasheet pg 38
 *
 * @param page_num <uint16_t> Page number of data to load to the device's buffer
 */
void load_page(W25N01GV_Flash *flash, uint16_t page_num) {
	uint8_t page_num_8bit_array[2] = UNPACK_UINT16_TO_2_BYTES(page_num);	// split the page number into 2 bytes

	uint8_t tx[4] = {PAGE_DATA_READ, 0, page_num_8bit_array[0], page_num_8bit_array[1]};	// 2nd byte is unused

	spi_transmit(flash, tx, 4);

	flash->last_page_loaded = page_num;
  while(flash_is_busy(flash));      // wait for the page to load
}

/**
 * Unfinished, this function probably won't be used;
 * Read 1 page, from the column_adr input byte to the end,
 * including the extra 64 byte memory array.
 *
 * @param column_adr <uint16_t> The column number of page to start reading from (byte number).
 * 	Maximum of 2111.
 */
void read_page_buffer_mode(W25N01GV_Flash *flash, uint16_t column_adr) {
	uint8_t column_adr_8bit_array[2] = UNPACK_UINT16_TO_2_BYTES(column_adr);

	uint8_t tx[4] = {READ_DATA, column_adr_8bit_array[0], column_adr_8bit_array[1], 0};	// last byte is unused
	uint8_t rx[PAGE_TOTAL_NUM_BYTES - column_adr];	// reads from column_adr to end of page, including extra memory array

	spi_transmit_receive(flash, tx, 4, rx, PAGE_TOTAL_NUM_BYTES - column_adr);

	while(flash_is_busy(flash));      // wait for the page to load

	// figure out what to do with rx lmao
}

/**
 * Unfinished, this function probably won't be used;
 * Reads the memory array starting from column 0 of the buffer,
 * then immediately starts reading from the next page, continuing until it
 * reaches the end of the last page.
 *
 * WARNING: size of rx buffer can be up to 128MB, STM32 flash only stores 512KB
 * so basically, don't use this function
 *
 * Only reads the 2048-byte main array of each page, not the extra array.
 * Requires chunky data buffer.
 */
void read_page_continuous_mode(W25N01GV_Flash *flash) {
	uint8_t tx[4] = {READ_DATA, 0, 0, 0};	// '0' bytes are unused
	uint8_t rx[(TOTAL_NUM_PAGES - flash->last_page_loaded) * PAGE_MAIN_ARRAY_NUM_BYTES];

	spi_transmit_receive(flash, tx, 4, rx, (TOTAL_NUM_PAGES - flash->last_page_loaded) * PAGE_MAIN_ARRAY_NUM_BYTES);

	// figure out what to do with rx and the long ass transmission time
}

/**
 * Enables writing to flash by setting the Write Enable Latch (WEL) bit in the
 * status register to 1.
 *
 * Must be set prior to every Page Program, Quad Page Program, Block Erase
 * and Bad Block Management instruction.
 */
void enable_write(W25N01GV_Flash *flash) {
	send_cmd(flash, WRITE_ENABLE);
}

/**
 * Disables writing to flash by setting the write enable bit (WEL) bit in the
 * status register to 0.
 *
 * The WEL bit is automatically reset after Power-up and upon completion of
 * the Page Program, Quad Page Program, Block Erase, Reset and Bad Block
 * Management instructions.
 */
void disable_write(W25N01GV_Flash *flash) {
	send_cmd(flash, WRITE_DISABLE);
}

/**
 * Writes data to the device's buffer in preparation for writing
 * it to memory. It writes the data at the specified column (byte), and writes
 * data up to column 2047 or the end of the user supplied data array.
 *
 * The _no_overwrite means that it ignores all other bytes not touched
 * by the user instead of resetting them to 0xFF.
 *
 * @param data <uint8_t*> Data array containing data to write to flash
 * @param num_bytes <uint16_t> Number of bytes to write
 * @param column_adr <uint16_t> Byte in buffer to start writing at
 */
void write_page_to_buffer_no_overwrite(W25N01GV_Flash *flash, uint8_t *data, uint16_t num_bytes, uint16_t column_adr) {
	uint8_t column_adr_8bit_array[2] = UNPACK_UINT16_TO_2_BYTES(column_adr);
	uint8_t tx1[3] = {RANDOM_LOAD_PROGRAM_DATA, column_adr_8bit_array[0], column_adr_8bit_array[1]};

	// if error correction is on this happens automatically, but just in case
	// ignore all data after 2048 bytes - don't overwrite the extra array at the end of the page
	if (num_bytes > PAGE_MAIN_ARRAY_NUM_BYTES)
		num_bytes = PAGE_MAIN_ARRAY_NUM_BYTES;

	// not using spi_transmit() because I didn't want to fuck with combining arrays
	__disable_irq();
	chip_select(flash);
	HAL_SPI_Transmit(flash->SPI_bus, tx1, 3, SPI_TIMEOUT);
	HAL_SPI_Transmit(flash->SPI_bus, data, num_bytes, SPI_TIMEOUT);
	chip_release(flash);
	__enable_irq();
}

/**
 * Writes data to the device's buffer in preparation for writing
 * it to memory. It writes the data at the specified column (byte), and writes
 * data up to column 2047 or the end of the user supplied data array.
 *
 * The _overwrite means that it sets all other bytes in the buffer that are
 * not touched by the user to 0xFF.
 *
 * @param data <uint8_t*> Data array containing data to write to flash
 * @param num_bytes <uint16_t> Number of bytes to write
 * @param column_adr <uint16_t> Byte in buffer to start writing at
 */
void write_page_to_buffer_overwrite(W25N01GV_Flash *flash, uint8_t *data, uint16_t num_bytes, uint16_t column_adr) {
	uint8_t column_adr_8bit_array[2] = UNPACK_UINT16_TO_2_BYTES(column_adr);
	uint8_t tx1[3] = {LOAD_PROGRAM_DATA, column_adr_8bit_array[0], column_adr_8bit_array[1]};

	// if error correction is on this happens automatically, but just in case
	// ignore all data after 2048 bytes - don't overwrite the extra array at the end of the page
	if (num_bytes > PAGE_MAIN_ARRAY_NUM_BYTES)
		num_bytes = PAGE_MAIN_ARRAY_NUM_BYTES;

	// not using spi_transmit() because I didn't want to fuck with combining arrays
	__disable_irq();
	chip_select(flash);
	HAL_SPI_Transmit(flash->SPI_bus, tx1, 3, SPI_TIMEOUT);
	HAL_SPI_Transmit(flash->SPI_bus, data, num_bytes, SPI_TIMEOUT);
	chip_release(flash);
	__enable_irq();
}

/**
 * Run the program execute command to store the data in the device's buffer into
 * memory at the specified page. This should be run after running
 * write_page_to_buffer_overwrite() or write_page_to_buffer_no_overwrite().
 *
 * @param page_adr <uint16_t> The page for the buffer to be written to.
 */
void program_buffer_to_memory(W25N01GV_Flash *flash, uint16_t page_adr) {
	uint8_t page_adr_8bit_array[2] = UNPACK_UINT16_TO_2_BYTES(page_adr);
	uint8_t tx[4] = {PROGRAM_EXECUTE, 0, page_adr_8bit_array[0], page_adr_8bit_array[1]}; // 2nd byte unused

	spi_transmit(flash, tx, 4);
	while (flash_is_busy(flash));	// wait for the data to be written to memory
}

/**
 * Erases all data in the block containing the specified page address.
 * Each block has 64 pages, for a total of 128KB. Data is erased by setting
 * each byte to 0xFF.
 *
 * The Write Enable Latch bit is first set high, and is automatically set low
 * after the erase process finishes.
 *
 * If the block containing the specified page address is protected by the
 * Block Protect bits in the protection register, then the erase command
 * will not execute.
 *
 * It takes up to 10 milliseconds to complete the process, but typically takes
 * 2 milliseconds (datasheet pg 59). If the command executes successfully,
 * the device will enter a BUSY state until it finishes.
 *
 * TODO: test
 *
 * @param page_adr <uint16_t> Address of the page whose block should be erased
 */
void erase_block(W25N01GV_Flash *flash, uint16_t page_adr) {
	uint8_t page_adr_8bit_array[2] = UNPACK_UINT16_TO_2_BYTES(page_adr);
	enable_write(flash);	// Set WEL bit high, it will automatically be set back to 0 after the command executes

	uint8_t tx[4] = {BLOCK_ERASE_128KB, 0, page_adr_8bit_array[0], page_adr_8bit_array[1]};	// 2nd byte unused
	spi_transmit(flash, tx, 4);

	disable_write(flash);	// in case the command doesn't execute, if the block is protected
}

/**
 * Check that the device's JEDEC ID matches the one listed in the datasheet.
 * Good way to check if the device is alive.
 *
 * Reads and checks the manufacturer ID and the device ID.
 * datasheet pg 27
 *
 * @retval 1 if the ID is correct, 0 if it's not
 */
uint8_t flash_ID_is_correct(W25N01GV_Flash *flash) {

	uint8_t tx[2] = {READ_JEDEC_ID, 0};	// second byte is unused
	uint8_t rx[3];

	spi_transmit_receive(flash, tx, 2, rx, 3);
	uint8_t manufacturer_ID = rx[0];
	uint16_t device_ID = PACK_2_BYTES_TO_UINT16(rx+1);

	if (manufacturer_ID == W25N01GV_MANUFACTURER_ID && device_ID == W25N01GV_DEVICE_ID)
		return 1;
	else
		return 0;
}

/**
 * First checks the status register to see if the device is busy.
 * If it's busy, the function does nothing. If it's not busy, it sends the
 * reset command.
 *
 * @retval 1 if the device was not busy and the reset command was sent,
 * 	0 if the device was busy and the reset command was not sent.
 *
 * datasheet pg 26
 */
uint8_t reset_flash(W25N01GV_Flash *flash) {
	if (!flash_is_busy(flash)) {
		send_cmd(flash, DEVICE_RESET);
		while(flash_is_busy(flash));	//wait for it to reset
		return 1;
	}
	else {
		return 0;
	}
}

/**
 * Set the ECC-E bit in the configuration register (SR2) to 1, enabling the
 * onboard error correction algorithms. If ECC-E is already 1, does nothing.
 */
void enable_ECC(W25N01GV_Flash *flash) {
	uint8_t configuration_register = read_status_register(flash, SR2_CONFIGURATION_REGISTER_ADDRESS);
	uint8_t ECC_enabled_register = configuration_register | SR2_ECC_ENABLE;	//or: add bit
	if (ECC_enabled_register != configuration_register)
		write_status_register(flash, SR2_CONFIGURATION_REGISTER_ADDRESS, ECC_enabled_register);
}

/**
 * Set the ECC-E bit in the configuration register (SR2) to 0, disabling the
 * onboard error correction algorithms. If ECC-E is already 0, does nothing.

 * Don't actually use this, but I included it because why not.
 */
void disable_ECC(W25N01GV_Flash *flash) {
	uint8_t configuration_register = read_status_register(flash, SR2_CONFIGURATION_REGISTER_ADDRESS);
	uint8_t ECC_disabled_register = configuration_register & ~SR2_ECC_ENABLE;	//remove bit
	if (ECC_disabled_register != configuration_register)
		write_status_register(flash, SR2_CONFIGURATION_REGISTER_ADDRESS, ECC_disabled_register);
}

/**
 * Sets the device to buffer read mode, which limits the user to reading
 * up to one page at a time before loading a new page. It sets the BUF bit
 * in the configuration register to 1 if BUF=0, and does nothing if BUF=1.
 */
void enable_buffer_mode(W25N01GV_Flash *flash) {
	uint8_t configuration_register = read_status_register(flash, SR2_CONFIGURATION_REGISTER_ADDRESS);
	uint8_t buffer_enabled_register = configuration_register | SR2_BUFFER_READ_MODE;	//or: add the bit on
	if (buffer_enabled_register != configuration_register)
		write_status_register(flash, SR2_CONFIGURATION_REGISTER_ADDRESS, buffer_enabled_register);
}

/**
 * Sets the device to continuous read mode, which automatically loads the next
 * page while reading out the current one, so data is continuously read out
 * until the chip select pin goes high or it reaches the end of memory.
 * It sets the BUF bit in the configuration register to 0 if BUF=1, and does
 * nothing if BUF=0.
 *
 * Don't actually use this, but I included it because why not.
 */
void enable_continuous_mode(W25N01GV_Flash *flash) {
	uint8_t configuration_register = read_status_register(flash, SR2_CONFIGURATION_REGISTER_ADDRESS);
	uint8_t buffer_enabled_register = configuration_register & ~SR2_BUFFER_READ_MODE;	//remove the bit
	if (buffer_enabled_register != configuration_register)
		write_status_register(flash, SR2_CONFIGURATION_REGISTER_ADDRESS, buffer_enabled_register);
}

/* Public functions */

/**
 * Initializes the flash memory chip with SPI and pin information, and sets
 * some parameters to an initial state.
 */
void init_flash(W25N01GV_Flash *flash, SPI_HandleTypeDef *SPI_bus_in,
		GPIO_TypeDef *cs_base_in,	uint16_t cs_pin_in) {
	flash->SPI_bus = SPI_bus_in;
	flash->cs_base = cs_base_in;
	flash->cs_pin = cs_pin_in;
	flash->last_page_loaded = 0;	// device loads page 0 on power up
	enable_ECC(flash);  // should be enabled by default, but just in case
	enable_buffer_mode(flash);  // -IG models start with buffer mode by default, -IG models don't
}

/**
 * Unlocks write capabilities on the device. All memory arrays are locked to
 * read only when the device is powered on to protect data. This function
 * unlocks memory by changing the block protect bits in the Protection register
 * to all 0's. It does not change the other bits in the register.
 *
 *
 * datasheet pg 15, 21
 */
void unlock_flash(W25N01GV_Flash *flash) {
	// Read the current contents of the protection register
	uint8_t protection_register = read_status_register(flash, SR1_PROTECTION_REGISTER_ADDRESS);

	// Remove the block protect bits - same as (protection_register ^ 01111100)
	// Only keep the 3 non-protect bits, if they're already enabled.
	uint8_t unlocked_protection_register = protection_register & ~(SR1_BLOCK_PROTECT_BP3
			| SR1_BLOCK_PROTECT_BP2
			| SR1_BLOCK_PROTECT_BP1
			| SR1_BLOCK_PROTECT_BP0
			| SR1_BLOCK_PROTECT_TB);	//remove the block protect bits

	write_status_register(flash, SR1_PROTECTION_REGISTER_ADDRESS, unlocked_protection_register);
}

/**
 * Locks write capabilities on the device. This function unlocks memory by
 * changing the block protect bits in the Protection register to a locked
 * configuration. It does not change the other bits in the register.
 *
 * datasheet pg 15, 21
 */
void lock_flash(W25N01GV_Flash *flash) {
	// Read the current contents of the protection register
	uint8_t protection_register = read_status_register(flash, SR1_PROTECTION_REGISTER_ADDRESS);

	// Enabling bits BP3 and BP2 will lock the entire 128MB memory array
	uint8_t locked_protection_register = protection_register
			| (SR1_BLOCK_PROTECT_BP3 | SR1_BLOCK_PROTECT_BP2);

	write_status_register(flash, SR1_PROTECTION_REGISTER_ADDRESS, locked_protection_register);
}

/**
 * Unfinished; this will be the main function users use to write data
 * it keeps track of the last address written to and continues writing
 * from there. It also writes to multiple pages if the buffer is too large
 * for one page.
 *
 * TODO: write the damn thing, implement address tracking in the flash struct
 */
void write_to_flash(W25N01GV_Flash *flash, uint8_t *data, uint16_t num_bytes) {

}

/**
 * Private function for testing; writes the contents of data into flash
 * at the specified page and column. It writes to the device's buffer,
 * then programs the buffer data into flash memory.
 *
 * @param data <uint8_t*> Array of data to write to flash
 * @param num_bytes <uint16_t> Number of bytes to write to flash
 * @param page_adr <uint16_t> Page to write data to
 * @param column_adr <uint16_t> Column of page to start writing data at
 */
void write_bytes(W25N01GV_Flash *flash, uint8_t *data, uint16_t num_bytes,
		uint16_t page_adr, uint16_t column_adr) {
	enable_write(flash);
	write_page_to_buffer_no_overwrite(flash, data, num_bytes, column_adr);
	program_buffer_to_memory(flash, page_adr);
	disable_write(flash);	//just in case ;)
}

/**
 * Private function for testing; Reads the contents of the flash's
 * buffer into an array, starting at the specified column and going until
 * it reaches the end of the buffer or reads in buffer_size number of bytes
 *
 * @param buffer <uint8_t*> Array to read the contents of the device's buffer into
 * @param num_bytes <uint16_t> Number of bytes to read into buffer
 * @param column_adr <uint16_t> Starting column address of the data to be read in
 */
void read_flash_buffer(W25N01GV_Flash *flash, uint8_t *buffer,
		uint16_t num_bytes, uint16_t column_adr) {
	uint8_t column_adr_8bit_array[2] = UNPACK_UINT16_TO_2_BYTES(column_adr);
	uint8_t tx[4] = {READ_DATA, column_adr_8bit_array[0], column_adr_8bit_array[1], 0};  // last byte is unused

	spi_transmit_receive(flash, tx, 4, buffer, num_bytes);

	while(flash_is_busy(flash));      // wait for the page to load

}

/**
 * Private function for testing, reads the specified number of bytes from the
 * specified page and column into an array.
 *
 * It should be used in buffer mode.
 * TODO: maybe enable buffer mode at the start? or not worth it?
 *
 * @param buffer <uint8_t*> Data buffer to read data into
 * @param num_bytes <uint16_t> Number of bytes to read in
 * 	Note: if num_bytes > (2112 - column_adr), then it will only
 * 	read in (2112 - column_adr) bytes
 * @param page_adr <uint16_t> The page to read data from
 * @param column_adr <uint16_t> The column to start reading data from
 */
void read_bytes(W25N01GV_Flash *flash, uint8_t *buffer, uint16_t num_bytes,
		uint16_t page_adr, uint16_t column_adr) {
	load_page(flash, page_adr);
	read_flash_buffer(flash, buffer, num_bytes, column_adr);
}

/**
 * Unfinished; read an entire page into an array and increment *page_adr
 * TODO: get rid of page_adr, that should be tracked by the flash struct
 */
void read_2KB_and_advance(W25N01GV_Flash *flash, uint8_t *buffer,
		uint16_t buffer_size, uint16_t *page_adr) {
	load_page(flash, *page_adr);


	(*page_adr)++;
}

/**
 * Unfinished; erase the entire chip after getting confirmation.
 * Erasing means setting every byte to 0xFF.
 *
 * TODO: test block erase, then write this function
 */
void erase_chip(W25N01GV_Flash *flash) {
	// Loop through every block to erase them one by one
	for (uint16_t block_count = 0; block_count < TOTAL_NUM_BLOCKS; block_count++) {

	}
}

#endif	// end SPI include protection
