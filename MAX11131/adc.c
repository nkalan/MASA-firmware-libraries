#include "adc.h"

void init_adc(SPI_HandleTypeDef* SPI_BUS, GPIO_ADC_Pinfo *pins, int num_adcs) {
    // Configure settings for all ADCs
    uint8_t tx[2];
	uint8_t rx[2];
    pinfo = pins; // assign gpio pins for adcs

    // disable interrupts until configurations are done
	__disable_irq();
    
    uint8_t adcn = 0;
    // Copy all adc CS addresses
    uint16_t adc_cmd    = 0;
    for(int i = 0; i < num_adcs; ++i) {
        // Configure each ADC identically
        adc_pin = pinfo.ADC_CS_ADDR[i];
        adc_port= pinfo.ADC_CS_POST[i];

        // Sets ADC conversions to be written to FIFO register
        set_adc(adc_pin, adc_port, GPIO_PIN_RESET); // Select adc
        package_cmd(ADC_MODE_CNTL|SCAN_STATES.STD_INT, tx); // prepare cmd
        write_adc_reg(SPI_BUS, tx, rx, 2); // Select register of adc
        set_adc(adc_pin, adc_port, GPIO_PIN_SET); // Release adc
    }
    __enable_irq();
}

uint16_t* read_adc(SPI_HandleTypeDef* SPI_BUS, uint8_t adcn) {
    // Reads for entire FIFO register on ADC
    uint16_t full_rx[16];

    // start ADC from correct base addr
    adcn    = ADC0 + adcn;

    __disable_irq();
    // ~CS should be high, Set ~CNVST low before pulling high
    HAL_GPIO_WritePin(  pinfo.ADC_CNVST_PORT[adcn], 
                        pinfo.ADC_CNVST_ADDR[adcn], 
                        GPIO_PIN_RESET);
    // Wait for EOC to pull low, then set ~CS low 
    while(HAL_GPIO_ReadPin(pinfo.ADC_EOC_PORT[adcn], pinfo.ADC_EOC_ADDR[adcn])){}
    set_adc(pinfo.ADC_CS_PORT[adcn], pinfo.ADC_CS_ADDR[adcn], GPIO_PIN_RESET);

    uint8_t tx[2];
    uint8_t rx[2];
    for(uint8_t ch = 0; ch < 16; ++ch) {
        //0b00001 sets that next channel to be selected is identified
		// in each SPI frame, conversion results are sent out next frame
		tx[0] = (ch >> 1) | 0b00000000; // gets 3 MSB from channel num
		tx[1] = (ch << 7) | 0b00000000; // gets LSB from channel num
        // Read analog in data from FIFO channels
        read_adc_ch(SPI_BUS, &tx, &rx);
        full_rx[ch] = (rx[1] | (rx[0]<<8)) & 0x0FFF; // MSB 4 bits is simply ch addr, LSB 12 bits is adc data
    }

    // Deselect ADC once reading finishes
    set_adc(pinfo.ADC_CS_PORT[adcn], pinfo.ADC_CS_ADDR[adcn], GPIO_PIN_SET);

    // Reenable Interrupts
    __disable_irq();

    // return adc data from all channels
    return &full_rx;
}

void read_adc_ch(SPI_HandleTypeDef* SPI_BUS, uint8_t *tx, uint8_t *rx) {
    if(HAL_SPI_TransmitReceive(SPI_BUS, tx, rx, 2, 1) == HAL_TIMEOUT){}
}

void write_adc_reg(SPI_HandleTypeDef* SPI_BUS, uint8_t *tx, uint8_t *rx) {
    if(HAL_SPI_TransmitReceive(SPI_BUS, tx, rx, 2, 1) ==  HAL_TIMEOUT){
	}
}

void package_cmd(uint16_t cmd, uint8_t* tx) {
    tx[0] = cmd & 0xff00;
    tx[1] = cmd & 0x00ff;
}

void set_adc(uint8_t adcn, GPIO_PinState state) {
    HAL_GPIO_WritePin(ADC_CS_PORT[adcn], ADC_CS_ADDR[adcn], state);
}