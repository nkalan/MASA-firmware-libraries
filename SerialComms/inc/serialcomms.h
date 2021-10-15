#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H

#define DMA_BUFFER_SIZE = 2048;

typedef struct UART_DMAHandle {
	UART_HandleTypeDef *huart;
	uint8_t *telembuffer;
	uint8_t *RxRollover;
	uint8_t *dma_buffer[DMA_BUFFER_SIZE];

} UART_DMAHandle;


void init_comms(UART_DMAHandle *comms);

void idle_line_interrupt_handle();

void transmit();

void recieve();

#endif
