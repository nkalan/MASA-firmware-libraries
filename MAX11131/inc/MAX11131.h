/** max11131.h
 *
 * Header file for communicating with MAX11131 adc
 * datasheet:https://datasheets.maximintegrated.com/en/ds/MAX11129-MAX11132.pdf
 *
 */
#ifndef MAX11131_H
#define MAX11131_H
/* Includes */
#include "stdint.h"
#include "stdlib.h"
#include "stm32f4xx_hal.h"

/* DEBUG ENABLER */
#define	MAX11131_DEBUG_EN	(uint16_t) 0x0001 // set to 1 to enable debugging

/* Register Definitions */

// Register Identification Code
// datasheet definitions between on pg21
#define MAX31_MODE_CNTL			(uint16_t) 0x0000 // 0b0          followed by 0s
#define MAX31_CONFIG 			(uint16_t) 0x8000 // 0b1000       followed by 0s
#define MAX31_UNIPOLAR 			(uint16_t) 0x8800 // 0b10001000   followed by 0s
#define MAX31_BIPOLAR 			(uint16_t) 0x9000 // 0b1001       followed by 0s
#define MAX31_RANGE 			(uint16_t) 0x9800 // 0b10011000   followed by 0

/* Offset bits mapping adc channel numbers to bit number */
#define MAX31_CUSTOM_SCAN0_SUB	(uint8_t) 5
#define MAX31_CUSTOM_SCAN1_ADD	(uint8_t) 3

/* Register bits for adc Mode Control registers */
#define SET_SWCNV 				(uint16_t) 0x0002 //0b10  preceded by 0s
#define SET_CHAN_ID				(uint16_t) 0x0004 // 0b100 preceded by 0s

/* Register bits for adc configuration registers */
#define SET_MAX31_AVGON 		(uint16_t) 0x0200 // 0b10 00000000preceded by 0s
#define SET_MAX31_ECHO_ON		(uint16_t) 0x0004 // 0b0100		  preceded by 0s

/* Register bits for adc scan registers */
#define MAX31_CUSTOM_SCAN0 		(uint16_t) 0xA000 // 0b10100      followed by 0s
#define MAX31_CUSTOM_SCAN1 		(uint16_t) 0xA800 // 0b10101      followed by 0s
#define MAX31_CUSTOM_SCAN_ALL_0 (uint16_t) 0x01F8 // 0b00111111000
#define MAX31_CUSTOM_SCAN_ALL_1 (uint16_t) 0x07F8 // 0b11111111000

/* Channel size in FIFO register */
#define MAX31_CHANNEL_SZ		(uint8_t) 	0x0002

/* Global Var Definitions */
#define MAX31_MAX_CHANNELS		(uint8_t)	0x000E

// GPIO pinout memory addresses
typedef struct GPIO_MAX31_Pinfo {
	GPIO_TypeDef* MAX31_CS_PORT[8];
	GPIO_TypeDef* MAX31_EOC_PORT[8];
	GPIO_TypeDef* MAX31_CNVST_PORT[8];
	uint16_t MAX31_CS_ADDR[8];
	uint16_t MAX31_EOC_ADDR[8];
	uint16_t MAX31_CNVST_ADDR[8];

	uint8_t NUM_CHANNELS;
	uint8_t MAX31_CHANNELS[16];
} GPIO_MAX31_Pinfo;

GPIO_MAX31_Pinfo *pinfo;

// Mode Control Scan Registers
// SCAN STATE specifications are defined in pg22
enum SCAN_STATES {
	HOLD,
	MANUAL,
	REPEAT,
	STD_INT,
	STD_EXT,
	UPPER_INT,
	UPPER_EXT,
	CUSTOM_INT,
	CUSTOM_EXT
};

/* Public Function Prototypes */

/**
 *  Initialize ADC hardware component
 *
 *  detailed documentation starts on datasheet pg21
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object ADC is on
 *  @param pins        	<GPIO_MAX31_Pinfo*>   contains ADC pin defs,
 *                                          	refer to def above
 *  @param num_adcs     <int>   number of adcs
 *
 *  Note: assumes 8bit data framing on SPI
 */
void init_adc(SPI_HandleTypeDef* SPI_BUS, GPIO_MAX31_Pinfo *pins,
					int num_adcs);

/**
 *  Sets range to read from adc
 *
 *  general documentation starts on datasheet pg 21
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object adc is on
 *  @param adcn         <uint8_t> selected adc number
 *
 */
void set_read_adc_range(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn);

/* Private Helper Functions */

/**
 *  Convenience function for reading from configured channels on adcx
 *  Note: this function assumes adc range to read from has already
 *          been set by set_read_adc_range()
 *
 *  general documentation starts on datasheet pg 21
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object adc is on
 *  @param adcn         <uint8_t> selected adc number
 *	@param adc_out		<uint16_t*> raw adc counts for each channel (0-4096)
 *
 *	Note: adc_out should be initialized to at least size 14 to guarantee safe op
 *			if channel x, y, z are selected, then indices x, y, z will be filled
 *			in adc_out
 */
void read_adc(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn,
		uint16_t* adc_out);

/**
 *  Selects/Disables adc for SPI transmissions
 *
 *  @param adcn         <uint8_t> adc number
 *  @param state        <GPIO_PinState> state of GPIO PIN
 *                            Note:
 */
void set_adc(uint8_t adcn, GPIO_PinState state);

/**
 * 	Convenience function for updating GPIO_MAX31_Pinfo to read from pins 0-13
 *
 * 	@param adcn			<uint8_t> selected adc number
 */
void configure_read_adc_all(uint8_t adcn);

/**
 *	Private function for transmit and receiving bytes to selected ADC
 *  
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object adc is on
 *  @param tx	        <uint8_t*> bytes to transmit (expected size 2)
 *	@param adc_out		<uint8_t*> bytes to receive (expected size 2)
 */
void write_adc_reg(SPI_HandleTypeDef *SPI_BUS, uint8_t *tx, uint8_t *rx);

/**
 *	Private function for packing 16 bit command to 8 bit chunks
 *  
 *  @param cmd      	<uint16_t> 16 bit command
 *  @param tx	        <uint8_t*> arr of 16 bit command MSB first (size 2)
 *
 */
void package_cmd(uint16_t cmd, uint8_t *tx);

#endif /* end header include protection */
