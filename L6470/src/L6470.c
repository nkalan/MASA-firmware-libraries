/*
 * L6470.c
 *
 *  Created on: May 23, 2021
 *      Author: natha
 *
 *  Datasheet:
 *  https://www.st.com/content/ccc/resource/technical/document/datasheet/a5/86/06/1c/fa/b2/43/db/CD00255075.pdf/files/CD00255075.pdf/_jcr_content/translations/en.CD00255075.pdf
 *
 */



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
 * Parameter Addresses
 * Stored in registers on motor IC
 *
 * Each parameter has a different length, see datasheet
 *
 * Parameter addresses should be ORed with the GETPARAM and SETPARAM commands
 *
 * datasheet pg 40
 */
#define L6470_PARAM_ABS_POS             ((uint8_t) 0x01)
#define L6470_PARAM_EL_POS              ((uint8_t) 0x02)
#define L6470_PARAM_MARK                ((uint8_t) 0x03)
#define L6470_PARAM_SPEED               ((uint8_t) 0x04)
#define L6470_PARAM_ACC                 ((uint8_t) 0x05)
#define L6470_PARAM_DEC                 ((uint8_t) 0x06)
#define L6470_PARAM_MAX_SPEED           ((uint8_t) 0x07)
#define L6470_PARAM_MIN_SPEED           ((uint8_t) 0x08)
#define L6470_PARAM_FS_SPD              ((uint8_t) 0x15)
#define L6470_PARAM_KVAL_HOLD           ((uint8_t) 0x09)
#define L6470_PARAM_KVAL_RUN            ((uint8_t) 0x0A)
#define L6470_PARAM_KVAL_ACC            ((uint8_t) 0x0B)
#define L6470_PARAM_KVAL_DEC            ((uint8_t) 0x0C)
#define L6470_PARAM_INT_SPEED           ((uint8_t) 0x0D)
#define L6470_PARAM_ST_SLP              ((uint8_t) 0x0E)
#define L6470_PARAM_FN_SLOP_ACC         ((uint8_t) 0x0F)
#define L6470_PARAM_FN_SLOP_DEC         ((uint8_t) 0x10)
#define L6470_PARAM_K_THERM             ((uint8_t) 0x11)
#define L6470_PARAM_ADC_OUT             ((uint8_t) 0x12)
#define L6470_PARAM_OCD_TH              ((uint8_t) 0x13)
#define L6470_PARAM_STALL_TH            ((uint8_t) 0x14)
#define L6470_PARAM_STEP_MODE           ((uint8_t) 0x16)
#define L6470_PARAM_ALARM_EN            ((uint8_t) 0x17)
#define L6470_PARAM_CONFIG              ((uint8_t) 0x18)
#define L6470_PARAM_STATUS              ((uint8_t) 0x19)
