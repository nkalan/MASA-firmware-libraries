/*
 * L6470.h
 *
 *  Created on: May 23, 2021
 *      Author: natha
 *
 *  Datasheet:
 *  https://www.st.com/content/ccc/resource/technical/document/datasheet/a5/86/06/1c/fa/b2/43/db/CD00255075.pdf/files/CD00255075.pdf/_jcr_content/translations/en.CD00255075.pdf
 *
 */

#ifndef INC_L6470_H_
#define INC_L6470_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

#ifdef HAL_SPI_MODULE_ENABLED	// Begin SPI include protection


/**
 * Returned from status register
 * datasheet pg 56
 */
typedef enum {
	Stopped = 0,
	Acceleration = 1,
	Deceleration = 2,
	Constant_Speed = 3
} L6470_Motor_Status;

typedef struct {
	SPI_HandleTypeDef *hspi;      // SPI bus for communication, specified by user
	GPIO_TypeDef *cs_base;        // Chip select pin, specified by user
	uint16_t cs_pin;

	GPIO_TypeDef *busy_base;      // Active low pin set by motor while executing commands
	uint16_t busy_pin;

	uint16_t speed;               // motor speed, in steps/tick
	// bounded by MIN_SPEED and MAX_SPEED

	// HAL SPI status gets updated after every SPI transmission
	HAL_StatusTypeDef HAL_SPI_Status;

	// Status bits that get updated when the STATUS register is read
	L6470_Motor_Status MOT_status;
	uint8_t HiZ_status;
	uint8_t BUSY_status;
	uint8_t SW_F_status;  // unused
	uint8_t SW_EVN_status;  // unused
	uint8_t DIR_status;
	uint8_t NOTPERF_CMD_status;
	uint8_t WRONG_CMD_status;
	uint8_t UVLO_status;
	uint8_t TH_WRN_status;
	uint8_t TH_SD_status;
	uint8_t OCD_status;
	uint8_t STEP_LOSS_A_status;
	uint8_t STEP_LOSS_B_status;
	uint8_t SCK_MOD_status;  // unused

} L6470_Motor_IC;

/**
 * Returns the motor speed from the chip's internal register
 * Raw bits are in steps/tick, function returns steps/s
 */
float L640_get_motor_speed(L6470_Motor_IC* motor);

void L6470_update_status(L6470_Motor_IC* motor);

#endif /* HAL_SPI_MODULE_ENABLED */
#endif /* INC_L6470_H_ */
