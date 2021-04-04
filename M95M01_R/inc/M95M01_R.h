/** M95M01_R.h
 *
 *
 * 	Author: Leif Gullstad (leifg@umich.edu)
 * 	Created: March 17, 2021
 * 	Modified: April 4, 2021
 *
 * 	Header file for communication with M95M01-R EEPROM
 * 	datasheet: https://www.st.com/resource/en/datasheet/m95m01-r.pdf
 *	Byte addressable with 24 bit addresses
 *
 *	read_eeprom()	-	Read from specified part of the eeprom.
 *
 *	write_eeprom()	-	Write to a specified portion of eeprom. Default to
 *						one past end of current data held in eeprom.
 *						Returns 1 if successful. Returns 0 if error
 *
 *	memory_remaining()	-	Returns how much memory remains on the chip in
 *							bytes(?)
 *
 */
#ifndef M95M01_R_H
#define M95M01_R_H

/* Includes */
#include "stdint.h"
#include "stdlib.h"
#include "stm32f4xx_hal.h"

#define M95M01_R_MEM_SIZE 131072 // Bytes

// GPIO pinout memory addresses
typedef struct GPIO_M95M01_R_Pinfo {
    GPIO_TypeDef* M95M01_R_CS_PORT;     // PORT belonging to Chip select pin
    uint16_t M95M01_R_CS_ADDR;          // PIN belonging to CS pin
} GPIO_M95M01_R_Pinfo;



/**
 * Writes to a specified address and data amount the EEPROM
 *
 * @param SPI_BUS	<SPI_HandleTypeDef*>	Settings for SPI
 * @param pinfo		<GPIO_M95M01_R_Pinfo*>	Contains pin defines
 * @param dataPtr	<uint8_t*>				Pointer to the start of the data to be written
 * @param dataSize	<uint16_t>				Amount of data (in bytes) to be written
 * @param address	<uint32_t>				32 bit int holding 24 bit address that specifies where to begin writing
 *
 */
void write_eeprom(SPI_HandleTypeDef *SPI_BUS, GPIO_M95M01_R_Pinfo *pinfo, uint8_t *dataPtr, uint16_t dataSize, uint32_t address);


/**
 * Reads a specified address and amount of data from the EEPROM
 *
 * @param SPI_BUS	<SPI_HandleTypeDef*>	Settings for SPI
 * @param pinfo		<GPIO_M95M01_R_Pinfo*>	Contains pin defines
 * @param dataPtr	<uint8_t*>				Pointer to a buffer to read data into
 * @param dataSize	<uint16_t>				Amount of data (in bytes) to be read
 * @param address	<uint32_t>				32 bit int holding 24 bit address that specifies where to begin reading
 *
 */
void read_eeprom(SPI_HandleTypeDef *SPI_BUS, GPIO_M95M01_R_Pinfo *pinfo, uint8_t *dataPtr, uint16_t dataSize, uint32_t address);


/**
 * Transmits a given address to the eeprom for the purpose of reads and writes
 *
 * @param address	<uint32_t>	address desired to be transmitted
 * @result						transmits the address in 1 byte chunks to the eeprom
 */
void transmit_address_eeprom(SPI_HandleTypeDef *SPI_BUS, uint32_t address);

/**
 * Returns the amount of memory remaining on the chip (in bytes) for the given session.
 * Return result is based on subtracting the sum of all write commands from the total data
 * 		the eeprom can hold
 */
uint32_t memory_remaining_eeprom();


#endif

