#include "../inc/MAX11131.h"

void init_adc(SPI_HandleTypeDef *SPI_BUS, GPIO_ADC_Pinfo *pins, int num_adcs) {
	/*
	 * 	Automatically configures ADC to read from custom internal channels
	 * 	Steps:
	 * 	ADC configuration register
	 * 		1.	Set AVG_ON bit to 1
	 * 		2.	Set NAVG[1:0]  to N
	 *	ADC custom scan registers
	 * 		3. Set CUSTOM_SCAN0 register
	 * 		4. Set CUSTOM_SCAN1 register
	 * 	ADC mode control register
	 * 		5. Set SCAN[3:0] to 0111
	 * 		6. Set CHSEL[3:0] to channel number (not needed for Custom int)
	 * 		7. Select the right SWCNV bit
	 *
	 *
	 */
	// Configure settings for all ADCs
	uint8_t tx[2] = {0, 0};
	uint8_t rx[2] = {0, 0};
	pinfo = pins;

	// 	note: these types are taken from the GPIO_TypeDef line 486
	//			in file stm32f446x.h
	uint8_t adcn;

	// Generate adc config data
	uint16_t ADC_CONFIG_REG			= ADC_CONFIG|SET_ADC_AVGON;
	uint16_t ADC_CUSTOM_SCAN0_REG	= ADC_CUSTOM_SCAN0|ADC_CUSTOM_SCAN_ALL_0;
	uint16_t ADC_CUSTOM_SCAN1_REG	= ADC_CUSTOM_SCAN1|ADC_CUSTOM_SCAN_ALL_1;
	uint16_t ADC_MODE_CNTL_REG 		= ADC_MODE_CNTL|(CUSTOM_INT<<11);

	__disable_irq();
	for (adcn = 0; adcn < num_adcs; ++adcn) {
		set_adc(adcn, GPIO_PIN_RESET);
		package_cmd(ADC_CUSTOM_SCAN0_REG, tx);
		//write_adc_reg(SPI_BUS, tx, rx);
		if (HAL_SPI_Transmit(SPI_BUS, tx, 2, 1) == HAL_TIMEOUT) {}
		set_adc(adcn, GPIO_PIN_SET);

		set_adc(adcn, GPIO_PIN_RESET);
		package_cmd(ADC_CUSTOM_SCAN1_REG, tx);
		//write_adc_reg(SPI_BUS, tx, rx);
		if (HAL_SPI_Transmit(SPI_BUS, tx, 2, 1) == HAL_TIMEOUT) {}
		set_adc(adcn, GPIO_PIN_SET);

		set_adc(adcn, GPIO_PIN_RESET);
		package_cmd(ADC_CONFIG_REG, tx);
		if (HAL_SPI_Transmit(SPI_BUS, tx, 2, 1) == HAL_TIMEOUT) {}
		//write_adc_reg(SPI_BUS, tx, rx);
		set_adc(adcn, GPIO_PIN_SET);

		set_adc(adcn, GPIO_PIN_RESET);
		package_cmd(ADC_MODE_CNTL_REG, tx);
		//write_adc_reg(SPI_BUS, tx, rx);
		if (HAL_SPI_Transmit(SPI_BUS, tx, 2, 1) == HAL_TIMEOUT) {}
		set_adc(adcn, GPIO_PIN_SET);

	} // program all adcs with custom int mode set to read all channels
	__enable_irq();
}

