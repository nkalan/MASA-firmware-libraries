#include "adc.h"

void init_adc(SPI_HandleTypeDef* SPI_BUS, int* adc_cs_addr, int* adc_cs_port, int num_adcs) {
    // Configure settings for all ADCs
    uint8_t tx[2];
	uint8_t rx[2];

    // disable interrupts until configurations are done
	__disable_irq();
    
    uint8_t adcn = 0;
    // Copy all adc CS addresses
    uint16_t adc_cmd    = 0;
    for(int i = 0; i < num_adcs; ++i) {
        ADC_CS_ADDR[i]  = adc_cs_addr[i];
        ADC_CS_PORT[i]  = adc_cs_addr[i];
        // Configure each ADC identically
        adcn    = ADC0 + i;
        adc_pin = adc_cs_addr[i];
        adc_port= adc_cs_port[i];

        // Sets ADC conversions to be written to FIFO register
        set_adc(adc_pin, adc_port, GPIO_PIN_RESET); // Select adc
        package_cmd(ADC_MODE_CNTL|MC_SCAN_STD_INT, tx); // prepare cmd
        write_adc_reg(SPI_BUS, tx, rx, 2); // Select register of adc
        set_adc(adc_pin, adc_port, GPIO_PIN_SET); // Release adc
    }
}

void write_adc_reg(SPI_HandleTypeDef* SPI_BUS, uint8_t *tx, uint8_t *rx) {
    if(HAL_SPI_TransmitReceive(SPI_BUS, tx, rx, 2, 1) ==  HAL_TIMEOUT){
	}
}

void package_cmd(uint16_t cmd, uint8_t* tx) {
    tx[0] = cmd & 0xff00;
    tx[1] = cmd & 0x00ff;
}

void set_adc(uint8_t adc_cs, uint8_t adc_port, GPIO_PinState state) {
    HAL_GPIO_WritePin(adc_port, adc_cs, state);
}