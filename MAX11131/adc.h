#ifndef adc_h
#define adc_h
/* Includes */
#include "stdint.h"
#include "stdlib.h"
#include "stm32f4xx_hal_def.h"

/* Register Definitions */
#define BIT_RES             4096
#define ADC0                190 // all subsequent adcs increment from base by 1
// Mode Control REgisters
#define MC_SCAN_STD_INT     0x3
#define MC_SCAN_STD_EXT     0x4

// Register Identification Code
#define ADC_MODE_CNTL   0x0000;
#define ADC_CONFIG      0x8000;
#define ADC_UNIPOLAR    0x8800;
#define ADC_BIPOLAR     0x9000;
#define ADC_RANGE       0x9800;

/* Global Var Definitions */
int ADC_CS_ADDR[8];
int ADC_CS_PORT[8];

/* Public Function Prototypes */
void init_adc(SPI_HandleTypeDef* SPI_BUS, int* adc_cs_addr, int* adc_cs_port, int num_adcs);

void read_adc_range(SPI_HandleTypeDef* SPI_BUS, uint8_t adc_num, uint8_t ch_start, uint8_t ch_num);

/* Private Helper Functions */

void set_adc(uint8_t adc_cs, uint8_t adc_port, GPIO_PinState state)
void write_adc_reg(SPI_HandleTypeDef* SPI_BUS, uint8_t *tx, uint8_t *rx);
void package_cmd(uint16_t cmd, uint8_t* tx);
#endif