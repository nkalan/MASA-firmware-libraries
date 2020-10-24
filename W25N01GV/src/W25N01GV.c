/**
 * Implementation MASA W25N01GV Flash Memory firmware library
 * Datasheet: https://www.winbond.com/resource-files/w25n01gv%20revl%20050918%20unsecured.pdf
 * 
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created July 20, 2020
 * Last edited October 24, 2020
 *
 * ===============================================================================
 * Memory architecture described on datasheet pg 11
 * Device operational flow described on datasheet pg 12
 *
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created July 20, 2020
 * Last edited October 24, 2020
 */

#include "W25N01GV.h"
#ifdef HAL_SPI_MODULE_ENABLED  // Begin SPI include protection

// Device ID information, used to check if flash is working
#define	W25N01GV_MANUFACTURER_ID                  (uint8_t)  0xEF
#define W25N01GV_DEVICE_ID                        (uint16_t) 0xAA21

// Chip is active low
#define W25N01GV_CS_ACTIVE                        (uint8_t)  GPIO_PIN_RESET  // Chip is active low
#define W25N01GV_CS_INACTIVE                      (uint8_t)  GPIO_PIN_SET

// Arbitrary timeout value
#define W25N01GV_SPI_TIMEOUT                      (uint8_t)  0xFF

// Each page has a 2048-byte main data array to read/write
#define W25N01GV_PAGE_MAIN_NUM_BYTES              (uint16_t) 2048

// 1024 blocks with 64 pages each = 65536 pages
#define W25N01GV_PAGES_PER_BLOCK                  (uint16_t) 64
#define W25N01GV_NUM_BLOCKS                       (uint16_t) 1024

/* Commands */
// Summary of commands and usage on datasheet pg 23-25
#define W25N01GV_DEVICE_RESET                     (uint8_t) 0xFF
#define W25N01GV_READ_JEDEC_ID                    (uint8_t) 0x9F
#define W25N01GV_READ_STATUS_REGISTER             (uint8_t) 0x0F  // 0x05 also works
#define W25N01GV_WRITE_STATUS_REGISTER            (uint8_t) 0x1F  // 0x01 also works
#define W25N01GV_WRITE_ENABLE                     (uint8_t) 0x06
#define W25N01GV_WRITE_DISABLE                    (uint8_t) 0x04
#define W25N01GV_READ_BBM_LOOK_UP_TABLE           (uint8_t) 0xA5
#define W25N01GV_ERASE_BLOCK                      (uint8_t) 0xD8
#define W25N01GV_LOAD_PROGRAM_DATA                (uint8_t) 0x02
#define W25N01GV_PROGRAM_EXECUTE                  (uint8_t) 0x10
#define W25N01GV_PAGE_DATA_READ                   (uint8_t) 0x13
#define W25N01GV_READ_DATA                        (uint8_t) 0x03

/* Status Register addressses */
#define W25N01GV_SR1_PROTECTION_REG_ADR           (uint8_t) 0xA0  // Listed as 0xAx in the datasheet
#define W25N01GV_SR2_CONFIG_REG_ADR               (uint8_t) 0xB0  // Listed as 0xBx in the datasheet
#define W25N01GV_SR3_STATUS_REG_ADR               (uint8_t) 0xC0  // Listed as 0xBx in the datasheet

/* Status Register bits */
// Protection register - datasheet pg 15
#define W25N01GV_SR1_BLOCK_PROTECT_BP3            (uint8_t) 0x40  // 0b01000000
#define W25N01GV_SR1_BLOCK_PROTECT_BP2            (uint8_t) 0x20  // 0b00100000
#define W25N01GV_SR1_BLOCK_PROTECT_BP1            (uint8_t) 0x10  // 0b00010000
#define W25N01GV_SR1_BLOCK_PROTECT_BP0            (uint8_t) 0x08  // 0b00001000
#define W25N01GV_SR1_BLOCK_PROTECT_TB             (uint8_t) 0x04  // 0b00000100
#define W25N01GV_SR1_WRITE_PROTECT_ENABLE         (uint8_t) 0x02  // 0b00000010
#define W25N01GV_SR1_STATUS_REGISTER_PROTECT_SRP1 (uint8_t) 0x00  // 0b10000000
#define W25N01GV_SR1_STATUS_REGISTER_PROTECT_SRP0 (uint8_t) 0x01  // 0b00000001

// Configuration register - datasheet pg 17
#define W25N01GV_SR2_ONE_TIME_PROGRAM_LOCK        (uint8_t) 0x80  // 0b10000000
#define W25N01GV_SR2_ENTER_TOP_ACCESS_MODE        (uint8_t) 0x40  // 0b01000000
#define W25N01GV_SR2_STATUS_REGISTER_1_LOCK       (uint8_t) 0x20  // 0b00100000
#define W25N01GV_SR2_ECC_ENABLE                   (uint8_t) 0x10  // 0b00010000
#define W25N01GV_SR2_BUFFER_READ_MODE             (uint8_t) 0x08  // 0b00001000

