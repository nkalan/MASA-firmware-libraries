/*
 * L6470.c
 *
 *  Created on: May 23, 2021
 *      Author: natha
 *
 *  Datasheet:
 *  https://www.st.com/content/ccc/resource/technical/document/datasheet/a5/86/06/1c/fa/b2/43/db/CD00255075.pdf/files/CD00255075.pdf/_jcr_content/translations/en.CD00255075.pdf
 *
 *
 *  Note: there's a lot of manual bit shifting because each SPI command
 *  argument is left justified, and they all have different lengths
 */

#include "L6470.h"

/**
 * Constants
 *
 */
#define L6470_SPI_TIMEOUT (0xFF)

/**
 * Stepping Mode
 *
 * Default mode on reset is 128th microstep.
 * When it's changed, the ABS_POS register is invalidated.
 *
 * datasheet pg 47
 */
#define L6470_FULL_STEP_MODE          (0b000)
#define L6470_HALF_STEP_MODE          (0b001)
#define L6470_QUARTER_MICROSTEP_MODE  (0b010)
#define L6470_EIGHTH_MICROSTEP_MODE   (0b011)
#define L6470_16_MICROSTEP_MODE       (0b100)
#define L6470_32_MICROSTEP_MODE       (0b101)
#define L6470_64_MICROSTEP_MODE       (0b110)
#define L6470_128_MICROSTEP_MODE      (0b111)

/**
 * Commands
 *
 * Each command is an 8bit binary code, where the first 3 bits (b7-5) are the
 * command type, and the remaining 5 bits (b4-0) are command parameters.
 * Some commands require additional parameters, which are commented.
 *
 * Comments identify bits 7-0, with b7=MSB, b0=LSB
 * Bits are MSB first
 *
 * Datasheet pg 56
 */
#define L6470_CMD_NOP                    ((uint8_t) 0b00000000)
#define L6470_CMD_SETPARAM               ((uint8_t) 0b00000000)  // set PARAM register in b4-0
#define L6470_CMD_GETPARAM               ((uint8_t) 0b00100000)  // set PARAM register in b4-0
#define L6470_CMD_RUN                    ((uint8_t) 0b01010000)  // b0=DIR
#define L6470_CMD_STEPCLOCK              ((uint8_t) 0b01011000)  // b0=DIR
#define L6470_CMD_MOVE                   ((uint8_t) 0b01000000)  // b0=DIR
#define L6470_CMD_GOTO                   ((uint8_t) 0b01100000)  // always takes minimum path
#define L6470_CMD_GOTO_DIR               ((uint8_t) 0b01101000)  // set b0=DIR to force direction
#define L6470_CMD_GOUNTIL                ((uint8_t) 0b10000010)  // b3=ACT, b0=DIR
#define L6470_CMD_RELEASESW              ((uint8_t) 0b10010010)  // b3=ACT, b0=DIR
#define L6470_CMD_GOHOME                 ((uint8_t) 0b01110000)
#define L6470_CMD_GOMARK                 ((uint8_t) 0b01111000)
#define L6470_CMD_RESETPOS               ((uint8_t) 0b11011000)
#define L6470_CMD_RESETDEVICE            ((uint8_t) 0b11000000)
#define L6470_CMD_SOFTSTOP               ((uint8_t) 0b10110000)
#define L6470_CMD_HARDSTOP               ((uint8_t) 0b10111000)
#define L6470_CMD_SOFTHIZ                ((uint8_t) 0b10100000)
#define L6470_CMD_HARDHIZ                ((uint8_t) 0b10101000)
#define L6470_CMD_GETSTATUS              ((uint8_t) 0b11010000)

/**
 * Register Addresses
 * Each one stores a different motor IC parameter
 * Each parameter has a different length, see datasheet
 *
 * Parameter addresses should be ORed with the GETPARAM and SETPARAM commands
 * Note that all param addresses are 5 bits. These fit into the GETPARAM and SETPARAM commands LSBs
 *
 * datasheet pg 40
 */
