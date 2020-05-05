/** ms5607.h
 *
 * Header file for communicating with MS5607 Pressure Altimeter
 * datasheet: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5607-02BA03%7FB2%7Fpdf%7FEnglish%7FENG_DS_MS5607-02BA03_B2.pdf%7FCAT-BLPS0035
 *
 * From the datasheet: "The MS5607-02BA consists of a piezo-resistive sensor and a sensor interface IC.
 * The main function of the MS5607-02BA is to convert the uncompensated analogue output voltage from the piezo-resistive
 * pressure sensor to a 24-bit digital value, as well as providing a 24-bit digital value for the temperature of the sensor."
 * 
 * Nathaniel Kalantar (nkalan@umich.edu)
 * Michigan Aeronautical Science Association
 * Created May 3, 2020
 */

#ifndef MS5607_H
#define MS5607_H

#include "stm32f4xx_hal_spi.h"

#define MAX11128_PACK_16_TO_8(data) {data>>8, (data<<8)>>8}  // MSB SPI line
#define MAX11128_CS_ACTIVE GPIO_PIN_RESET  // chip is active low
#define MAX11128_CS_INACTIVE GPIO_PIN_SET

// Pressure and temperature ranges the device can handle
#define MINIMUM_PRESSURE        = 1000    //mbar*100 (10 mbar)
#define MAXIMUM_PRESSURE        = 120000  //mbar*100 (1200 )
#define MINIMUM_TEMPERATURE     = -4000   //Celsius*100 (-40 C)
#define MAXIMUM_TEMPERATURE     = 8500    //Celsius*100 (85 C)
#define REFERENCE_TEMPERATURE   = 2000    //Celsius*100 (20 C)

/** Command bytes 
 * Each command has a corresponding 1-byte sequence, with
 * each bit representing something
 * 
 * datasheet p.10:
 * Bit #    Bit name
 * 0        PRM
 * 1        COV
 * 2        -
 * 3        Typ
 * 4        Ad2/Os2
 * 5        Ad1/Os1
 * 6        Ad0/Os0
 * 7        Stop
 * 
 * _OSR_####_ is "oversampling ratio", with commands for
 * each of the five settings: 256/512/1024/2048/4096
 * 
 */
#define RESET_CMD                   0x1E    // 0b00011110
#define CONVERT_D1_OSR_256_CMD      0x40    // 0b01000000
#define CONVERT_D1_OSR_512_CMD      0x42    // 0b01000010
#define CONVERT_D1_OSR_1024_CMD     0x44    // 0b01000100
#define CONVERT_D1_OSR_2048_CMD     0x46    // 0b01000110
#define CONVERT_D1_OSR_4096_CMD     0x48    // 0b01001000
#define CONVERT_D2_OSR_256_CMD      0x50    // 0b01010000
#define CONVERT_D2_OSR_512_CMD      0x52    // 0b01010010
#define CONVERT_D2_OSR_1024_CMD     0x54    // 0b01010100
#define CONVERT_D2_OSR_2048_CMD     0x56    // 0b01010110
#define CONVERT_D2_OSR_4096_CMD     0x58    // 0b01011000
#define ADC_READ_CMD                0x00    // 0b00000000
#define PROM_READ_CMD
/*PROM Read 1 0 1 0 Ad2 Ad1 Ad0 0 : 0xA0 to 0xAE
/*

/** WRITE_ADC_MODE
 * helper function to set MAX11128 Mode Control register
 * 
 * documentation starts at datasheet p.24 and just goes on...
 *
 * @param SPI_BUS      <SPI_HandleTypeDef*> SPI object ADC is on
 * @param cs_base      <GPIO_TypeDef*>      GPIO pin array cs pin is on
 * @param cs_pin       <uint16_t>           GPIO connected to ADC chip select
 * @param register_val <uint16_t>           value to write to mode control register
 *
 * Note: assumes 8bit data framing on SPI
 *
 */
/*
void WRITE_ADC_MODE(SPI_HandleTypeDef* SPI_BUS, GPIO_TypeDef* cs_fpio,
                    uint16_t cs_pin, uint16_t register_val)
{
    uint8_t tx[2] = MAX11128_PACK_16_TO_8(register_val & ADC_MODE_CTRL_RIN);
    __disable_irq();    // disable interrupts during transmission!!!!
    HAL_GPIO_WritePin(cs_gpio, cs_pin, MAX11128_CS_ACTIVE);
    if(HAL_SPI_Transmit(SPI_BUS, tx, 2, 1) ==  HAL_TIMEOUT){}
    HAL_GPIO_WritePin(cs_gpio, cs_pin, MAX11128_CS_INACTIVE);
    __enable_irq();  // end transmission... created by mr. warburton
}
*/

/** RESET
 * Reset sequence should be sent once after poewr-on to make sure that
 * the calibration PROM gets loaded into the internal register.
 * It can be also used to reset the device ROM from an unknown condition.
 * 
 */
void reset();

/**
 * 6 coefficients must be read from the device's memory (PROM)
 * to accurately convert pressure and temperature data into altitude
 */
void read_calibration_coefficients();

/**
 * D1 = digital pressure value
 */
void convert_digital_pressure();

/**
 * D2 = digital temperature value
 */
void convert_digital_temperature();

/**
 * 
 */
void read_ADC_result();

/*
1. Reset
2. Read PROM (128 bit of calibration words)
3. D1 conversion
4. D2 conversion
5. Read ADC result (
*/

#endif  /* end header include protection */