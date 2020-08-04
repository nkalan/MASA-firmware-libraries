# User Guide (LAST UPDATED: July 11, 2020)

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
    the underlying data structure can be found below. For instance, MAX31_CS_ADDR
    indicates that it is CS pin addr for an ADC.
3. To initialize multiple ADCs, create an array of `GPIO_MAX31_Pinfo` structs
    and manually assign each struct's pinouts.
4. For future reference, you must provide the pin data for each ADC everytime 
    you want to interface with the ADC.

```
typedef struct GPIO_MAX31_Pinfo {
	GPIO_TypeDef* MAX31_CS_PORT;		// PORT belonging to CS pin
	GPIO_TypeDef* MAX31_EOC_PORT;		// PORT belonging to EOC pin
	GPIO_TypeDef* MAX31_CNVST_PORT;		// PORT belonging to CNVST pin
	uint16_t MAX31_CS_ADDR;				// PIN belonging to CS pin
	uint16_t MAX31_EOC_ADDR;			// PIN belonging to EOC pin
	uint16_t MAX31_CNVST_ADDR;			// PIN belonging to CNVST pin

	uint8_t NUM_CHANNELS;				// Number of channels to read from
	uint8_t MAX31_CHANNELS[16];			// Channel Identification Numbers
} GPIO_MAX31_Pinfo;
```
3. Once ADC settings are configured, call `init_adc(SPI_HandleTypeDef* SPI_BUS, GPIO_MAX31_Pinfo *pins)` on each ADC struct, making sure to pass the correct `SPI BUS`, `GPIO_MAX31_Pinfo` configuration variable. This initializes, by default, channels 0-13.

4. To read current ADC values, call `read_adc(SPI_HandleTypeDef *SPI_BUS, GPIO_MAX31_Pinfo *pinfo, uint16_t* adc_out)`.

# Developer Guide

### Test Procedure (DO THIS EVERYTIME YOU EDIT THIS LIBRARY!)

To verify that adc channels 0-13 are working correctly in Custom Internal mode, 
repeat the following procedure below:

1. Setup a timer based interrupt that counts up and is triggered every 1-2 seconds.
2. In the timer interrupt handler, toggle as many output pins from the main
    microcontroller to each ADC_in pin. Ideally, adjacent pins should be configured
    to always have opposite states. In addition, I voltage divided some of the 
    pins and determined roughly what counts I should be expecting for those pins.
3. Initialize the microcontroller to read from channels 0-13 on the ADC. Additionally,
    it is helpful to initialize the ADC values array as a global var in order to
    view from the live expressions tab in STM32CubeIDE. More information on how
    to setup live expressions can be found at this [link.](https://www.youtube.com/watch?v=Nyml66k_Ppk)
4. Observe that each adc value in the array match what is expected. Note: the adc
    values are currently in counts, so some additional conversions may be needed.

For a brief example of what this implementation looks like on the Nucleo F446RE 
dev board, I have created an example program below that demonstrates the procedure
on adc pins 0-2. It is at this [Github Repo.](https://github.com/KingArthurZ3/MASA-firmware-dev) (Commit number: 2a15a8)

### Modifying the existing firmware

Most of the documentation for the exact details on how to configure each read mode should be documented in the firmware code. The README should only be for documenting project level configurations. In addition, detailed documentation
for this ADC can be found on the [technical datasheet.](https://datasheets.maximintegrated.com/en/ds/MAX11129-MAX11132.pdf)
