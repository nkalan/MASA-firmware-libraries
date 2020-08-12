# MS5607-02BA Barometric Pressure Sensor / Altimeter
[Link to datasheet](https://www.te.com/commerce/DocumentDelivery/DDEController?Action=srchrtrv&DocNm=MS5607-02BA03&DocType=Data+Sheet&DocLang=English)

This repository contains the MASA firmware library for the MS5607 altimeter. The altimeter supplies pressure and temperature, which is then converted to altitude according to the 1976 U.S. Standard Atmosphere model.

* Operating pressure range: 10 mbar to 1200 mbar
* Operating temperature range: -40 dg C to 85 dg C

## Example Code & Implementation Notes
Create an `MS5607_Altimeter` struct in the main code, and call these functions in the following order, with the specified delays.

```
MS5607_Altimeter altimeter;
MS5607_altimeter_init(&altimeter, ...)

while(1) {
    MS5607_first_conversion(&altimeter)
    delay MS5607_get_adc_conversion_time(&altimeter)
    MS5607_second_conversion(&altimeter)
    delay MS5607_get_adc_conversion_time(&altimeter)
    
    int altitude = MS5607_calculate_altitude(altimeter);
}
```

For debugging, the function `MS5607_calculate_pressure_and_temperature()` can be used to directly access pressure and temperature. The following lines should be substituted or added in the same general location in the code where `MS5607_calculate_altitude()` is called.

```
int32_t pressure, temperature;
MS5607_calculate_pressure_and_temperature(&altimeter, &pressure, &temperature);
```

A pointer to the `MS5607_Altimeter` struct must be passed to each function along with any other necessary parameters. The delays should be implemented with STM32 Timers.

## SPI Settings
* Frame Format: Motorola
* Data Size: 8 bits
* First Bit: MSB First
* Clock Polarity: Low
* Clock Phase: 1 Edge

The maximum SPI clock frequency supported by the MS5607 is 20 MHz. For extra caution, setting it to 15 MHz or below is acceptable.
The MS5607 can operate in SPI mode 0 (clock idle low, rising edge) or SPI mode 3 (clock idle high, falling edge). These can be configured in STM32CubeMX or STM32CubeIDE under Connectivity>SPI>Parameter Settings.