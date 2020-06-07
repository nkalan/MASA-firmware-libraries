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
// datasheet definitions between on pg21
#define ADC_MODE_CNTL               0x0000; // 0b0          followed by 0s
#define ADC_CONFIG                  0x8000; // 0b1000       followed by 0s
#define ADC_UNIPOLAR                0x8800; // 0b10001000   followed by 0s
#define ADC_BIPOLAR                 0x9000; // 0b1001       followed by 0s
#define ADC_RANGE                   0x9800; // 0b10011000   followed by 0

/* Offsets base channel addr to modify correct scan register bit */
#define ADC_CUSTOM_SCAN0_ADD        0x0003; // 0b11         preceded by 1s
#define ADC_CUSTOM_SCAN1_SUB        0x0005; // 0b101        preceded by 1s
/* Register bits used to configure ADC scan registers */
#define ADC_CUSTOM_SCAN0            0xA000; // 0b10100      followed by 0s
#define ADC_CUSTOM_SCAN1            0xA800; // 0b10101      followed by 0s

/* Global Var Definitions */
// TODO: document what hardware setup needs to be for this library
//          Write up README talking about what 
typedef GPIO_ADC_Pinfo {
    uint16_t ADC_CS_ADDR[8];
    uint16_t ADC_CS_PORT[8];
    uint16_t ADC_EOC_ADDR[8];
    uint16_t ADC_EOC_PORT[8];
    uint16_t ADC_CNVST_ADDR[8];
    uint16_t ADC_CNVST_PORT[8];
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
    UPPER_EXT,
    CUSTOM_INT,
    CUSTOM_EXT
}

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
void init_adc(SPI_HandleTypeDef* SPI_BUS, GPIO_ADC_Pinfo *pinfo, 
                    uint8_t num_adcs);

/**
 *  Reads adc range from a specified range of pins on adc and 
 *  returns arr of read values
 *  
 *  general documentation starts on datasheet pg 21
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object ADC is on
 *  @param adc_num      <uint8_t> selected adc number
 * 
 */
uint16_t* read_adc_range(SPI_HandleTypeDef* SPI_BUS, uint8_t adcn);

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
void set_read_adc_range(SPI_HandleTypeDef* SPI_BUS, uint8_t adcn, 
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
uint16_t* read_adc(SPI_HandleTypeDef* SPI_BUS, uint8_t adcn);

/**
 *  
 */
void set_adc(uint8_t adcn, GPIO_PinState state);
void write_adc_reg(SPI_HandleTypeDef* SPI_BUS, uint8_t *tx, uint8_t *rx);
void read_adc_ch(SPI_HandleTypeDef* SPI_BUS, uint8_t *tx, uint8_t *rx);
void package_cmd(uint16_t cmd, uint8_t* tx);



#endif /* end header include protection */