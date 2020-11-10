#include "../inc/pack_telem_defines.h"

/* TODO: DELETE LATER ONCE TESTING IS DONE */
uint16_t pressures[2] = { 0x1234, 0x5678 };
uint8_t test            = 3;

void pack_telem_data(uint8_t* dst) {
    *(dst) = TELEM_ITEM_0;
    *(dst+1) = TELEM_ITEM_1;
    *(dst+2) = TELEM_ITEM_2;
    *(dst+3) = TELEM_ITEM_3;
    *(dst+4) = TELEM_ITEM_4;
}
