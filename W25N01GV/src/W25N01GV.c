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
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created July 20, 2020
 * Last edited July 27, 2020
 */

#include "W25N01GV.h"

#ifdef HAL_SPI_MODULE_ENABLED	// begin SPI include protection

const uint8_t	W25N01GV_MANUFACTURER_ID								= 0xEF;
const uint16_t W25N01GV_DEVICE_ID											= 0xAA21;

const uint8_t W25N01GV_CS_ACTIVE    									= GPIO_PIN_RESET;  // Chip is active low
const uint8_t W25N01GV_CS_INACTIVE  									= GPIO_PIN_SET;

const uint8_t SPI_TIMEOUT															= 0xFF;

/* Commands */
// Summary of commands and usage on datasheet pg 23-25
const uint8_t DEVICE_RESET													 	= 0xFF;
const uint8_t READ_JEDEC_ID														= 0x9F;
const uint8_t READ_STATUS_REGISTER										= 0x0F;	// 0x05 also works
const uint8_t WRITE_STATUS_REGISTER										= 0x1F;	// 0x01 also works
const uint8_t WRITE_ENABLE														= 0x06;
const uint8_t WRITE_DISABLE														= 0x04;
const uint8_t BAD_BLOCK_MANAGEMENT										= 0xA1;
const uint8_t READ_BBM_LOOK_UP_TABLE									= 0xA5;
const uint8_t LAST_ECC_FAILURE_PAGE_ADDRESS						= 0xA9;
const uint8_t BLOCK_ERASE_128KB												= 0xD8;
const uint8_t LOAD_PROGRAM_DATA												= 0x02;
const uint8_t RANDOM_LOAD_PROGRAM_DATA								= 0x84;
const uint8_t QUAD_LOAD_PROGRAM_DATA									= 0x32;
const uint8_t QUAD_RANDOM_LOAD_PROGRAM_DATA						= 0x34;
const uint8_t PROGRAM_EXECUTE													= 0x10;
const uint8_t PAGE_DATA_READ 													= 0x13;
const uint8_t READ_DATA																= 0x03;
const uint8_t FAST_READ																= 0x0B;
const uint8_t FAST_READ_4BYTE_ADDRESS									= 0x0C;
const uint8_t FAST_READ_DUAL_OUTPUT										= 0x3B;
const uint8_t FAST_READ_DUAL_OUTPUT_4BYTE_ADDRESS			= 0x3C;
const uint8_t FAST_READ_QUAD_OUTPUT										= 0x6B;
const uint8_t FAST_READ_QUAD_OUTPUT_4BYTE_ADDRESS			= 0x6C;
const uint8_t FAST_READ_DUAL_IO												= 0xBB;
const uint8_t FAST_READ_DUAL_IO_4BYTE_ADDRESS					= 0xBC;
const uint8_t FAST_READ_QUAD_IO												= 0xEB;
const uint8_t FAST_READ_QUAD_IO_4BYTE_ADDRESS					= 0xEC;

/* Status Register addressses */
const uint8_t SR1_PROTECTION_REGISTER_ADDRESS					= 0xA0;	// Listed as 0xAx in the datasheet
const uint8_t SR2_CONFIGURATION_REGISTER_ADDRESS			= 0xB0;	// Listed as 0xBx in the datasheet
const uint8_t SR3_STATUS_REGISTER_ADDRESS							= 0xC0;	// Listed as 0xBx in the datasheet

/* Status Register bits */
// Protection register - datasheet pg 15
const uint8_t SR1_BLOCK_PROTECT_BP3										= 0x40;	// 0b01000000
const uint8_t SR1_BLOCK_PROTECT_BP2										= 0x20;	// 0b00100000
const uint8_t SR1_BLOCK_PROTECT_BP1										= 0x10;	// 0b00010000
const uint8_t SR1_BLOCK_PROTECT_BP0										= 0x08;	// 0b00001000
const uint8_t SR1_BLOCK_PROTECT_TB										= 0x04;	// 0b00000100
const uint8_t SR1_WRITE_PROTECT_ENABLE								= 0x02;	// 0b00000010
const uint8_t SR1_STATUS_REGISTER_PROTECT_SRP1				= 0x00;	// 0b10000000
const uint8_t SR1_STATUS_REGISTER_PROTECT_SRP0				= 0x01;	// 0b00000001

// Configuration register - datasheet pg 17
const uint8_t SR2_ONE_TIME_PROGRAM_LOCK								= 0x80;	// 0b10000000
const uint8_t SR2_ENTER_TOP_ACCESS_MODE								= 0x40;	// 0b01000000
const uint8_t SR2_STATUS_REGISTER_1_LOCK							= 0x20;	// 0b00100000
const uint8_t SR2_ECC_ENABLE													= 0x10;	// 0b00010000
const uint8_t SR2_BUFFER_READ_MODE										= 0x08;	// 0b00001000

// Status register - datasheet pg 19
const uint8_t SR3_BBM_LOOK_UP_TABLE_FULL					    = 0x40;	// 0b01000000
const uint8_t SR3_ECC_STATUS_SUCCESS									= 0x00;	// 0b00000000
const uint8_t SR3_ECC_STATUS_SUCCESS_WITH_CORRECTIONS	= 0x10;	// 0b00010000
const uint8_t SR3_ECC_STATUS_ERROR_ONE_PAGE						= 0x20;	// 0b00100000
const uint8_t SR3_ECC_STATUS_ERROR_MULTIPLE_PAGES			= 0x30;	// 0b00110000
const uint8_t SR3_PROGRAM_FAILURE											= 0x08;	// 0b00001000
const uint8_t SR3_ERASE_FAILURE												= 0x04;	// 0b00000100
const uint8_t SR3_WRITE_ENABLE_LATCH									= 0x02;	// 0b00000010
const uint8_t SR3_OPERATION_IN_PROGRESS								= 0x01;	// 0b00000001