// Status register - datasheet pg 19
#define W25N01GV_SR3_BBM_LOOK_UP_TABLE_FULL       (uint8_t) 0x40  // 0b01000000
#define W25N01GV_SR3_ECC_STATUS_BIT_1             (uint8_t) 0x20  // 0b00100000
#define W25N01GV_SR3_ECC_STATUS_BIT_0             (uint8_t) 0x10  // 0b00010000
#define W25N01GV_SR3_PROGRAM_FAILURE              (uint8_t) 0x08  // 0b00001000
#define W25N01GV_SR3_ERASE_FAILURE                (uint8_t) 0x04  // 0b00000100
#define W25N01GV_SR3_WRITE_ENABLE_LATCH           (uint8_t) 0x02  // 0b00000010
#define W25N01GV_SR3_OPERATION_IN_PROGRESS        (uint8_t) 0x01  // 0b00000001


/* Private functions */

/**
 * Converts a uint16_t number into an array literal of two uint8_t numbers.
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param num        <uint16_t>           A 16 bit number to be converted to an array of two 8 bit numbers.
 * @retval An array literal of 2 uint8_t numbers, with the first entry
 * 	representing the first 8 bits	of the input, and the second entry
 * 	representing the last 8 bits of the input.
 */
#define W25N01GV_UNPACK_UINT16_TO_2_BYTES(num)		{(uint8_t) (((num) & 0xFF00) >> 8), (uint8_t) ((num) & 0x00FF)}

/**
 * Converts an array of 2 uint8_t numbers into 1 uint16_t number.
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param bytes      <uint8_t*>           A pointer to an array of two uint8_t numbers
 * @retval A uint16_t equal to (2^8)*B1 + B0, where bytes = {B1, B0}, which is
 *   equivalent to concatenating B1B0.
 */
#define W25N01GV_PACK_2_BYTES_TO_UINT16(bytes)		(uint16_t) ((((uint16_t) *(bytes)) << 8) + *((bytes)+1))

/**
 * Transmit 1 or more bytes to the device via SPI. This function exists to
 * shorten the number of commands required to write something over SPI
 * to 2 lines of code: define tx, then call this function.
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param tx         <uint8_t*>           Data buffer to transmit
 * @param size       <uint16_t>           Number of bytes to transmit
 * @retval The SPI status code
 */
static HAL_StatusTypeDef spi_transmit(W25N01GV_Flash *flash, uint8_t *tx, uint16_t size) {
	HAL_StatusTypeDef tx_status;

	__disable_irq();
	HAL_GPIO_WritePin(flash->cs_base, flash->cs_pin, W25N01GV_CS_ACTIVE);  // Select chip
	// Transmit data and store the status code
	tx_status = HAL_SPI_Transmit(flash->SPI_bus, tx, size, W25N01GV_SPI_TIMEOUT);
	HAL_GPIO_WritePin(flash->cs_base, flash->cs_pin, W25N01GV_CS_INACTIVE);  // Release chip
	__enable_irq();

	return tx_status;
}

/**
 * Transmit 1 or more bytes and receive 1 or more bytes to/from the device via SPI.
 * This function exists to shorten the number of commands required to write
 * and read something over SPI to 3 lines of code: define tx array, declare rx, and then
 * call this function.
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param tx         <uint8_t*>           Data buffer to transmit
 * @param tx_size    <uint16_t>           Number of bytes to transmit
 * @param rx         <uint8_t*>           Buffer to receive data
 * @param rx_size    <uint16_t>           Number of bytes to receive
 * @retval The SPI status code
 */
static HAL_StatusTypeDef spi_transmit_receive(W25N01GV_Flash *flash, uint8_t *tx,
		uint16_t tx_size,	uint8_t *rx, uint16_t rx_size) {
	HAL_StatusTypeDef tx_status, rx_status;

	__disable_irq();
	HAL_GPIO_WritePin(flash->cs_base, flash->cs_pin, W25N01GV_CS_ACTIVE);  // select chip
	// Transmit/receive, and store the status code
	tx_status = HAL_SPI_Transmit(flash->SPI_bus, tx, tx_size, W25N01GV_SPI_TIMEOUT);
	rx_status = HAL_SPI_Receive(flash->SPI_bus, rx, rx_size, W25N01GV_SPI_TIMEOUT);
	HAL_GPIO_WritePin(flash->cs_base, flash->cs_pin, W25N01GV_CS_INACTIVE);  // release chip
	__enable_irq();

	return tx_status | rx_status;  // Return both status codes
}

