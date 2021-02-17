#include <stdint.h>

void set_vlv(uint8_t* data, uint8_t* status){

	uint32_t vlv_num = (data[3]<<24|data[2]<<16|data[1]<<8|data[0])/1;
	uint8_t state = (data[4])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED
	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void send_telem_short(uint8_t* data, uint8_t* status){

	uint8_t board_num = (data[0])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void send_telem_all(uint8_t* data, uint8_t* status){

	uint8_t board_num = (data[0])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_stepper_period(uint8_t* data, uint8_t* status){

	uint8_t stepper_num = (data[0])/1;
	uint16_t period = (data[2]<<8|data[1])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_stepper_direction(uint8_t* data, uint8_t* status){

	uint8_t stepper_num = (data[0])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_kp(uint8_t* data, uint8_t* status){

	uint8_t motor_num = (data[0])/1;
	double gain = (data[1])/100.0;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_ki(uint8_t* data, uint8_t* status){

	uint8_t motor_num = (data[0])/1;
	double gain = (data[1])/100.0;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_kd(uint8_t* data, uint8_t* status){

	uint8_t motor_num = (data[0])/1;
	double gain = (data[1])/100.0;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_control_calc_period(uint8_t* data, uint8_t* status){

	uint8_t period = (data[0])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_state(uint8_t* data, uint8_t* status){

	uint8_t state = (data[0])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void move_stepper_degrees(uint8_t* data, uint8_t* status){

	uint8_t motor_num = (data[0])/1;
	uint16_t deg = (data[2]<<8|data[1])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void download_flash(uint8_t* data, uint8_t* status){

	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void wipe_flash(uint8_t* data, uint8_t* status){

	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void start_logging(uint8_t* data, uint8_t* status){

	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void stop_logging(uint8_t* data, uint8_t* status){

	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_stepper_pos(uint8_t* data, uint8_t* status){

	uint8_t motor_num = (data[0])/1;
	float position = (data[4]<<24|data[3]<<16|data[2]<<8|data[1])/100.0;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_stepper_zero(uint8_t* data, uint8_t* status){

	uint8_t motor_num = (data[0])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_control_target_pressure(uint8_t* data, uint8_t* status){

	uint8_t tank_num = (data[0])/1;
	float target_pressure = (data[4]<<24|data[3]<<16|data[2]<<8|data[1])/10.0;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void ambientize_pressure_transducers(uint8_t* data, uint8_t* status){

	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_low_toggle_percent(uint8_t* data, uint8_t* status){

	uint8_t tank_num = (data[0])/1;
	float lower_threshold_pct = (data[4]<<24|data[3]<<16|data[2]<<8|data[1])/10.0;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_high_toggle_percent(uint8_t* data, uint8_t* status){

	uint8_t tank_num = (data[0])/1;
	float upper_threshold_pct = (data[4]<<24|data[3]<<16|data[2]<<8|data[1])/10.0;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_control_loop_duration(uint8_t* data, uint8_t* status){

	uint32_t duration = (data[3]<<24|data[2]<<16|data[1]<<8|data[0])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_stepper_speed(uint8_t* data, uint8_t* status){

	uint8_t motor_num = (data[0])/1;
	uint16_t target_speed = (data[2]<<8|data[1])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

void set_telem(uint8_t* data, uint8_t* status){

	uint8_t state = (data[0])/1;
	
	// USER CODE BEGIN - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

	// USER CODE END - MODIFICATIONS OUTSIDE THIS SECTION WILL BE DELETED

}

