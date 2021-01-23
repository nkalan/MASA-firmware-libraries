#include <stdint.h>

void set_vlv(uint8_t* data, uint8_t* status){

	uint32_t vlv_num = (data[0]<<data[1]<<data[2])/1;
	uint8_t state = (data[3])/1;
	
}
void send_telem_short(uint8_t* data, uint8_t* status){

	uint8_t board_num = (data[0])/1;
	
}
void send_telem_all(uint8_t* data, uint8_t* status){

	uint8_t board_num = (data[0])/1;
	
}
void set_stepper_period(uint8_t* data, uint8_t* status){

	uint8_t stepper_num = (data[0])/1;
	uint16_t period = (data[1]<<data[2])/1;
	
}
void set_stepper_direction(uint8_t* data, uint8_t* status){

	uint8_t stepper_num = (data[0])/1;
	
}
void set_kp(uint8_t* data, uint8_t* status){

	double gain = (data[0])/100.0;
	
}
void set_ki(uint8_t* data, uint8_t* status){

	double gain = (data[0])/100.0;
	
}
void set_kd(uint8_t* data, uint8_t* status){

	double gain = (data[0])/100.0;
	
}
void set_control_calc_period(uint8_t* data, uint8_t* status){

	uint16_t period = (data[0]<<data[1])/1;
	
}
void set_state(uint8_t* data, uint8_t* status){

	uint8_t state = (data[0])/1;
	
}
void move_stepper_degrees(uint8_t* data, uint8_t* status){

	uint8_t motor_num = (data[0])/1;
	uint16_t deg = (data[1]<<data[2])/1;
	
}
void download_flash(uint8_t* data, uint8_t* status){

	
}
void wipe_flash(uint8_t* data, uint8_t* status){

	
}
void start_logging(uint8_t* data, uint8_t* status){

	
}
void stop_logging(uint8_t* data, uint8_t* status){

	
}
void set_stepper_pos(uint8_t* data, uint8_t* status){

	uint8_t motor_num = (data[0])/1;
	float position = (data[1])/100.0;
	
}
void set_stepper_zero(uint8_t* data, uint8_t* status){

	uint8_t motor_num = (data[0])/1;
	
}
