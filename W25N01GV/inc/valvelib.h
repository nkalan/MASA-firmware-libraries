/*
 * valvelib.h
 *
 *  Created on: Jul. 5, 2021
 *      Author: elham
 */

#ifndef W25N01GV_INC_VALVELIB_H_ // Begin header include protection
#define W25N01GV_INC_VALVELIB_H_


typedef struct {
	uint8_t num_valves;
	uint8_t valve_states[num_valves];
	void (*set_valve_func)(uint8_t, uint8_t);


} Valves;

void power_valve(Valves *valves);

void depower_valve(Valves *valves);


#endif /* W25N01GV_INC_VALVELIB_H_ */
