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
#define ADC0	190; // Other adcs increment by 1

// Register Identification Code
// datasheet definitions between on pg21
#define ADC_MODE_CNTL	(uint16_t) 0x0000 // 0b0          followed by 0s
#define ADC_CONFIG 		(uint16_t) 0x8000 // 0b1000       followed by 0s
#define ADC_UNIPOLAR 	(uint16_t) 0x8800 // 0b10001000   followed by 0s
#define ADC_BIPOLAR 	(uint16_t) 0x9000 // 0b1001       followed by 0s
#define ADC_RANGE 		(uint16_t) 0x9800 // 0b10011000   followed by 0

/* Offset bits mapping adc channel numbers to bit number */
#define ADC_CUSTOM_SCAN1_SUB	(uint8_t) 5
#define ADC_CUSTOM_SCAN0_ADD	(uint8_t) 3

/* Register bits for adc Mode Control registers */
#define SET_SWCNV 			(uint16_t) 0x0002 //0b10  preceded by 0s
#define SET_CHAN_ID			(uint16_t) 0x0004 // 0b100 preceded by 0s

/* Register bits for adc configuration registers */
#define SET_ADC_AVGON 		(uint16_t) 0x0200 // 0b10 0000 0000 preceded by 0s
#define SET_ADC_ECHO_ON		(uint16_t) 0x0004 // 0b0100		  preceded by 0s

/* Register bits for adc scan registers */
#define ADC_CUSTOM_SCAN0 	(uint16_t) 0xA000 // 0b10100      followed by 0s
#define ADC_CUSTOM_SCAN1 	(uint16_t) 0xA800 // 0b10101      followed by 0s
#define ADC_CUSTOM_SCAN_ALL_0 (uint16_t) 0x01F8 // 0b00111111000
#define ADC_CUSTOM_SCAN_ALL_1 (uint16_t) 0x07F8 // 0b11111111000

/* Channel size in FIFO register */
#define ADC_CHANNEL_SZ		(uint8_t)  0x0002

/* Global Var Definitions */

// GPIO pinout memory addresses
typedef struct GPIO_ADC_Pinfo {
	GPIO_TypeDef* ADC_CS_PORT[8];
	GPIO_TypeDef* ADC_EOC_PORT[8];
	GPIO_TypeDef* ADC_CNVST_PORT[8];
	uint16_t ADC_CS_ADDR[8];
	uint16_t ADC_EOC_ADDR[8];
	uint16_t ADC_CNVST_ADDR[8];
} GPIO_ADC_Pinfo;

GPIO_ADC_Pinfo *pinfo;

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
 *  @param pinfo        <GPIO_ADC_Pinfo*>   contains ADC pin defs,
 *                                          refer to def above
 *  @param num_adcs     <int>   number of adcs
 *
 *  Note: assumes 8bit data framing on SPI
 */
void init_adc(SPI_HandleTypeDef* SPI_BUS, GPIO_ADC_Pinfo *pins,
					int num_adcs);

/**
 *  Reads adc range from a specified range of pins on adc and
 *  returns arr of read values
 *
 *  general documentation starts on datasheet pg 21
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object adc is on
 *  @param adc_num      <uint8_t> selected adc number
 *
 */
void read_adc_range(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn,
		uint16_t* adc_out, uint8_t ch_num);

/**
 *  Sets range to read from adc
 *
 *  general documentation starts on datasheet pg 21
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object adc is on
 *  @param adcn         <uint8_t> selected adc number
 *  @param channels     <uint8_t*> arr of adc channels to read from
 *  @param ch_num       <uint8_t> number of channels to read
 *
 */
void set_read_adc_range(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn,
		uint8_t *channels, uint8_t ch_num);

/* Private Helper Functions */

/**
 *  Convenience function for reading from all channels on adcx
 *  Note: this function assumes adc range to read from has already
 *          been set by set_read_adc_range()
 *
 *  general documentation starts on datasheet pg 21
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object adc is on
 *  @param adcn         <uint8_t> selected adc number
 *
 */
uint16_t* read_adc(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn);

/**
 *  Selects/Disables adc for SPI transmissions
 *
 *  @param adcn         <uint8_t> adc number
 *  @param state        <GPIO_PinState> state of GPIO PIN
 *                            Note:
 */
void set_adc(uint8_t adcn, GPIO_PinState state);

/**
 * 	Convenience function for reading all adc channels for adc n
 *
 * 	@param SPI_BUS		<SPI_HandleTypeDef>	SPI object adc is on
 * 	@param adcn			<uint8_t> selected adc number
 *
 * 	Note: adc should be configured to update all SPI channels prior
 * 			to function call
 */
void read_adc_all(SPI_HandleTypeDef *SPI_BUS, uint16_t* adc_out, uint8_t adcn);

/**
 * 	Convenience function for setting adc to update all channels
 *
 * 	@param SPI_BUS		<SPI_HandleTypeDef>	SPI object adc is on
 * 	@param adcn			<uint8_t> selected adc number
 */
void set_read_adc_all(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn);

void write_adc_reg(SPI_HandleTypeDef *SPI_BUS, uint8_t *tx, uint8_t *rx);
void read_adc_ch(SPI_HandleTypeDef *SPI_BUS, uint8_t *tx, uint8_t *rx);
void package_cmd(uint16_t cmd, uint8_t *tx);

#endif /* end header include protection */
