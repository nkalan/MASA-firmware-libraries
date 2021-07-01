# MAX31856 thermocouple library User Guide (Last Updated: Jun 30 2021)
This repository contains the MASA firmware library for the MAX31856  thermocouple to digital converter.

[MAX31856 Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX31856.pdf)

[Dev board datasheet (includes soldering + setup instructions)](https://cdn-learn.adafruit.com/downloads/pdf/adafruit-max31856-thermocouple-amplifier.pdf)

## What is different about this library
The main difference between this firmware library and others is that previous libraries assumed that each TC chip had its own chip select (CS) pin connected to the microcontroller, but the flight EC will instead have a 4-bit decoder that can select 1 TC chip at a time, and the nosecone recovery board will have 2 TCs each with their own CS pin connected to the micro. This library should support both boards, so chip select and chip release have been abstracted to be function pointers, with the thermocouple index as an input. That way, the flight EC can implement its own GPIO MUX logic, and the nosecone board can implement a simpler GPIO toggle for chip select.

[Old MAX31855 TC library](https://gitlab.eecs.umich.edu/masa/avionics/firmware-libraries/-/tree/feature-MAX31856-dev/MAX31855)
The difference between this library and the MAX31855 library is that this library uses an internal look up table (LUT) for linearization.

## SPI configuration
* Frame Format: Motorola
* Data Size: 8 bits
* First Bit: MSB First
* Maximum Serial Clock Frequency: 5MHz

## Testing
When testing with a nucleo board, we need to implement the chip select and chip release functions. Because the nucleo board acts nomal (like the nosecone recovery board), chip select and chip release will just write the CS pin. 


