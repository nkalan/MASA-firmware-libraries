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
    uint16_t adc_pin, adc_port = 0;
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

    // Set ADC CNVST low here to guarantee its held for at least 5 ns
    HAL_GPIO_WritePin(  pinfo.ADC_CNVST_PORT[adcn], 
                        pinfo.ADC_CNVST_ADDR[adcn], 
                        GPIO_PIN_RESET);
    __enable_irq();
}

uint16_t* read_adc_range(SPI_HandleTypeDef* SPI_BUS, uint8_t adcn, 
                            uint8_t *channels, uint8_t ch_num) {
    uint16_t full_rx[16] = {-1};

    __disable_irq();
    // ~CS should be high, Set ~CNVST low before pulling high after 5ns
    HAL_GPIO_WritePin(  pinfo.ADC_CNVST_PORT[adcn], 
                        pinfo.ADC_CNVST_ADDR[adcn], 
                        GPIO_PIN_SET
                        );
    // Wait for EOC to pull low, then set ~CS low 
    while(HAL_GPIO_ReadPin(pinfo.ADC_EOC_PORT[adcn], pinfo.ADC_EOC_ADDR[adcn])){}
    set_adc(pinfo.ADC_CS_PORT[adcn], pinfo.ADC_CS_ADDR[adcn], GPIO_PIN_RESET);

    uint8_t tx[2];
    uint8_t rx[2];
    for(uint8_t i = 0; i < ch_num; ++i) {
        uint8_t ch  = channels[i];
        //0b00001 sets that next channel to be selected is identified
        // in each SPI frame, conversion results are sent out next frame
        tx[0] = (ch >> 1) | 0b00000000; // gets 3 MSB from channel num
        tx[1] = (ch << 7) | 0b00000000; // gets LSB from channel num
        // Read analog in data from FIFO channel
        read_adc_ch(SPI_BUS, &tx, &rx);
        // MSB 4 bits is simply ch addr, LSB 12 bits is adc data
        full_rx[ch] = (rx[1] | (rx[0]<<8)) & 0x0FFF;
    }

    // Deselect ADC once reading finishes
    set_adc(pinfo.ADC_CS_PORT[adcn], pinfo.ADC_CS_ADDR[adcn], GPIO_PIN_SET);

    // Set CNVST low to guarantee hold for at least 5ns before next adc read
    HAL_GPIO_WritePin(  pinfo.ADC_CNVST_PORT[adcn], 
                        pinfo.ADC_CNVST_ADDR[adcn], 
                        GPIO_PIN_RESET);

    // Reenable Interrupts
    __enable_irq();

    // return adc data from all channels
    return full_rx;
}


void set_read_adc_range(SPI_HandleTypeDef* SPI_BUS, uint8_t adcn, 
                            uint8_t *channels, uint8_t ch_num) {
    uint8_t tx[2];
	uint8_t rx[2];

    /* Set ADC to custom scan channel range */
    __disable_irq();

    /* ~CS should be high, Set ~CNVST low before pulling high after 5ns */
    uint16_t SET_CUSTOM_SCAN        = ADC_MODE_CNTL|SCAN_STATES.CUSTOM_INT;
    
    uint16_t SET_SCAN_REGISTER_0    = ADC_CUSTOM_SCAN0;
    uint16_t SET_SCAN_REGISTER_1    = ADC_CUSTOM_SCAN1;
    for (uint8_t i = 0; i < ch_num; ++i) {
        uint8_t ch = channels[i];
        if (ch > 7) {
            ch -= ADC_CUSTOM_SCAN1_SUB;
            SET_SCAN_REGISTER_1 = SET_SCAN_REGISTER_1|(1<<ch);
        } else {
            ch += ADC_CUSTOM_SCAN0_ADD;
            SET_SCAN_REGISTER_0 = SET_SCAN_REGISTER_0|(1<<ch);
        } // sets channel register bit in Custom Scan0 Register
    }

    /* Configure ADC with channels */
    uint16_t adc_pin    = pinfo.ADC_CS_ADDR[adcn];
    uint16_t adc_port   = pinfo.ADC_CS_PORT[adcn];

    /* Transmit custom channels to send data from */
    set_adc(adc_pin, adc_port, GPIO_PIN_RESET); // Select adc
    package_cmd(SET_SCAN_REGISTER_1, tx);
    write_adc_reg(SPI_BUS, tx, rx, 2); // Select register of adc
    package_cmd(SET_SCAN_REGISTER_0, tx);
    write_adc_reg(SPI_BUS, tx, rx, 2); // Select register of adc
    set_adc(adc_pin, adc_port, GPIO_PIN_SET); // Release adc

    __enable_irq();
}

uint16_t* read_adc_all(SPI_HandleTypeDef* SPI_BUS, uint8_t adcn) {
    // Convenience function for reading all channels on adc
    uint8_t channels[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8
                            9, 10, 11, 12, 13, 14, 15} 
    uint8_t ch_num      = 16;
    return read_adc_range(SPI_BUS, adcn, channels, ch_num);
    
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