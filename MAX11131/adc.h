/** max11128_ex.h
 *
 * Header file for communicating with MAX11128 ADC
 * datasheet:https://datasheets.maximintegrated.com/en/ds/MAX11129-MAX11132.pdf
 *
 */
#ifndef adc_h
#define adc_h
/* Includes */
#include "stdint.h"
#include "stdlib.h"
#include "stm32f4xx_hal_def.h" // SPI Typedef Header

/* Register Definitions */
#define ADC0                190 // all subsequent adcs increment from base by 1

// Register Identification Code
//
// datasheet definitions between on pg21
#define ADC_MODE_CNTL   0x0000; // 0b0          followed by 0s
#define ADC_CONFIG      0x8000; // 0b1000       followed by 0s
#define ADC_UNIPOLAR    0x8800; // 0b10001000   followed by 0s
#define ADC_BIPOLAR     0x9000; // 0b1001       followed by 0s
#define ADC_RANGE       0x9800; // 0b10011000   followed by 0s

/* Global Var Definitions */
// TODO: document what hardware setup needs to be for this library
//          Write up README talking about what 
typedef GPIO_ADC_Pinfo {
    int ADC_CS_ADDR[8];
    int ADC_CS_PORT[8];
    int ADC_EOC_ADDR[8];
    int ADC_EOC_PORT[8];
    int ADC_CNVST_ADDR[8];
    int ADC_CNVST_PORT[8];
} GPIO_ADC_Pinfo;

GPIO_ADC_Pinfo pinfo;

// Mode Control Scan Registers
enum SCAN_STATES{
    HOLD,
    MANUAL,
    REPEAT,
    STD_INT,
    STD_EXT,
    UPPER_INT,
    UPPER_EXT
}

/* Public Function Prototypes */

/**
 *  Initialize ADC hardware component 
 *  
 *  detailed documentation starts on datasheet pg21 
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object ADC is on
 *  @param pinfo        <GPIO_ADC_Pinfo*> contains adc pin defs, refer to def above
 *  @param num_adcs     <int>   number of adcs
 * 
 *  Note: assumes 8bit data framing on SPI
 */
void init_adc(SPI_HandleTypeDef* SPI_BUS, GPIO_ADC_Pinfo *pinfo, int num_adcs);

/**
 *  Reads adc range from a specified range of pins on adc
 *  
 *  general documentation starts on datasheet pg 21
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object ADC is on
 *  @param adc_num      <uint8_t> selected adc number
 *  @param ch_start     <uint8_t> start adc channel to read from, indexed from 0
 *  @param ch_num       <uint8_t> number of channels to read including ch_start
 * 
 */
void read_adc_range(SPI_HandleTypeDef* SPI_BUS, uint8_t adc_num, uint8_t ch_start, uint8_t ch_num);

/* Private Helper Functions */

uint16_t* read_adc(SPI_HandleTypeDef* SPI_BUS, uint8_t adcn);
void set_adc(uint8_t adcn, GPIO_PinState state);
void write_adc_reg(SPI_HandleTypeDef* SPI_BUS, uint8_t *tx, uint8_t *rx);
void read_adc_ch(SPI_HandleTypeDef* SPI_BUS, uint8_t *tx, uint8_t *rx);
void package_cmd(uint16_t cmd, uint8_t* tx);


#endif /* end header include protection */