/**
 * The Read Status Register instruction may be used at any time, even while
 * a Program or Erase cycle is in progress.
 *
 * The status registers available to read are:
 * Protection Register (SR1)				Address: W25N01GV_SR1_PROTECTION_REG_ADR (0xA0)
 * Configuration Register (SR2)			Address: W25N01GV_SR2_CONFIG_REG_ADR (0xB0)
 * Status Register (SR3)						Address: W25N01GV_SR3_STATUS_REG_ADR (0xC0)
 *
 * datasheet pg 28
 *
 * @param flash        <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param register_adr <uint8_t>            The address of the status register to be read
 * @retval The contents of the 8 bit status register specified by the user
 */
static uint8_t read_status_register(W25N01GV_Flash *flash, uint8_t register_adr) {
	uint8_t tx[2] = {W25N01GV_READ_STATUS_REGISTER, register_adr};
	uint8_t rx[1];

	spi_transmit_receive(flash, tx, 2, rx, 1);

	return *rx;
}

/**
 * Checks to see if flash is busy. While it's busy, all commands will be
 * ignored except for Read Status Register and Read JEDEC ID.
 *
 * BUSY is a read only bit in the status register (S0) that is set to a 1
 * state when the device is powering up or executing a Page Data Read, Bad
 * Block Management, Program Execute, Block Erase, Program Execute for OTP
 * area, OTP Locking or after a Continuous Read instruction.
 *
 * datasheet pg 20
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval 0 if the device is busy,  nonzero integer (1) if it's not.
 */
static uint8_t flash_is_busy(W25N01GV_Flash *flash) {
	uint8_t status_register = read_status_register(flash, W25N01GV_SR3_STATUS_REG_ADR);
	return status_register & W25N01GV_SR3_OPERATION_IN_PROGRESS;
}

/**
 * Writes 1 byte to the selected status register. Takes at least 50 ns to complete.
 *
 * The status registers available to write are:
 * Protection Register (SR1)				Address: W25N01GV_SR1_PROTECTION_REG_ADR (0xA0)
 * Configuration Register (SR2)			Address: W25N01GV_SR2_CONFIG_REG_ADR (0xB0)
 *
 * TODO: specify delay
 *
 * datasheet pg 29
 *
 * @param flash              <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param register_adr       <uint8_t>            The address of the status register to be written to
 * @param register_write_val <uint8_t>            The 8bit value to write to the status register
 * 	Note: this will overwrite all values in the register
 */
static void write_status_register(W25N01GV_Flash *flash, uint8_t register_adr,
		uint8_t register_write_val) {
	uint8_t tx[3] = {W25N01GV_WRITE_STATUS_REGISTER, register_adr, register_write_val};

	spi_transmit(flash, tx, 3);

	while (flash_is_busy(flash));	 // Wait for writing to finish
}

/**
 * Checks to see if all 20 entries in the Bad Block Management look up table are full.
 *
 * datasheet pg 19, 32
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval 0 if LUT is not full, nonzero int (64) if full
 */
static uint8_t BBM_look_up_table_is_full(W25N01GV_Flash *flash) {
	uint8_t status_register = read_status_register(flash, W25N01GV_SR3_STATUS_REG_ADR);
	return status_register & W25N01GV_SR3_BBM_LOOK_UP_TABLE_FULL;
}

/**
 * Read the bad block management look up table. The manufacturer marks some of
 * the bad memory blocks and writes them into a look up table. The datasheet
 * says to scan all blocks before writing or erasing on a new flash chip
 * so you don't delete the bad block information that's loaded at the factory.
 *
 * TODO: maybe shorten the parameter names
 *
 * datasheet pg 32
 *
 * @param flash                    <W25N01GV_Flash*> Struct used to store flash pins and addresses
 * @param logical_block_addresses  <uint16_t*>       Pointer to array of 20 uint16_t's to store the table's LBAs
 * @param physical_block_addresses <uint16_t*>       Pointer to array of 20 uint16_t's to store the table's PBAs
 */
static void read_BBM_look_up_table(W25N01GV_Flash *flash, uint16_t *logical_block_addresses, uint16_t *physical_block_addresses) {
	uint8_t tx[2] = {W25N01GV_READ_BBM_LOOK_UP_TABLE, 0};	 // 2nd byte is unused
	uint8_t rx[80];

	spi_transmit_receive(flash, tx, 2, rx, 80);

	// Format the received bytes into the user-supplied arrays
	for (int i = 0; i < 20; i++) {
		logical_block_addresses[i] = W25N01GV_PACK_2_BYTES_TO_UINT16(rx+(2*i));
		physical_block_addresses[i] = W25N01GV_PACK_2_BYTES_TO_UINT16(rx+(2*i)+1);
	}
}

