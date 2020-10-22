/*
 * comms.h
 *
 *  Created on: Jul 21, 2020
 *      Author: Jack Taliercio
 */

#ifndef INC_COMMS_H_
#define INC_COMMS_H_

/* Included */
#include "stdint.h"
#include "stdlib.h"



/* Telemetry Data */
uint8_t telem_unstuffed[253];
uint8_t telem_stuffed[255];
uint8_t *data_arr[254];
extern uint8_t test;

#define TELEM_ITEM_0 test
#define TELEM_ITEM_1 ('7' >> 0)
#define TELEM_ITEM_2 ('3' >> 0)
#define TELEM_ITEM_3 ('5' >> 0)
#define TELEM_ITEM_4 ('1' >> 0)

/* Public Function Prototypes */

/**
 *  packs (updates) the unstuffed telem packet with new data
 *
 *  @param unstuffed	Pointer to unstuffed array that needs to be updated with new data
 *
 *  Note:
 */
void pack_data(uint8_t *unstuffed, uint8_t size_t length);


/**
 *  Stuff, using COBS https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing,
 *  an ustuffed packet in preperation for transmission
 *
 *  @param unstuffed	Pointer to unstuffed populated array
 *  @param stuffed		Pointer to unpolulated array that will be stuffed
 *  @param length		length of the unstuffed packet to be stuffed
 *
 *	@returns			Returns the length of the stuffed packet
 *  Note:
 */
size_t stuff_packet(uint8_t *unstuffed, uint8_t *stuffed, size_t length);





#endif /* INC_COMMS_H_ */
