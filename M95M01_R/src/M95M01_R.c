/**
 * M95M01_R.c
 *
 *
 *
 */
#include "../Inc/M95M01_R.h"

const uint8_t M95M01_R_WREN = 0b00000110; // Write Enable
const uint8_t M95M01_R_WRDI = 0b00000100; // Write Disable
const uint8_t M95M01_R_RDSR = 0b00000101; // Read Status Register
const uint8_t M95M01_R_WRSR = 0b00000001; // Write Status Register
const uint8_t M95M01_R_READ = 0b00000011; // Read from Memory Array
const uint8_t M95M01_R_WRITE = 0b00000010; // Write to Memory Array
const uint8_t M95M01_R_RDID = 0b10000011; // Read Identification Page
const uint8_t M95M01_R_WRID = 0b10000010; // Write Identification Page
const uint8_t M95M01_R_RDLS = 0b10000011; // Reads the Identification Page Lock Status
const uint8_t M95M01_R_LID = 0b10000010; // Locks the identification Page in read-only mode


uint32_t adjust_mem_remaining_eeprom(uint16_t dataSize) {
	static uint32_t memoryRemaining = M95M01_R_MEM_SIZE; // Memory starts with 131,072 Bytes on 512 Pages
	memoryRemaining -= (uint32_t)dataSize;
	return memoryRemaining;
}


uint32_t memory_remaining_eeprom() {
	return adjust_mem_remaining_eeprom(0);
}


void init_eeprom(SPI_HandleTypeDef *SPI_BUS, GPIO_M95M01_R_Pinfo *pinfo) {

	if(HAL_SPI_Init(SPI_BUS) != HAL_OK) {
		// TODO: Error handling here
	}

	// Set chip select line to high, the default
	HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_SET);
}

// TODO: Error handling?
// TODO: Check for WIP before certain commands?

void write_eeprom(SPI_HandleTypeDef *SPI_BUS, GPIO_M95M01_R_Pinfo *pinfo, uint8_t *dataPtr, uint16_t dataSize, uint32_t address) {

	HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_RESET); // Bring chip select line low
	HAL_SPI_Transmit(SPI_BUS, (uint8_t *)&M95M01_R_WREN, 1, 100); // Write enable
	HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_SET); // Bring chip select line back high

	HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_RESET); // Bring chip select line low
	HAL_SPI_Transmit(SPI_BUS, (uint8_t *)&M95M01_R_WRITE, 1, 100); // Telling chip I am going to write to it
	transmit_address_eeprom(SPI_BUS, address);
	HAL_SPI_Transmit(SPI_BUS, dataPtr, dataSize, 100); // Write data
	HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_SET); // Bring chip select line back high
	int wip = 1;
	char str[10];
	while(wip) { // Pauses program until writing is done
		HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_RESET);
		HAL_SPI_Transmit(SPI_BUS, (uint8_t *)&M95M01_R_RDSR, 1, 100);
		HAL_SPI_Receive(SPI_BUS, (uint8_t *)str, 1, 100);
		HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_SET);

		wip = str[0] & 0b00000001;
	}


	HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_RESET); // Bring chip select line low
	HAL_SPI_Transmit(SPI_BUS, (uint8_t *)&M95M01_R_WRDI, 1, 100); // Write disable
	HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_SET); // Bring chip select line back high

	adjust_mem_remaining_eeprom(dataSize);


}

void read_eeprom(SPI_HandleTypeDef *SPI_BUS, GPIO_M95M01_R_Pinfo *pinfo, uint8_t *dataPtr, uint16_t dataSize, uint32_t address) {

	HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_RESET); // Bring chip select line low
	HAL_SPI_Transmit(SPI_BUS, (uint8_t *)&M95M01_R_READ, 1, 100); // Tell chip I want to read from it
	transmit_address_eeprom(SPI_BUS, address);
	HAL_SPI_Receive(SPI_BUS, dataPtr, dataSize, 100);
	HAL_GPIO_WritePin(pinfo->M95M01_R_CS_PORT, pinfo->M95M01_R_CS_ADDR, GPIO_PIN_SET);
}


void split_address_eeprom(uint32_t address, uint8_t *addressPtr) {

	uint32_t bitMask = 0x00FF0000; // 8 ones followed by 16 zeros
	addressPtr[0] = (address & bitMask) >> 16;
	bitMask = bitMask >> 8; // 8 ones followed by 8 zeros
	addressPtr[1] = (address & bitMask) >> 8;
	bitMask = bitMask >> 8; // 8 ones
	addressPtr[2] = (address & bitMask) >> 0;

}

void transmit_address_eeprom(SPI_HandleTypeDef *SPI_BUS, uint32_t address) {
	uint8_t addressPtr[3];

	split_address_eeprom(address, addressPtr);

	HAL_SPI_Transmit(SPI_BUS, addressPtr, 3, 100);
}