/**
 * Loads a page specified by the user into the device's buffer.
 * Load process takes 25 microseconds if ECC is disabled, and 60 microseconds if enabled.
 * The device will be in a BUSY state and ignore most commands until loading finishes.
 *
 * datasheet pg 38
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param page_num   <uint16_t>           Page number of data to load to the device's buffer
 * @retval SPI status code
 */
static HAL_StatusTypeDef load_page(W25N01GV_Flash *flash, uint16_t page_num) {
	uint8_t page_num_8bit_array[2] = W25N01GV_UNPACK_UINT16_TO_2_BYTES(page_num);
	uint8_t tx[4] = {W25N01GV_PAGE_DATA_READ, 0, page_num_8bit_array[0], page_num_8bit_array[1]};  // 2nd byte is unused

	HAL_StatusTypeDef spi_status = spi_transmit(flash, tx, 4);

  while(flash_is_busy(flash));  // Wait for the page to load

  return spi_status;
}

/**
 * Unlocks write capabilities on the device. All memory arrays are locked to
 * read-only when the device is powered on to protect data. This function
 * unlocks memory by changing the block protect bits in the Protection register
 * to all 0's. It does not change the other bits in the register.
 *
 * datasheet pg 15, 21
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 */
static void unlock_flash(W25N01GV_Flash *flash) {
	// Read the current contents of the protection register
	uint8_t protection_register = read_status_register(flash, W25N01GV_SR1_PROTECTION_REG_ADR);

	// Remove the block protect bits, and only keep the 3 non-protect bits, if they're already enabled.
	uint8_t unlocked_protection_register = protection_register & ~(W25N01GV_SR1_BLOCK_PROTECT_BP3
			| W25N01GV_SR1_BLOCK_PROTECT_BP2
			| W25N01GV_SR1_BLOCK_PROTECT_BP1
			| W25N01GV_SR1_BLOCK_PROTECT_BP0
			| W25N01GV_SR1_BLOCK_PROTECT_TB);

	// Write the new value of the status register block protect bits off
	write_status_register(flash, W25N01GV_SR1_PROTECTION_REG_ADR, unlocked_protection_register);
}

/**
 * Locks write capabilities on the device. This function unlocks memory by
 * changing the block protect bits in the Protection register to a locked
 * configuration. It does not change the other bits in the register.
 *
 * fix this function - first remove the bits, then add them back on
 * TODO implemented, now test this fix
 *
 * datasheet pg 15, 21
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 */
static void lock_flash(W25N01GV_Flash *flash) {
	// Read the current contents of the protection register
	uint8_t protection_register = read_status_register(flash, W25N01GV_SR1_PROTECTION_REG_ADR);

	// Enabling bits BP3 and BP2 and disabling the others will lock the entire 128MB memory array
	// First remove all protection bits
	uint8_t locked_protection_register = protection_register & ~(W25N01GV_SR1_BLOCK_PROTECT_BP3
			| W25N01GV_SR1_BLOCK_PROTECT_BP2
			| W25N01GV_SR1_BLOCK_PROTECT_BP1
			| W25N01GV_SR1_BLOCK_PROTECT_BP0
			| W25N01GV_SR1_BLOCK_PROTECT_TB);

	// Then add on the bits that are needed to lock flash
	locked_protection_register = protection_register
			| (W25N01GV_SR1_BLOCK_PROTECT_BP3 | W25N01GV_SR1_BLOCK_PROTECT_BP2);

	write_status_register(flash, W25N01GV_SR1_PROTECTION_REG_ADR, locked_protection_register);
}

/**
 * Enables writing to flash by sending a command to set the
 * Write Enable Latch (WEL) bit in the status register to 1.
 *
 * The WEL bit is automatically reset after Power-up and upon completion of
 * the Page Program, Quad Page Program, Block Erase, Reset and Bad Block
 * Management instructions.
 *
 * datasheet pg 30
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval SPI status code
 */
static HAL_StatusTypeDef enable_write(W25N01GV_Flash *flash) {
	uint8_t tx[1] = { W25N01GV_WRITE_ENABLE };
	return spi_transmit(flash, tx, 1);
}

/**
 * Disables writing to flash by sending a command to set the
 * Write Enable Latch (WEL) bit in the status register to 0.
 *
 * The WEL bit is automatically reset after Power-up and upon completion of
 * the Page Program, Quad Page Program, Block Erase, Reset and Bad Block
 * Management instructions.
 *
 * datasheet pg 30
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval SPI status code
 */
