# User Guide

### Configuring SPI settings

The maximum SPI frequency allowed for this ADC is 40 MHz. In the STM32CubeIde
SPI configuration, the configuration settings should be identical to those below. Note: the prescaler is not noted below but should be adjusted accordingly

* Frame Format: Motorola
* Data Size: 8 bits
* First Bit: MSB First
* Clock Polarity: High
* Clock Phase: 2 Edge

### Configuring ADC pins

For this ADC, we need to configure three pins on the microcontroller: EOC, CNVST, and CS. These pins may be on any GPIO pin, but they must be configured exactly as shown below.

CS: (Output pin)
* GPIO output level: High
* GPIO mode: Output Push Pull
* GPIO Pull-up/Pull-down: None
* Maximum output speed: High

CNVST: (Output pin)
* GPIO output level: High
* GPIO mode: Output Push Pull
* GPIO Pull-up/Pull-down: None
* Maximum output speed: High

EOC: (Input pin)
* GPIO mode: Input mode
* GPIO Pull-up/Pull-down: None

### Configuring Hardware Pinouts
1. Initialize `GPIO_MAX31_Pinfo` struct, this is your main interface for 
    configuring ADC hardware pins and which channels to read from
2. Manually assign pins in `GPIO_MAX31_Pinfo` to hardware pins. A snippet of 
    the underlying data structure can be found below. Each index in the member
    variable identifies which adc it belongs to. For instance, MAX31_CS_ADDR[0]
    indicates that it is CS pin addr for ADC 0.
    Note: the maximum number of ADCs currently supported at once is 8 (arbitrary)

```
typedef struct GPIO_MAX31_Pinfo {
	GPIO_TypeDef* MAX31_CS_PORT[8];		// PORT belonging to CS pin
	GPIO_TypeDef* MAX31_EOC_PORT[8];	// PORT belonging to EOC pin
	GPIO_TypeDef* MAX31_CNVST_PORT[8];	// PORT belonging to CNVST pin
	uint16_t MAX31_CS_ADDR[8];			// PIN belonging to CS pin
	uint16_t MAX31_EOC_ADDR[8];			// PIN belonging to EOC pin
	uint16_t MAX31_CNVST_ADDR[8];		// PIN belonging to CNVST pin
	uint8_t NUM_ADCS;					// Number of ADCs

	uint8_t NUM_CHANNELS[8];				// Number of channels to read from
	uint8_t MAX31_CHANNELS[8][16];			// Channel Identification Numbers
} GPIO_MAX31_Pinfo;
```
3. Once ADC settings are configured, call `init_adc(SPI_HandleTypeDef* SPI_BUS, GPIO_MAX31_Pinfo *pins)`, making sure to pass the correct `SPI BUS`, `GPIO_MAX31_Pinfo` configuration variable. This initializes, by default, channels 0-13.

4. To read current ADC values, call `read_adc(SPI_HandleTypeDef *SPI_BUS, uint8_t adcn, uint16_t* adc_out)`.

# Developer Guide

Most of the documentation for the exact details on how to configure each read mode should be documented in the firmware code. The README should only be for documenting project level configurations. In addition, detailed documentation
for this ADC can be found on the technical datasheet: `https://datasheets.maximintegrated.com/en/ds/MAX11129-MAX11132.pdf`
