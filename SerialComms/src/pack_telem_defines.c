#include "../inc/pack_telem_defines.h"

void pack_telem_data(uint8_t* dst) {
    *(dst) = TELEM_ITEM_0;
    *(dst+1) = TELEM_ITEM_1;
    *(dst+2) = TELEM_ITEM_2;
    *(dst+3) = TELEM_ITEM_3;
    *(dst+4) = TELEM_ITEM_4;
    *(dst+5) = TELEM_ITEM_5;
    *(dst+6) = TELEM_ITEM_6;
    *(dst+7) = TELEM_ITEM_7;
    *(dst+8) = TELEM_ITEM_8;
    *(dst+9) = TELEM_ITEM_9;
}