static HAL_StatusTypeDef disable_write(W25N01GV_Flash *flash) {
	uint8_t tx[1] = { W25N01GV_WRITE_DISABLE };
	return spi_transmit(flash, tx, 1);
}

/**
 * Writes data to the device's buffer in preparation for writing
 * it to memory. It writes the data at the specified column (byte), and writes
 * data up to column 2047 or the end of the user supplied data array.
 *
 * datasheet pg 35
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param data       <uint8_t*>           Data array containing data to write to flash
 * @param num_bytes  <uint16_t>           Number of bytes to write
 * @param column_adr <uint16_t>           Byte in buffer to start writing at
 * @retval SPI status code
 */
static HAL_StatusTypeDef write_page_to_buffer(W25N01GV_Flash *flash, uint8_t *data,
		uint16_t num_bytes, uint16_t column_adr) {

	uint8_t column_adr_8bit_array[2] = W25N01GV_UNPACK_UINT16_TO_2_BYTES(column_adr);
	uint8_t tx1[3] = {W25N01GV_LOAD_PROGRAM_DATA, column_adr_8bit_array[0], column_adr_8bit_array[1]};

	// Ignore all data that would be written to column 2048 and after.
	// You don't want to overwrite the extra memory at the end of the page.
	// (If the onboard ECC is turned on, this happens automatically, but just in case)
	if (num_bytes > W25N01GV_PAGE_MAIN_NUM_BYTES)
		num_bytes = W25N01GV_PAGE_MAIN_NUM_BYTES;

	HAL_StatusTypeDef tx1_status, tx2_status;

	// Not using spi_transmit() because I didn't want to mess with combining the tx arrays
	__disable_irq();
	HAL_GPIO_WritePin(flash->cs_base, flash->cs_pin, W25N01GV_CS_ACTIVE);
	tx1_status = HAL_SPI_Transmit(flash->SPI_bus, tx1, 3, W25N01GV_SPI_TIMEOUT);
	tx2_status = HAL_SPI_Transmit(flash->SPI_bus, data, num_bytes, W25N01GV_SPI_TIMEOUT);
	HAL_GPIO_WritePin(flash->cs_base, flash->cs_pin, W25N01GV_CS_INACTIVE);
	__enable_irq();

	return tx1_status | tx2_status;  // Return both status codes
}

/**
 * Run the program execute command to store the data in the device's buffer into
 * memory at the specified page. This should be run after running write_page_to_buffer().
 *
 * This function incurs a typical delay of 250 microseconds,
 * with a max delay of 700 microseconds.
 *
 * datasheet pg 37
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param page_adr   <uint16_t>           The page for the buffer to be written to.
 * @retval SPI error code
 */
static HAL_StatusTypeDef program_buffer_to_memory(W25N01GV_Flash *flash, uint16_t page_adr) {
	uint8_t page_adr_8bit_array[2] = W25N01GV_UNPACK_UINT16_TO_2_BYTES(page_adr);
	uint8_t tx[4] = {W25N01GV_PROGRAM_EXECUTE, 0, page_adr_8bit_array[0], page_adr_8bit_array[1]};  // 2nd byte unused

	HAL_StatusTypeDef tx_status = spi_transmit(flash, tx, 4);
	while (flash_is_busy(flash));	 // Wait for the data to be written to memory

	return tx_status;
}

/**
 * Returns whether or not the last program write command executed successfully
 * by reading the status register (SR3) and checking the program failure bit.
 *
 * This should always return true if the last page written to is in a
 * protected part of the memory array (for this firmware, when flash is
 * locked), or if the write enable command is not given before writing.
 * In both cases, flash's memory array at that page shouldn't be changed.
 *
 * datasheet pg 20
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval 0 if write was successful, nonzero int if unsuccessful
 */
static uint8_t get_write_failure_status(W25N01GV_Flash *flash) {
	uint8_t status_register = read_status_register(flash, W25N01GV_SR3_STATUS_REG_ADR);
	return status_register & W25N01GV_SR3_PROGRAM_FAILURE;
}

/**
 * Returns whether or not the last erase command executed successfully by
 * reading the status register (SR3) and checking the erase failure bit.
 *
 * datasheet pg 20
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval 0 if erase was successful, nonzero int if unsuccessful
 */
