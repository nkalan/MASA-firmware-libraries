/*
 * Header file for wrapper around existing valve functions
 *
 * Elham Islam (eislam@umich.edu)
 * Michigan Aeronautical Science Association
 * Created July 5, 2021
 * Last edited July 5, 2021
 */

#ifndef W25N01GV_INC_VALVELIB_H_ // Begin header include protection
#define W25N01GV_INC_VALVELIB_H_


typedef struct {
	uint8_t num_valves;
	uint8_t valve_states[num_valves]; // array containing states of each valve
	void (*set_valve_func)(uint8_t, uint8_t); // existing function used to set valve states


} Valves;

void power_valve(Valves *valves); //set valve channels high

void depower_valve(Valves *valves); //set valve channels low


#endif /* W25N01GV_INC_VALVELIB_H_ */
