#include "adc.h"

void init_adc(SPI_HandleTypeDef* SPI_BUS, GPIO_ADC_Pinfo *pins, int num_adcs) {
    // Configure settings for all ADCs
    uint8_t tx[2];
	uint8_t rx[2];
    pinfo = pins; // assign gpio pins for adcs

    uint8_t adcn = 0;
    uint16_t adc_cmd    = 0;
    uint16_t adc_pin, adc_port = 0;

	__disable_irq();
    
    for(int i = 0; i < num_adcs; ++i) {
        adc_pin = pinfo.ADC_CS_ADDR[i];
        adc_port= pinfo.ADC_CS_POST[i];

        // Sets ADC conversions to be written to FIFO register 
        set_adc(adc_pin, adc_port, GPIO_PIN_RESET);
        package_cmd(ADC_MODE_CNTL|SCAN_STATES.STD_INT, tx);
        write_adc_reg(SPI_BUS, tx, rx);
        set_adc(adc_pin, adc_port, GPIO_PIN_SET);
    }

    // Set ADC CNVST low here to guarantee its held for at least 5 ns
    HAL_GPIO_WritePin(  pinfo.ADC_CNVST_PORT[adcn], 
                        pinfo.ADC_CNVST_ADDR[adcn], 
                        GPIO_PIN_RESET);
    __enable_irq();
}

uint16_t* read_adc_range(SPI_HandleTypeDef* SPI_BUS, suint8_t adcn, 
                            uint8_t *channels, uint8_t ch_num) {
    /*
        Read ADC Procedure (pg17): 
            1.  Toggle CS on, note this is done instead of setting ~CNVST 
                because conversions are intiated on rising edge of ~CS with 
                SWCNV bit is set
            2.  When ADC read of all requested channels is complete, the EOC 
                pin is pulled low and results are available in FIFO
            3.  SPI Transmit each channel to ADC and store into full_rx
            // TODO modify below code to better suite behavior stated in step 1
    */
    uint16_t full_rx[16] = {-1};

    __disable_irq();

    HAL_GPIO_WritePin(  pinfo.ADC_CNVST_PORT[adcn], 
                        pinfo.ADC_CNVST_ADDR[adcn], 
                        GPIO_PIN_RESET
                        );

    while(HAL_GPIO_ReadPin(pinfo.ADC_EOC_PORT[adcn], pinfo.ADC_EOC_ADDR[adcn]))
    {}
    set_adc(pinfo.ADC_CS_PORT[adcn], pinfo.ADC_CS_ADDR[adcn], GPIO_PIN_RESET);

    uint8_t tx[2];
    uint8_t rx[2];
    for(uint8_t i = 0; i < ch_num; ++i) {
        uint8_t ch  = channels[i];

        tx[0] = (ch >> 1) | 0b00000000; // gets 3 MSB from channel num
        tx[1] = (ch << 7) | 0b00000000; // gets LSB from channel num
        // Read analog in data from FIFO channel
        read_adc_ch(SPI_BUS, &tx, &rx);
        // MSB 4 bits is simply ch addr, LSB 12 bits is adc data
        full_rx[ch] = (rx[1] | (rx[0]<<8)) & 0x0FFF;
    }

    set_adc(pinfo.ADC_CS_PORT[adcn], pinfo.ADC_CS_ADDR[adcn], GPIO_PIN_SET);

    HAL_GPIO_WritePin(  pinfo.ADC_CNVST_PORT[adcn], 
                        pinfo.ADC_CNVST_ADDR[adcn], 
                        GPIO_PIN_SET); // prep ADC for next read

    __enable_irq();

    return full_rx;
}


void set_read_adc_range(SPI_HandleTypeDef* SPI_BUS, uint8_t adcn, 
                            uint8_t *channels, uint8_t ch_num) {
    /*
        Configure Custom ADC read procedure (pg32-33):
            1. Set AVGON BIT reg to 1
            2. Set NAVG to desired number of samples (default 4 samples)
            3. Set CUSTOM Scan0 and CUSTOM Scan1 Registers
            4. Set ADC MODE CONTROL REGISTER SET SCAN[3:0] 0b0111
            5. Set CHSEL[3:0] to Channel Number (Not needed for CUSTOM INT)
            6. Set SWCNV bit to 1 to enable conversions with chip select
    */
    uint8_t tx[2];
	uint8_t rx[2];

    uint16_t SET_AVG_ON_BIT         = ADC_CONFIG|SET_ADC_AVGON;
    uint16_t SET_ADC_MODE_CNTL      = ADC_MODE_CNTL|SCAN_STATES.CUSTOM_INT;
    uint16_t SET_CS_CHIP_CONV       = ADC_MODE_CNTL|SET_SWCNV;
    
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
    uint16_t cs_pin    = pinfo.ADC_CS_ADDR[adcn];
    uint16_t cs_port   = pinfo.ADC_CS_PORT[adcn];

    /* Set ADC to custom scan channel range */
    __disable_irq();

    /* Transmit custom channels to send data from */
    set_adc(cs_pin, cs_port, GPIO_PIN_RESET);
    package_cmd(SET_AVG_ON_BIT, tx);
    write_adc_reg(SPI_BUS, tx, rx);
    package_cmd(SET_SCAN_REGISTER_1, tx);
    write_adc_reg(SPI_BUS, tx, rx);
    package_cmd(SET_SCAN_REGISTER_0, tx);
    write_adc_reg(SPI_BUS, tx, rx);
    package_cmd(SET_ADC_MODE_CNTL, tx);
    write_adc_reg(SPI_BUS, tx, rx);
    package_cmd(SET_CS_CHIP_CONV, tx);
    write_adc_reg(SPI_BUS, tx, rx);
    set_adc(cs_pin, cs_port, GPIO_PIN_SET);

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
    if(HAL_SPI_TransmitReceive(SPI_BUS, tx, rx, 2, 1) ==  HAL_TIMEOUT){}
}

void package_cmd(uint16_t cmd, uint8_t* tx) {
    tx[0] = cmd & 0xff00;
    tx[1] = cmd & 0x00ff;
}

void set_adc(uint8_t adcn, GPIO_PinState state) {
    HAL_GPIO_WritePin(ADC_CS_PORT[adcn], ADC_CS_ADDR[adcn], state);
}