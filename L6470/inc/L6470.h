/*
 * L6470.h
 *
 *  Created on: May 23, 2021
 *      Author: natha
 */

#ifndef INC_L6470_H_
#define INC_L6470_H_

//#ifdef HAL_SPI_MODULE_ENABLED	// Begin SPI include protection


typedef struct {
	SPI_HandleTypeDef *hspi;      // SPI bus for communication, specified by user
	GPIO_TypeDef *cs_base;        // Chip select GPIO base, specified by user
	uint16_t cs_pin;              // Chip select GPIO pin, specified by user

	GPIO_TypeDef *busy_base;      // Active low pin set by motor while executing commands
	uint16_t busy_pin;

} L6470_Motor_IC;




//#endif /* HAL_SPI_MODULE_ENABLED */
#endif /* INC_L6470_H_ */
