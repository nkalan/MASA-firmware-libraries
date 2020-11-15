#include "pack_cmd_defines.h"

Cmd_Pointer cmds_ptr[NUM_CMD_ITEMS] = {
set_vlv,
send_telem_short,
send_telem_all,
set_stepper_period,
set_stepper_direction,
set_kp,
set_ki,
set_kd,
set_control_calc_period,
set_state,
move_stepper_degrees
};
