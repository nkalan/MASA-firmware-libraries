#include <stdint.h>

void set_vlv(uint8_t* data, uint8_t* status){

	uint32_t vlv_num = (data[0]<<data[1]<<data[2])/1;
	uint8_t state = (data[3])/1;
	
}
void send_telem_short(uint8_t* data, uint8_t* status){

	uint8_t board_num = (data[0])/1;
	
}
void send_telem_all(uint8_t* data, uint8_t* status){

	
}
