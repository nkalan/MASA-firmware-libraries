/*
 * Implementation of wrapper around existing valve functions
 * Elham Islam (eislam@umich.edu)
 * Michigan Aeronautical Science Association
 * Created July 5, 2021
 * Last edited July 5, 2021
 */


#include "../inc/valvelib.h"


void power_valve(Valves *valves){
	for(i = 0; i < valves->num_valves; i++){
		if(valves->valve_states[i] == 1){
		valves->set_valve_func(i, 1); // set valve channel to high as per valve_states array
	}
}

void depower_valve(Valves *valves){
	for(i = 0; i < valves->num_valves; i++){
		if(valves->valve_states[i] == 0){
		valves->set_valve_func(i, 0); // set valve channel to low as per valve_states array
	}
}

