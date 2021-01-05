/*
 * MAX31855.h
 *
 *  Created on: Jan 4, 2021
 *      Author: arthur
 */

#ifndef MAX31855_H_
#define MAX31855_H_
#include <stdint.h>
#include "stm32f4xx_hal.h"

#define MAX31855_lutIndexOffset         200
#define MAX31855_minVoltage             -5603
#define MAX31855_maxVoltage             17819
// Fault code to be returned if no thermocouple is attached
// or there are short circuits
#define FAULT_CODE                      9999
// Out of range code to be returned if the thermocouple microvolts reading
// is outside our -200 to 350C LUT range
#define OOR_CODE                        8888
// Coeff used by MAX31855 to calculate temperature
// for type T thermocouple (units in uV/C)
#define COLD_JUNC_SENSITIVITY_COEFF_T   52.18

const int16_t MAX31855_TempToMicroVolts_LUT[551];

typedef struct MAX31855_Pinfo {
    GPIO_TypeDef* MAX31855_CS_PORT;     // PORT belonging to CS pin
    uint16_t MAX31855_CS_ADDR;          // PIN belonging to CS pin
} MAX31855_Pinfo;

/* Public Function Prototypes */
#ifdef HAL_SPI_MODULE_ENABLED
/**
 *  Initialize thermocouple hardware component
 *
 *  @param pins         <MAX31855_Pinfo*> contains tc pin defs,
 *                                              refer to def above
 *
 */
void init_tc(MAX31855_Pinfo* pinfo);

/**
 *  Convenience function for reading from a tc
 *
 *  @param SPI_BUS      <SPI_HandleTypeDef*> SPI object tc is on
 *  @param pinfo        <MAX31855_Pinfo*>   contains tc pin defs
 *  @returns            <float> calibrated tc reading
 *
 */
float read_tc(SPI_HandleTypeDef* SPI_BUS, MAX31855_Pinfo* pinfo);

#endif /* HAL SPI */

#endif /* MAX31855_H_ */
