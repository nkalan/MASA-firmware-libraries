/*
 * valvelib.c
 *
 *  Created on: Jul. 5, 2021
 *      Author: elham
 */


#include "../inc/valvelib.h"


void power_valve(Valves *valves){
	for(i = 0; i < valves->num_valves; i++){
		if(valves->valve_states[i] == 1){
		valves->set_valve_func(valve_states[i], 1);
	}
}

void depower_valve(Valves *valves){
	for(i = 0; i < valves->num_valves; i++){
		if(valves->valve_states[i] == 0){
		valves->set_valve_func(valve_states[i], 0);
	}
}