#define L6470_PARAM_ABS_POS_ADDR        ((uint8_t) 0x01)  // 22 bits
#define L6470_PARAM_EL_POS_ADDR         ((uint8_t) 0x02)  // 9 bits
#define L6470_PARAM_MARK_ADDR           ((uint8_t) 0x03)  // 22 bits
#define L6470_PARAM_SPEED_ADDR          ((uint8_t) 0x04)  // 20 bits
#define L6470_PARAM_ACC_ADDR            ((uint8_t) 0x05)  // 12 bits
#define L6470_PARAM_DEC_ADDR            ((uint8_t) 0x06)  // 12 bits
#define L6470_PARAM_MAX_SPEED_ADDR      ((uint8_t) 0x07)  // 10 bits
#define L6470_PARAM_MIN_SPEED_ADDR      ((uint8_t) 0x08)  // 13 bits
#define L6470_PARAM_FS_SPD_ADDR         ((uint8_t) 0x15)  // 10 bits
#define L6470_PARAM_KVAL_HOLD_ADDR      ((uint8_t) 0x09)  // 8 bits
#define L6470_PARAM_KVAL_RUN_ADDR       ((uint8_t) 0x0A)  // 8 bits
#define L6470_PARAM_KVAL_ACC_ADDR       ((uint8_t) 0x0B)  // 8 bits
#define L6470_PARAM_KVAL_DEC_ADDR       ((uint8_t) 0x0C)  // 8 bits
#define L6470_PARAM_INT_SPEED_ADDR      ((uint8_t) 0x0D)  // 14 bits
#define L6470_PARAM_ST_SLP_ADDR         ((uint8_t) 0x0E)  // 8 bits
#define L6470_PARAM_FN_SLOP_ACC_ADDR    ((uint8_t) 0x0F)  // 8 bits
#define L6470_PARAM_FN_SLOP_DEC_ADDR    ((uint8_t) 0x10)  // 8 bits
#define L6470_PARAM_K_THERM_ADDR        ((uint8_t) 0x11)  // 4 bits
#define L6470_PARAM_ADC_OUT_ADDR        ((uint8_t) 0x12)  // 5 bits
#define L6470_PARAM_OCD_TH_ADDR         ((uint8_t) 0x13)  // 4 bits
#define L6470_PARAM_STALL_TH_ADDR       ((uint8_t) 0x14)  // 7 bits
#define L6470_PARAM_STEP_MODE_ADDR      ((uint8_t) 0x16)  // 8 bits
#define L6470_PARAM_ALARM_EN_ADDR       ((uint8_t) 0x17)  // 8 bits
#define L6470_PARAM_CONFIG_ADDR         ((uint8_t) 0x18)  // 16 bits
#define L6470_PARAM_STATUS_ADDR         ((uint8_t) 0x19)  // 16 bits

/**
 * Status register bits
 * Latched status bits stay in that state until they are read
 * datasheet pg 55
 */
#define L6470_STATUS_BIT_HiZ           ((uint16_t)0x0001)  // Bridges are in HiZ
#define L6470_STATUS_BIT_BUSY          ((uint16_t)0x0002)  // Mirrors ~BUSY pin
#define L6470_STATUS_BIT_SW_F          ((uint16_t)0x0004)  // SW input status (low==open)
#define L6470_STATUS_BIT_SW_EVN        ((uint16_t)0x0008)  // (latched) SW input falling edge (switch turn on event)
#define L6470_STATUS_BIT_DIR           ((uint16_t)0x0010)  // 1=forward, 0=reverse
#define L6470_STATUS_BIT_MOT_STATUS_1  ((uint16_t)0x0020)  // See L6470_Motor_Status enum
#define L6470_STATUS_BIT_MOT_STATUS_0  ((uint16_t)0x0040)  // ^
#define L6470_STATUS_BIT_NOTPERF_CMD   ((uint16_t)0x0080)  // (latched) SPI command not performed
#define L6470_STATUS_BIT_WRONG_CMD     ((uint16_t)0x0100)  // (latched) SPI command doesn't exist
#define L6470_STATUS_BIT_UVLO          ((uint16_t)0x0200)  // (active low, latched) Undervoltage lockout or reset
#define L6470_STATUS_BIT_TH_WRN        ((uint16_t)0x0400)  // (latched) Thermal warning
#define L6470_STATUS_BIT_TH_SD         ((uint16_t)0x0800)  // (latched) Thermal shutdown
#define L6470_STATUS_BIT_OCD           ((uint16_t)0x1000)  // (latched) Overcurrent
#define L6470_STATUS_BIT_STEP_LOSS_A   ((uint16_t)0x2000)  // (active low, latched) Stall detected on bridge A
#define L6470_STATUS_BIT_STEP_LOSS_B   ((uint16_t)0x4000)  // (active low, latched) Stall detected on bridge B
#define L6470_STATUS_BIT_SCK_MOD       ((uint16_t)0x8000)  // (unused) step-clock mode

/**
 * Transmit a message to the motor chip over SPI
 *
 * @param tx <uint8_t*> Pointer to array containing SPI transmit message
 * @param tx_sz <uint16_t> Size of the tx array in bytes
 */