static uint8_t get_erase_failure_status(W25N01GV_Flash *flash) {
	uint8_t status_register = read_status_register(flash, W25N01GV_SR3_STATUS_REG_ADR);
	return status_register & W25N01GV_SR3_ERASE_FAILURE;
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
 * 2 milliseconds (datasheet pg 59).
 *
 * datasheet pg 34
 *
 * Note: the input parameter is the address of a page in the block you want to
 * erase (between 0 and W25N01GV_NUM_PAGES-1), not the block number (0 to 1023).
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param page_adr   <uint16_t>           Address of the page whose block should be erased
 * @retval 0 if erase was successful, nonzero int if at least 1 block failed to erase
 */
static uint8_t erase_block(W25N01GV_Flash *flash, uint16_t page_adr) {
	while(flash_is_busy(flash));  // Don't try to erase while flash is busy

	enable_write(flash);	// Set WEL bit high, it will automatically be set back to 0 after the command executes

	uint8_t page_adr_8bit_array[2] = W25N01GV_UNPACK_UINT16_TO_2_BYTES(page_adr);
	uint8_t tx[4] = {W25N01GV_ERASE_BLOCK, 0, page_adr_8bit_array[0], page_adr_8bit_array[1]};	// 2nd byte unused
	spi_transmit(flash, tx, 4);

	disable_write(flash);	// Disable WEL just in case the erase block command doens't execute

	while (flash_is_busy(flash));  // Wait for it to finish erasing

	return get_erase_failure_status(flash);
}

/**
 * Reads the status of the error corrections done on the last read command.
 * This function should be used after read operations to verify data integrity.
 *
 * It reads the ECC1 and ECC0 bits of the status register (SR3) and determines
 * what the error status is, based on the table in the datasheet.
 *
 * datasheet pg 20
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @retval The ECC status of the last read command
 */
static W25N01GV_ECC_Status get_ECC_status(W25N01GV_Flash *flash) {
	uint8_t status_register, ECC1, ECC0;

	status_register = read_status_register(flash, W25N01GV_SR3_STATUS_REG_ADR);
	ECC1 = status_register & W25N01GV_SR3_ECC_STATUS_BIT_1;
	ECC0 = status_register & W25N01GV_SR3_ECC_STATUS_BIT_0;

	// Return status according to table on datasheet pg 20
	if (!ECC1 && !ECC0)
		return SUCCESS_NO_CORRECTIONS;
	else if (!ECC1 && ECC0)
		return SUCCESS_WITH_CORRECTIONS;
	else if (ECC1 && !ECC0)
		return ERROR_ONE_PAGE;
	else  // else if (ECC1 && ECC0)
		return ERROR_MULTIPLE_PAGES;
}

/**
 * Reads the contents of the flash's buffer into an array, starting at the
 * specified column and going until it reaches the end of the buffer or
 * reads in buffer_size number of bytes.
 *
 * This function incurs a delay that varies linearly with num_bytes
 * and the SPI clock period.
 *
 * This function is called by read_bytes_from_page().
 *
 * datasheet pg 39
 *
 * @param flash       <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param buffer      <uint8_t*>           Array to read the contents of the device's buffer into
 * @param num_bytes   <uint16_t>           Number of bytes to read into buffer
 * @param column_adr  <uint16_t>           Starting column address of the data to be read in
 * @retval SPI status code
 */
static HAL_StatusTypeDef read_flash_buffer(W25N01GV_Flash *flash, uint8_t *buffer,
		uint16_t num_bytes, uint16_t column_adr) {

	uint8_t column_adr_8bit_array[2] = W25N01GV_UNPACK_UINT16_TO_2_BYTES(column_adr);
	uint8_t tx[4] = {W25N01GV_READ_DATA, column_adr_8bit_array[0], column_adr_8bit_array[1], 0};  // last byte is unused

	HAL_StatusTypeDef spi_status = spi_transmit_receive(flash, tx, 4, buffer, num_bytes);

	return spi_status;
}

/**
 * Reads the specified number of uint8_t bytes from the
 * specified page and column into an array.
 *
 * It should only be used with buffer mode enabled [buffer mode is enabled
 * by default both during power-on and reset, and in init_flash()]
 *
 * This function incurs a delay that varies linearly with num_bytes
 * and the SPI clock period.
 *
 * Note: if num_bytes > (2112 - column_adr), then it will only
 * read in (2112 - column_adr) bytes
 *
 * TODO: add datasheet pages
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param buffer     <uint8_t*>           Data buffer to read data into
 * @param num_bytes  <uint16_t>           Number of bytes to read in (see note on range)
 * @param page_adr   <uint16_t>           The page to read data from
 * @param column_adr <uint16_t>           The column to start reading data from
 * @retval ECC Status
 */
static W25N01GV_ECC_Status read_bytes_from_page(W25N01GV_Flash *flash, uint8_t *buffer, uint16_t num_bytes,
		uint16_t page_adr, uint16_t column_adr) {

	load_page(flash, page_adr);  // Load the page into flash's buffer
	read_flash_buffer(flash, buffer, num_bytes, column_adr);

	return get_ECC_status(flash);
}

/**
 * Writes the contents of data into flash at the specified page and column.
 * It writes to the device's buffer, then programs the buffer data into flash memory.
 *
 * This function incurs a delay that varies linearly with num_bytes
 * and the SPI clock period.
 *
 * Note: unlock_flash() and enable_write() must be called before
 * calling this function, otherwise it will do nothing.
 *
 * TODO: add datasheet pages
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 * @param num_bytes  <uint16_t>           Number of bytes to write to flash
 * @param page_adr   <uint16_t>           Page to write data to
 * @param column_adr <uint16_t>           Column of page to start writing data at
 * @retval Write failure status bit
 */
static uint8_t write_bytes_to_page(W25N01GV_Flash *flash, uint8_t *data, uint16_t num_bytes,
		uint16_t page_adr, uint16_t column_adr) {

	write_page_to_buffer(flash, data, num_bytes, column_adr);
	program_buffer_to_memory(flash, page_adr);

	// TODO set this so that it still returns an error if it doesn't explicitly read a 0
	// maybe check HAL_status - right now it returns 0 even if flash isn't plugged in
	return get_write_failure_status(flash);
}

/**
 * Set the ECC-E bit in the configuration register (SR2) to 1, enabling the
 * onboard error correction algorithms. If ECC-E is already 1, does nothing.
 *
 * datasheet pg 18
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 */
static void enable_ECC(W25N01GV_Flash *flash) {
	uint8_t config_reg_read = read_status_register(flash, W25N01GV_SR2_CONFIG_REG_ADR);
	uint8_t ECC_enabled_register = config_reg_read | W25N01GV_SR2_ECC_ENABLE;	 // OR: turn bit on

	if (ECC_enabled_register != config_reg_read)
		write_status_register(flash, W25N01GV_SR2_CONFIG_REG_ADR, ECC_enabled_register);
}

/**
 * Set the ECC-E bit in the configuration register (SR2) to 0, disabling the
 * onboard error correction algorithms. If ECC-E is already 0, does nothing.
 * Don't actually use this, but I included it because why not.
 *
 * datasheet pg 18
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 */
static void disable_ECC(W25N01GV_Flash *flash) {
	uint8_t config_reg_read = read_status_register(flash, W25N01GV_SR2_CONFIG_REG_ADR);
	uint8_t ECC_disabled_register = config_reg_read & ~W25N01GV_SR2_ECC_ENABLE;	 // Remove bit

	if (ECC_disabled_register != config_reg_read)
		write_status_register(flash, W25N01GV_SR2_CONFIG_REG_ADR, ECC_disabled_register);
}

/**
 * Sets the device to buffer read mode, which limits the user to reading
 * up to one page at a time before loading a new page. It sets the BUF bit
 * in the configuration register to 1 if BUF=0, and does nothing if BUF=1.
 *
 * datasheet pg 18
 *
 * @param flash      <W25N01GV_Flash*>    Struct used to store flash pins and addresses
 */
static void enable_buffer_mode(W25N01GV_Flash *flash) {
	uint8_t config_reg_read = read_status_register(flash, W25N01GV_SR2_CONFIG_REG_ADR);
	uint8_t buffer_enabled_register = config_reg_read | W25N01GV_SR2_BUFFER_READ_MODE;	// OR: turn bit on
	if (buffer_enabled_register != config_reg_read)
		write_status_register(flash, W25N01GV_SR2_CONFIG_REG_ADR, buffer_enabled_register);
}


/* Public function definitions */

void init_flash(W25N01GV_Flash *flash, SPI_HandleTypeDef *SPI_bus_in,
		GPIO_TypeDef *cs_base_in,	uint16_t cs_pin_in) {
	flash->SPI_bus = SPI_bus_in;
	flash->cs_base = cs_base_in;
	flash->cs_pin = cs_pin_in;
	flash->next_page_to_read = 0;

	reset_flash(flash);

	enable_ECC(flash);  // Should be enabled by default, but enable ECC just in case
	enable_buffer_mode(flash);  // -IG models start with buffer mode by default, -IT models don't
}

uint8_t is_flash_ID_correct(W25N01GV_Flash *flash) {

	uint8_t tx[2] = {W25N01GV_READ_JEDEC_ID, 0};	// Second byte is unused
	uint8_t rx[3];

	spi_transmit_receive(flash, tx, 2, rx, 3);
	uint8_t manufacturer_ID = rx[0];
	uint16_t device_ID = W25N01GV_PACK_2_BYTES_TO_UINT16(rx+1);

	if (manufacturer_ID == W25N01GV_MANUFACTURER_ID && device_ID == W25N01GV_DEVICE_ID)
		return 1;
	else
		return 0;
}

HAL_StatusTypeDef reset_flash(W25N01GV_Flash *flash) {
	while(flash_is_busy(flash));  // Wait for it to finish its current operation

	uint8_t tx[1] = { W25N01GV_DEVICE_RESET };
	uint8_t spi_status = spi_transmit(flash, tx, 1);

	while(flash_is_busy(flash));  // Wait for it to reset

	return spi_status;
}

uint8_t write_to_flash(W25N01GV_Flash *flash, uint8_t *data, uint32_t num_bytes) {

	// If there's not enough space, truncate the data
	uint32_t bytes_remaining = get_bytes_remaining(flash);
	if (num_bytes > bytes_remaining)
		num_bytes = bytes_remaining;

	uint32_t write_counter = 0;  // Track how many bytes have been written so far
	uint8_t failure_status = 0;  // Track write and erase errors

	// Disable write protection
	unlock_flash(flash);
	enable_write(flash);

	while (write_counter < num_bytes) {

		// If there's not enough space on the page, only write as much as will fit
		uint16_t num_bytes_to_write_on_page = num_bytes - write_counter;
		if (num_bytes_to_write_on_page > W25N01GV_PAGE_MAIN_NUM_BYTES - flash->next_free_column)
			num_bytes_to_write_on_page = W25N01GV_PAGE_MAIN_NUM_BYTES - flash->next_free_column;

		// Write the array (or a part of it if it's too long for one page) to flash
		failure_status |= write_bytes_to_page(flash, data + write_counter,
				num_bytes_to_write_on_page,	flash->current_page, flash->next_free_column);

		write_counter += num_bytes_to_write_on_page;

		// If there's room left over at the end of the page,
		// increment the column counter and leave the page counter the same
		if (flash->next_free_column + num_bytes_to_write_on_page < W25N01GV_PAGE_MAIN_NUM_BYTES)
			flash->next_free_column += num_bytes_to_write_on_page;

		// If it fills the current page and runs out of pages, set the column counter over
		// the limit so it can't write again (will make get_bytes_remaining() return 0)
		else if (flash->current_page == W25N01GV_NUM_PAGES-1)
			flash->next_free_column = W25N01GV_PAGE_MAIN_NUM_BYTES;

		// Otherwise if there's more pages left, bring the address counter to the next page
		// and reset the column counter
		else {
			flash->next_free_column = 0;
			flash->current_page++;  // data got truncated earlier so no worries about running out of pages
		}
	}

	// Re enable write protection
	disable_write(flash);
	lock_flash(flash);

	return failure_status;
}

void reset_flash_read_pointer(W25N01GV_Flash *flash) {
	flash->next_page_to_read = 0;
}

W25N01GV_ECC_Status read_next_2KB_from_flash(W25N01GV_Flash *flash, uint8_t *buffer) {
	read_bytes_from_page(flash, buffer,	W25N01GV_PAGE_MAIN_NUM_BYTES, flash->next_page_to_read, 0);
	flash->next_page_to_read++;  // Increment the page read counter

	return get_ECC_status(flash);
	// TODO: continue testing for read errors
}

uint8_t erase_flash(W25N01GV_Flash *flash) {
	uint8_t failure_status = 0;

	unlock_flash(flash);

	// Loop through every block to erase them one by one
	for (uint16_t block_count = 0; block_count < W25N01GV_NUM_BLOCKS; block_count++) {
		erase_block(flash, block_count * W25N01GV_PAGES_PER_BLOCK);  // address of first page in each block

		// Check if the erase failed
		failure_status |= get_erase_failure_status(flash);
	}

	lock_flash(flash);

	// Reset the address pointer after erasing
	flash->current_page = 0;
	flash->next_free_column = 0;

	return failure_status;
}

uint32_t get_bytes_remaining(W25N01GV_Flash *flash) {
	return (W25N01GV_NUM_BLOCKS * W25N01GV_PAGES_PER_BLOCK * W25N01GV_PAGE_MAIN_NUM_BYTES)
			- (flash->current_page * W25N01GV_PAGE_MAIN_NUM_BYTES + flash->next_free_column);
}

uint16_t scan_bad_blocks(W25N01GV_Flash *flash, uint16_t *bad_blocks) {

	uint8_t read_byte[1];
	uint16_t num_bad_blocks = 0;

	for (uint16_t block_adr = 0; block_adr < W25N01GV_NUM_BLOCKS; block_adr++) {  // block 0, 1, ..., 1022, 1023
		read_bytes_from_page(flash, read_byte, 1, block_adr*W25N01GV_PAGES_PER_BLOCK, 0);  // page 0, 64, 128, ...

		if (*read_byte != 0xFF) {  // Look for non-0xFF bytes
			bad_blocks[num_bad_blocks] = block_adr;
			num_bad_blocks++;
		}
	}

	return num_bad_blocks;
}

#endif	// End SPI include protection
