/*
 * comms.c
 *
 *  Created on: Jul 21, 2020
 *      Author: Jack Taliercio
 */

#include "comms.h"


uint8_t test;


void init_data(){
	test = 0;
	*data_arr = &TELEM_ITEM_0;
	//*(data_arr+1) = &TELEM_ITEM_1;
	//*(data_arr+2) = &TELEM_ITEM_2;
	//*(data_arr+3) = &TELEM_ITEM_3;
}



void pack_data(uint8_t *unstuffed, size_t length){
	for(size_t i = 0; i < length; i++){
		*(unstuffed + i) = *(data_arr[i]);
	}
}



//This code was shamelessly stolen from wikipedia, docs by me tho
size_t stuff_packet(uint8_t *unstuffed, uint8_t *stuffed, size_t length){

	//Start just keeps track of the start point
	uint8_t *start = stuffed;
	//Code represents the number of positions till the next 0 and code_ptr
	//holds the position of the last zero to be updated when the next 0 is found
	uint8_t code = 1;
	uint8_t *code_ptr = stuffed++; //Note: this sets code_ptr to stuffed, then ++ stuffed

	while (length--)
	{
		//If the current byte is not zero, add that byte to suffed data and increment
		//the position of the last zero (code)
		if (*unstuffed){
			*stuffed = *unstuffed;
			++code;

		}
		//IF the current byte is not zero, OR if the current code is maxed out
		//Update the last zero position with code, reset code, and set the code_ptr
		//To the new stuffed position
		else if (!*unstuffed || code == 0xFF){ /* Input is zero or complete block */
			*code_ptr = code;
			code = 1;
			code_ptr = stuffed;
		}
		//Move both unstuffed and stuffed forward
		unstuffed++;
		stuffed++;
	}
	//Set the final code
	*code_ptr = code;
	//Add ending 0
	*stuffed = 0;
	//Returns length of encoded data
	return stuffed - start;

}
