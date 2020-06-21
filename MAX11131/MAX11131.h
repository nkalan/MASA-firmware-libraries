/** max11128_ex.h
 *
 * Header file for communicating with MAX11128 ADC
 * datasheet:https://datasheets.maximintegrated.com/en/ds/MAX11129-MAX11132.pdf
 *
 */
#ifndef MAX11131_h
#define MAX11131_h
/* Includes */
#include "stdint.h"
#include "stdlib.h"
#include "stm32f4xx_hal.h"

/* Register Definitions */
const uint32_t ADC0 = 190; // Other adcs increment by 1

// Register Identification Code
// datasheet definitions between on pg21
const uint16_t ADC_MODE_CNTL 	= 0x0000; // 0b0          followed by 0s
const uint16_t ADC_CONFIG 		= 0x8000; // 0b1000       followed by 0s
const uint16_t ADC_UNIPOLAR 	= 0x8800; // 0b10001000   followed by 0s
const uint16_t ADC_BIPOLAR 		= 0x9000; // 0b1001       followed by 0s
const uint16_t ADC_RANGE 		= 0x9800; // 0b10011000   followed by 0

/* Offset bits mapping adc channel numbers to bit number */
const uint8_t  ADC_CUSTOM_SCAN1_SUB	= 5;
const uint8_t  ADC_CUSTOM_SCAN0_ADD	= 3;

/* Register bits for ADC Mode Control registers */
const uint16_t SET_SWCNV = 0x0002; //0b10           preceded by 0s

/* Register bits for ADC configuration registers */
const uint16_t SET_ADC_AVGON = 0x0200; // 0b1000000000 preceded by 0s

/* Register bits for ADC scan registers */
const uint16_t ADC_CUSTOM_SCAN0 = 0xA000; // 0b10100      followed by 0s
const uint16_t ADC_CUSTOM_SCAN1 = 0xA800; // 0b10101      followed by 0s

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

GPIO_ADC_Pinfo pinfo;

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
 *  @param pinfo        <GPIO_ADC_Pinfo*>   contains adc pin defs,
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
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object ADC is on
 *  @param adc_num      <uint8_t> selected adc number
 *
 */
uint16_t* read_adc_range(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn,
		uint8_t *channels, uint8_t ch_num);

/**
 *  Sets range to read from adc
 *
 *  general documentation starts on datasheet pg 21
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object ADC is on
 *  @param adcn         <uint8_t> selected adc number
 *  @param channels     <uint8_t*> arr of adc channels to read from
 *  @param ch_num       <uint8_t> number of channels to read
 *
 */
void set_read_adc_range(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn,
		uint8_t *channels, uint8_t ch_num);

/* Private Helper Functions */

/**
 *  Convenience function for reading from all channels on ADCx
 *  Note: this function assumes adc range to read from has already
 *          been set by set_read_adc_range()
 *
 *  general documentation starts on datasheet pg 21
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object ADC is on
 *  @param adcn         <uint8_t> selected adc number
 *
 */
uint16_t* read_adc(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn);

/**
 *  Selects/Disables ADC for SPI transmissions
 *
 *  @param adcn         <uint8_t> adc number
 *  @param state        <GPIO_PinState> state of GPIO PIN
 *                            Note:
 */
void set_adc(uint8_t adcn, GPIO_PinState state);
void write_adc_reg(SPI_HandleTypeDef *SPI_BUS, uint8_t *tx, uint8_t *rx);
void read_adc_ch(SPI_HandleTypeDef *SPI_BUS, uint8_t *tx, uint8_t *rx);
void package_cmd(uint16_t cmd, uint8_t *tx);

#endif /* end header include protection */
