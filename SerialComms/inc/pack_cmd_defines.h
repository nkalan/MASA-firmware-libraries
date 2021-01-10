#ifndef PACK_CMD_DEFINES_H
#define PACK_CMD_DEFINES_H
#define NUM_CMD_ITEMS 11
#include <stdint.h>

void set_vlv(uint8_t* data, uint8_t* status);

void send_telem_short(uint8_t* data, uint8_t* status);

void send_telem_all(uint8_t* data, uint8_t* status);

void set_stepper_period(uint8_t* data, uint8_t* status);

void set_stepper_direction(uint8_t* data, uint8_t* status);

void set_kp(uint8_t* data, uint8_t* status);

void set_ki(uint8_t* data, uint8_t* status);

void set_kd(uint8_t* data, uint8_t* status);

void set_control_calc_period(uint8_t* data, uint8_t* status);

void set_state(uint8_t* data, uint8_t* status);

void move_stepper_degrees(uint8_t* data, uint8_t* status);

typedef void (*Cmd_Pointer)(uint8_t* x, uint8_t* y);

static Cmd_Pointer cmds_ptr[NUM_CMD_ITEMS];

// Note: to call a function do
/**
* (*cmds_ptr[0])(array ptr here)
*
* The actual cmd functions will be defined in a separate c file by the firwmare
* developer for each board. They simply need to include this header file
* in the cfile in which they define the function. This allows the developer
* to import/use any variables or typedefs from the hal library. This file is
* simply the jumptable that gives the comms library an easy way to call
* custom functions without additional knowledge of where this file is defined
*
*/


#endif
