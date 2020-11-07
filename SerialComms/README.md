Basic Function Operations

init_data() - User must initialize an arr for telem data buffer and pass this 
                buffer's address as a pointer to this library. The library will use this pointer whenever it needs to pack packets to telemetry. Using this method, the user only needs to fill their telem data buffer before calling any of the functions used to preparing a packet for transmission                