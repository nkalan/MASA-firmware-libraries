#include "../inc/pack_cmd_defines.h"

// TODO decide if this is needed

void digital_write(uint8_t* data, uint8_t* status) {
    uint16_t port_number = data[0]<<8|data[1];
    uint16_t pin_number = data[2]<<8|data[3];
    uint8_t state = data[4];
}