void read_adc_range(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn,
		uint16_t* adc_out, uint8_t ch_num) {
	/*
	 Read ADC Procedure for internal clock using SWCNV bit set(pg17):
	 	 1.	Set CS high to initiate conversions
	 	 2. Wait for EOC pin to be pulled low
	 	 3. Read from FIFO register on DOUT pin
	 	 4. Set CS high again

	 Read ADC Procedure for internal clock not using SWCNV bit
	 	 1. Set CS high
	 	 2. Set CNVST low for at least 5ns before pulling high again
	 	 3. Wait for EOC to be pulled low
	 	 4. Set CS low and High to initiate serial communications
	 		Note: EOC stays low until CS or CNVST is pulled low again
	 */
	/* ADC startup and FIFO register intialization */

	set_adc(adcn, GPIO_PIN_SET);
	HAL_GPIO_WritePin(pinfo->ADC_CNVST_PORT[adcn],
			pinfo->ADC_CNVST_ADDR[adcn], GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(pinfo->ADC_CNVST_PORT[adcn],
				pinfo->ADC_CNVST_ADDR[adcn], GPIO_PIN_SET);

	uint8_t pin_state = HAL_GPIO_ReadPin(pinfo->ADC_EOC_PORT[adcn],
					pinfo->ADC_EOC_ADDR[adcn]);

	while (pin_state != 0) {
		pin_state = HAL_GPIO_ReadPin(pinfo->ADC_EOC_PORT[adcn],
							pinfo->ADC_EOC_ADDR[adcn]);
	}

	/* Serial communications with ADC */
	__disable_irq();

	// The number of bytes in the FIFO is simply the
	// number of channels * 2 (bytes for each channel)
	uint8_t rx[2] = {0};
	uint8_t tx[2] = {0};

	for (uint8_t i = 0; i < ch_num; ++i) {
		set_adc(adcn, GPIO_PIN_RESET);
		if (HAL_SPI_TransmitReceive(SPI_BUS, tx, rx, 2, 1) == HAL_TIMEOUT) {}
		set_adc(adcn, GPIO_PIN_SET);

		uint16_t adc_counts = ((rx[0]<<8)|rx[1]) & 0x0FFF;
		uint16_t channelId = (rx[0] >> 4) & 0x0F;
		adc_out[channelId] = adc_counts;
	}

	__enable_irq();
}

void set_read_adc_range(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn,
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

	uint16_t SET_AVG_ON_BIT = ADC_CONFIG | SET_ADC_AVGON;
	uint16_t SET_ADC_MODE_CNTL = ADC_MODE_CNTL | CUSTOM_INT;

	uint16_t SET_SCAN_REGISTER_0 = ADC_CUSTOM_SCAN0;
	uint16_t SET_SCAN_REGISTER_1 = ADC_CUSTOM_SCAN1;
	for (uint8_t i = 0; i < ch_num; ++i) {
		uint8_t ch = channels[i];
		if (ch > 7) {
			ch -= ADC_CUSTOM_SCAN1_SUB;
			SET_SCAN_REGISTER_1 = SET_SCAN_REGISTER_1 | (1 << ch);
		} else {
			ch += ADC_CUSTOM_SCAN0_ADD;
			SET_SCAN_REGISTER_0 = SET_SCAN_REGISTER_0 | (1 << ch);
		} // sets channel register bit in Custom Scan0 Register
	}

	/* Set ADC to custom scan channel range */
	__disable_irq();

	/* Transmit custom channels to send data from */
	set_adc(adcn, GPIO_PIN_RESET);
	package_cmd(SET_AVG_ON_BIT, tx);
	write_adc_reg(SPI_BUS, tx, rx);
	package_cmd(SET_SCAN_REGISTER_0, tx); // doesn't work on this cmd
	write_adc_reg(SPI_BUS, tx, rx);
	package_cmd(SET_SCAN_REGISTER_1, tx);
	write_adc_reg(SPI_BUS, tx, rx);
	package_cmd(SET_ADC_MODE_CNTL, tx);
	write_adc_reg(SPI_BUS, tx, rx);
	set_adc(adcn, GPIO_PIN_SET);

	__enable_irq();
}

void set_read_adc_all(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn) {
	// Convenience function for reading all channels on adc
	uint8_t channels[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
			15 };
	uint8_t ch_num = 16;
	return set_read_adc_range(SPI_BUS, adcn, channels, ch_num);
}

void read_adc_all(SPI_HandleTypeDef *SPI_BUS, uint16_t* adc_out, uint8_t adcn) {
	// Convenience function for reading all channels on adc
	uint8_t ch_num = 16;
	read_adc_range(SPI_BUS, adcn, adc_out, ch_num);
}

void read_adc_ch(SPI_HandleTypeDef *SPI_BUS, uint8_t *tx, uint8_t *rx) {
	if (HAL_SPI_TransmitReceive(SPI_BUS, tx, rx, 2, 1) == HAL_TIMEOUT) {
	}
}

void write_adc_reg(SPI_HandleTypeDef *SPI_BUS, uint8_t *tx, uint8_t *rx) {
	if (HAL_SPI_TransmitReceive(SPI_BUS, tx, rx, 2, 1) == HAL_TIMEOUT) {
	}
}

void package_cmd(uint16_t cmd, uint8_t *tx) {
	tx[0] = (cmd >> 8) & 0x00ff;
	tx[1] = (cmd & 0x00ff);
}

void set_adc(uint8_t adcn, GPIO_PinState state) {
	HAL_GPIO_WritePin(pinfo->ADC_CS_PORT[adcn], pinfo->ADC_CS_ADDR[adcn], state);
}
