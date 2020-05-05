#ifndef adc_h
#define adc_h
/* Includes */
#include "stdint.h"
#include "stdlib.h"
#include "stm32f4xx_hal_def.h"

/* TODO Hardware Pin Definitions */
#define ADC_EOC_PORT
#define ADC_EOC_ADDR
#define ADC_CNVST_

/* Register Definitions */
#define BIT_RES             4096
#define ADC0                190 // all subsequent adcs increment from base by 1

// Register Identification Code
#define ADC_MODE_CNTL   0x0000;
#define ADC_CONFIG      0x8000;
#define ADC_UNIPOLAR    0x8800;
#define ADC_BIPOLAR     0x9000;
#define ADC_RANGE       0x9800;

/* Global Var Definitions */

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
void init_adc(SPI_HandleTypeDef* SPI_BUS, GPIO_ADC_Pinfo *pinfo, int num_adcs);

void read_adc_range(SPI_HandleTypeDef* SPI_BUS, uint8_t adc_num, uint8_t ch_start, uint8_t ch_num);

/* Private Helper Functions */

uint16_t* read_adc(SPI_HandleTypeDef* SPI_BUS, uint8_t adcn);
void set_adc(uint8_t adcn, GPIO_PinState state);
void write_adc_reg(SPI_HandleTypeDef* SPI_BUS, uint8_t *tx, uint8_t *rx);
void read_adc_ch(SPI_HandleTypeDef* SPI_BUS, uint8_t *tx, uint8_t *rx);
void package_cmd(uint16_t cmd, uint8_t* tx);
#endif