/* Unique ID / Parameter / OPT Page Addresses */
//datasheet pg 52-53
const uint8_t UNIQUE_ID_PAGE_ADDRESS 									= 0x00;
const uint8_t PARAMETER_PAGE_ADDRESS									= 0x01;
const uint8_t OTP_PAGE_0															= 0x02;	// Add i to get page i, 0 <= i <= 9

/* Private functions */

void chip_select(W25N01GV_Flash *flash) {
	HAL_GPIO_WritePin(flash->cs_base, flash->cs_pin, W25N01GV_CS_ACTIVE);
}

void chip_release(W25N01GV_Flash *flash) {
	HAL_GPIO_WritePin(flash->cs_base, flash->cs_pin, W25N01GV_CS_INACTIVE);
}

/**
 * Sends a 1 byte command over SPI to the flash device.
 * Receives nothing.
 *
 * @param cmd <uint8_t> 1 byte command to be sent over SPI
 */
void send_cmd(W25N01GV_Flash *flash, uint8_t cmd) {
	__disable_irq();
	chip_select(flash);
	HAL_SPI_Transmit(flash->SPI_bus, &cmd, 1, SPI_TIMEOUT);
	chip_release(flash);
	__enable_irq();
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

	__disable_irq();
	chip_select(flash);
	HAL_SPI_Transmit(flash->SPI_bus, tx, 2, SPI_TIMEOUT);
	HAL_SPI_Receive(flash->SPI_bus, rx, 1, SPI_TIMEOUT);
	chip_release(flash);
	__enable_irq();

	return *rx;
}

/**
 * The status registers available to write are:
 * Protection Register (SR1)				Address: SR1_PROTECTION_REGISTER_ADDRESS (0xA0)
 * Configuration Register (SR2)			Address: SR2_CONFIGURATION_REGISTER_ADDRESS (0xB0)
 *
 * @param register_adr <uint8_t> The address of the status register to be read
 * @param register_write_val <uint8_t> The 8bit value to write to the status register
 */
uint8_t write_status_register(W25N01GV_Flash *flash, uint8_t register_adr, uint8_t register_write_val) {
	uint8_t status_register = read_status_register(flash, register_adr);

	uint8_t tx[3] = {WRITE_STATUS_REGISTER, register_adr, register_write_val};

	__disable_irq();
	chip_select(flash);
	HAL_SPI_Transmit(flash->SPI_bus, tx, 3, SPI_TIMEOUT);
	chip_release(flash);
	__enable_irq();


}


/* Public functions */

void init_flash(W25N01GV_Flash *flash, SPI_HandleTypeDef *SPI_bus_in,
		GPIO_TypeDef *cs_base_in,	uint16_t cs_pin_in) {
	flash->SPI_bus = SPI_bus_in;
	flash->cs_base = cs_base_in;
	flash->cs_pin = cs_pin_in;
}

/**
 * Checks to see if flash is busy. While it's busy, all commands will be
 * ignored except for Read Status Register and Read JEDEC ID.
 *
 * @retval 1 if the device is busy, 0 if it's not.
 */
bool flash_is_busy(W25N01GV_Flash *flash) {
	uint8_t status_register = read_status_register(flash, SR3_STATUS_REGISTER_ADDRESS);
	return status_register & SR3_OPERATION_IN_PROGRESS;
}

/**
 * Checks to see if all 20 entries in the Bad Block Management look up table
 * are full.
 *
 * @retval 1 if the LUT is full, 0 if not
 */
bool BBM_look_up_table_is_full(W25N01GV_Flash *flash) {
	uint8_t status_register = read_status_register(flash, SR3_STATUS_REGISTER_ADDRESS);
	return status_register & SR3_BBM_LOOK_UP_TABLE_FULL;
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
	//maybe add confirmation?
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
	//maybe add confirmation?
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
bool flash_ID_is_correct(W25N01GV_Flash *flash) {
	uint8_t tx[2] = {READ_JEDEC_ID, 0};	// second byte is unused
	uint8_t rx[3];

	__disable_irq();
	chip_select(flash);
	HAL_SPI_Transmit(flash->SPI_bus, tx, 2, SPI_TIMEOUT);
	HAL_SPI_Receive(flash->SPI_bus, rx, 3, SPI_TIMEOUT);
	chip_release(flash);
	__enable_irq();

	uint8_t manufacturer_ID = rx[0];
	uint16_t device_ID = ((uint16_t) rx[1] << 8) + rx[2];

	if (manufacturer_ID == W25N01GV_MANUFACTURER_ID && device_ID == W25N01GV_DEVICE_ID)
		return true;
	else
		return false;
}

/**
 * First checks the status register to see if the device is busy.
 * If it's busy, the function does nothing. If it's not busy, it sends the reset command.
 *
 * @retval 1 if the device was not busy and the reset command was sent,
 * 	0 if the device was busy and the reset command was not sent.
 *
 * datasheet pg 26
 */
bool reset_flash(W25N01GV_Flash *flash) {
	if (!flash_is_busy(flash)) {
		send_cmd(flash, DEVICE_RESET);
		return true;
	}
	else {
		return false;
	}
}

#endif	// end SPI include protection