void L6470_SPI_transmit(L6470_Motor_IC *motor, uint8_t *tx, uint16_t tx_sz) {
	__disable_irq();  // Prevent interrupts during the communication
	HAL_GPIO_WritePin(motor->cs_base, motor->cs_pin, GPIO_PIN_RESET);  // chip select active low
	motor->HAL_SPI_Status = HAL_SPI_Transmit(motor->hspi, tx, tx_sz,
			L6470_SPI_TIMEOUT);  // Store status code
	HAL_GPIO_WritePin(motor->cs_base, motor->cs_pin, GPIO_PIN_SET);
	__enable_irq();
}

/**
 * Transmit and receive a message to the motor chip over SPI.
 * Same functionality as L6470_SPI_transmit_receive except with receiving
 *
 * @param tx <uint8_t*> Pointer to array containing SPI transmit message
 * @param tx_sz <uint16_t> Size of the tx array in bytes
 * @param rx <uint8_t*> Pointer to array to receive the message in
 * @param rx_sz <uint16_t> Size of the rx array in bytes
 */
void L6470_SPI_transmit_receive(L6470_Motor_IC *motor, uint8_t *tx,
		uint16_t tx_sz, uint8_t *rx, uint16_t rx_sz) {
	__disable_irq();
	HAL_GPIO_WritePin(motor->cs_base, motor->cs_pin, GPIO_PIN_RESET);
	motor->HAL_SPI_Status = HAL_SPI_Transmit(motor->hspi, tx, tx_sz,
			L6470_SPI_TIMEOUT);
	motor->HAL_SPI_Status |= HAL_SPI_Receive(motor->hspi, rx, rx_sz,
			L6470_SPI_TIMEOUT);
	HAL_GPIO_WritePin(motor->cs_base, motor->cs_pin, GPIO_PIN_SET);
	__enable_irq();
}

/**
 * Read the status register and update the struct's status variables
 * Datasheet pg 55
 */
void L6470_update_status(L6470_Motor_IC *motor) {
	uint8_t tx[1] = { L6470_CMD_GETPARAM | L6470_PARAM_STATUS_ADDR };
	uint8_t rx[2];
	L6470_SPI_transmit_receive(motor, tx, 1, rx, 2);
	uint16_t status_reg = ((uint16_t)rx[1] << 8) | ((uint16_t)rx[0]);

	// 1 bit statuses ("casting as bool" to avoid integer overflow)
	motor->HiZ_status         = 0 != (status_reg & L6470_STATUS_BIT_HiZ);
	motor->BUSY_status        = 0 != (status_reg & L6470_STATUS_BIT_BUSY);
	motor->SW_F_status        = 0 != (status_reg & L6470_STATUS_BIT_SW_F);
	motor->SW_EVN_status      = 0 != (status_reg & L6470_STATUS_BIT_SW_EVN);
	motor->DIR_status         = 0 != (status_reg & L6470_STATUS_BIT_DIR);
	motor->NOTPERF_CMD_status = 0 != (status_reg & L6470_STATUS_BIT_NOTPERF_CMD);
	motor->WRONG_CMD_status   = 0 != (status_reg & L6470_STATUS_BIT_WRONG_CMD);
	motor->UVLO_status        = 0 != (status_reg & L6470_STATUS_BIT_UVLO);
	motor->TH_WRN_status      = 0 != (status_reg & L6470_STATUS_BIT_TH_WRN);
	motor->TH_SD_status       = 0 != (status_reg & L6470_STATUS_BIT_TH_SD);
	motor->OCD_status         = 0 != (status_reg & L6470_STATUS_BIT_OCD);
	motor->STEP_LOSS_A_status = 0 != (status_reg & L6470_STATUS_BIT_STEP_LOSS_A);
	motor->STEP_LOSS_B_status = 0 != (status_reg & L6470_STATUS_BIT_STEP_LOSS_B);
	motor->SCK_MOD_status     = 0 != (status_reg & L6470_STATUS_BIT_SCK_MOD);

	// 2 bit motor status
	uint8_t motor_status_1    = status_reg & L6470_STATUS_BIT_MOT_STATUS_1;
	uint8_t motor_status_0    = status_reg & L6470_STATUS_BIT_MOT_STATUS_0;

	uint8_t motor_status = (motor_status_1 << 1) | (motor_status_0);
	switch(motor_status) {
	case 0:
		motor->MOT_status = Stopped;
		break;
	case 1:
		motor->MOT_status = Acceleration;
		break;
	case 2:
		motor->MOT_status = Deceleration;
		break;
	case 3:
		motor->MOT_status = Constant_Speed;
		break;
	default:
		break;
	}
}

float L6470_get_motor_speed(L6470_Motor_IC *motor) {
	// Datasheet pg 42
	return 0;
}
