# User Guide

* [MAX3491 Technical Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX3483-MAX3491.pdf)

## Board to Computer Hardware Setup Details

### MAX3491 RS422 Chip Hardware pinouts

A common mistake is to incorrectly flip the RJ-45 ethernet cable pins and the MAX3491 chip. On the current RS422 to USB converter board that was designed by Sternie (a previous MASA member), the RX+- and TX+- aren't mapped the standard RJ45 connector positions. The actual pinouts from RS422 harness to RJ45 connector are as shown below:

1. RX+ -> pin6
2. RX- -> pin3
3. TX- -> pin2
4. TX+ -> pin1

## Configuring Daisy Chain Pinouts

Furthermore, when two boards daisy chained, make sure to pinout a custom CAT5e cable to connect the TX+/TX- pins on one board to the RX+/RX- pins on the other. This ensures that the boards can communicate properly. 

## Configuring UART/DMA Settings On STM32CubeIDE

A higher baud rate can be selected for this library. However, if this is done, please ensure that the clock configuration for UART is at least 16 times faster than the baud rate to ensure proper behavior.


Developers will need to configure the UART settings in the .ioc file under th e connectivity section. Use the following configuration for the various tabs in the desired UART channel:

### Parameter Settings

* Mode: Asynchronous
* Hardware Flow Control (RS232): Disable
* Baud Rate: 115200 Bits/s
* Word Length: 8 Bits
* Parity: None
* Stop Bits: 1
* Data Direction: Receive and Transmit
* Over Sampling: 16 times

### DMA Settings

* Add DMA RX request
* Mode: Circular
* Data Width: Peripheral Byte, Memory Byte
* Increment Address: Peripheral unchecked, Memory Checked
* Use Fifo: Unchecked

### NVIC tab

* USART2 global interrupt: Enabled
* DMAx streamY global interrupt: Enabled

Check that the preemption and sub priority are both 0 for these interrupts. We want to allow dma for telemetry operations to interrupts other less essential interrupts as well.

## Comms Library Setup Struct Explanation

The packet header needs to specified before any commands are transmitted using this library. The packet header will contain essential information, such as the `packet_type`, which will be used to identify how this packet should be handled once it is received. The packet type definitions can be found in the /firmware-libraries/SerialComms/python/packet_telem_template.csv file. 

The `origin_addr` is used for specifying which board the packet originated from. This can be useful for interboard communications. 

In addition, the `target_addr` will be used in the future for allowing you to address specific boards or specify which board transmitted the message. 

The `priority` byte can be used to set different priorities for each telem packet received if multiple are received in parallel. However, all mission critical commands should be notified using an external gpio interrupt instead to guarantee timely handling.

The `do_cobbs` byte is used to indicate whether or not the latter packet is cobbs encoded or not. This is mainly used when encoding the data.

Finally, the `timestamp` variable is used to specify a 32 bit unsigned integer that counts the time in microseconds that have elapsed since the board thas started up. It is important to keep the counts of this timestamp in microseconds to ensure consistent behavior across all boards. 

Note: with a 32 bit unsigned integer for timestamps, we have log for about 1 hour and 20 minutes before the onboard timers will reset back to 0.

```
// Packet Header 
typedef struct CLB_Packet_Header {
    uint8_t packet_type;        // CMD/DATA packet ID
    uint8_t origin_addr;        // origin board address
    uint8_t target_addr;        // target board address
    uint8_t priority;           // priority of packet
    uint8_t do_cobbs;           // 1 to enable cobbs encoding
    uint16_t checksum;          // checksum to ensure robustness (generated)
    uint32_t timestamp;         // timestamp for data
} CLB_Packet_Header;
```

In addition to the packet header, firmware writers should also define the `CLB_send_data_info` struct as well. This struct is used to specify which uart channel should be used when transmitting packets and also packs the packet into an array of bytes that can be written directly into flash. If the developer is only concerned with transmitting packets over uart, they can simply configure the `uartx` field with a pointer to their uart channel.

If the developer needs to pack telemtry data to store into flash, they need to set the `flash_arr` to an array that the library can store the packed telemetry data into. The user should also specify the size of this array buffer using the `flash_arr_sz` variable. After the library is done packing the data, it returns the size of the packet after packing in `flash_arr_used`.

```
// Send Data Info
typedef struct CLB_send_data_info {
	UART_HandleTypeDef* uartx;  // UART channel
	int16_t flash_arr_sz;       // Size of flash_arr
	int16_t flash_arr_used;     // # bytes packed into flash_arr
	uint8_t *flash_arr;         // Array pointer to store packed data
} CLB_send_data_info;
```

## Autogeneration Files

In order to use this library properly, there are a few modifications that you will need to make to two configuration csv files. After making these modifications, you will need to call two python scripts and copy over the autogenerated files. I will describe these steps in more detail below.

1. To modify what data is included in the default telemetry packet, you will need to modify the `SerialComms/python/telem_data_template.csv` file. Specifically, you should modify the `should_generate` column. Writing a *y* in this column indicates that you want the current row to be included in the telemetry packet. Subsequently, writing a *n* implies the opposite. If you feel that a telemetry item is not currently available that you will need, let someone from avionics know and they will make this change for you manually. Finally, the last part you will need to update is the row with the name Own Board Addr. You will need to set the min and max value columns in this row to the address of your board. Failure to do this will result in undefined behavior when receiving telemetry. For people developing firmware that will run specific boards, you will have to create a copy of the template csv file with the name `telem_data_youboardnamehere.csv`. Note: the data is sent in byte sized chunks over RS-422, which means that if you select a telemetry point that is x bits, you will need to select ceil(x/8) rows. 

2. To add/enable commands that can be processed by the target board, you will need to modify the `SerialComms/python/telem_cmd_template.csv` file. If you need to add a new command, please update the `packet_type` column for the new function to be the next consecutive number in the list. Note: packet types 0-7 are reserved for critical functions that must be included on all boards. At this point, the only critical function implemented is 0, which sends a full telem packet to the target board. In addition, you must add to the `supported_target_addr` column your board addr. If you are confused as what this, please refer to point 1. 

3. After modifying the config files, navigate to the `SerialComms/python` directory. Then, call `telem_file_generator.py telem_data_template.csv` to generate the corresponding `globals.c`, `globals.h`, `pack_telem_defines.c`, and `pack_telem_defines.h` files.  You will need to copy and paste the autogenerated portion of the `globals.c` and `globals.h` into the `${Project_DIR}/Core/Src` and `${Project_DIR}/Core/Inc` directories respectively. Remember to delete the globals.c and globals.h files after copy and pasting their contents to avoid compilation errors. The other two files do not need to be touched. 

For convenience, there is a Makefile in the Python directory, if you run `make BOARD_NAME_HERE` while in that directory, the Makefile will automatically run the parser script and autogenerate the output files to the correct directory in your current project and also in the GUI project. However, you will need to export a `GUI_PATH` environment variable in your PATH. This will point to the path of your GUI project directory, or any arbitrary path on your computer if you do not have the GUI git repo cloned. An explanation for how to do this can be found online. **I highly recommend setting this up if you plan on frequently using the comms library.**

4. If you plan using a variable to keep of board state, make sure to select the STATES line in the `telem_data_template.csv` file and copy and paste this defintion into the user section of the autogenerated file `globals.h`. **Note: you may need to modify this struct to better suit your board's needs.**

5. If your board will need to use various calibrations, you will need to add these calibration to the `calibrations.csv` file. Before doing this, be sure to consult someone from ATLO to ensure that the calibration parameters are correct before doing so.

```
enum States {
    Idle    = 0,
    Armed   = 1,
    Running = 2,
    Stop    = 3
};
```

5. After following this setup, you will be able to use the sample comms library code below for transmitting the default telem packet.

# Sample Comms Library Code

The sample code references a `SYS_MICROS` variable. We assume that you have setup a timer to increments once every microsecond in the ioc file. After doing doing this, simply create a macro for getting the timer counter shown like below with your timer.

```
// You should configure timer 5 in the .ioc file to increment once per uS
#define SYS_MICROS      (uint32_t)((__HAL_TIM_GET_COUNTER(&htim5))
```

## Sample code for transmitting default telem packet (COBS encoded)
```
CLB_Packet_Header header;
header.packet_type = 0; // default packet
header.origin_addr = your_board_addr_here; 
header.target_addr = target_board_addr_here; // computer
header.priority = 1; // medium
header.do_cobbs = 1; // enable cobbs
header.timestamp= SYS_MICROS; // change this to micros
init_data(NULL, -1, &header);   // pack all telem data
CLB_send_data_info info;
info.uartx = your_uart_channel_here;
send_data(&info, CLB_Telem);
```

## Sample code for transmitting custom telem packet
```
// You should configure timer 5 in the .ioc file to increment once per uS
#define SYS_MICROS      (uint32_t)((__HAL_TIM_GET_COUNTER(&htim5))

CLB_Packet_Header header;
header.packet_type = 0; // default packet
header.origin_addr = your_board_addr_here; 
header.target_addr = target_board_addr_here;
header.priority = 1; // medium
header.do_cobbs = 1; // enable cobbs
header.timestamp= SYS_MICROS;
uint8_t buffer[10] = {0};
int16_t buffer_sz = 10;
init_data(buffer, buffer_sz, &header);
CLB_send_data_info info;
info.uartx = your_uart_channel_here;
send_data(&info, CLB_Telem);
```

## Sample code for receiving custom telem packet over DMA

Please keep in mind that once a telem buffer is received, it is up to the programmer to declare functions for command handling. Writing functions to handle commands is a process that will be covered in the later sections. If this code seems out of date or has issues, it is most likely because we are in the middle of transitioning towards using dma.

### main.c
```
// DMA
#define DMA_RX_BUFFER_SIZE          2048
uint8_t DMA_RX_Buffer[DMA_RX_BUFFER_SIZE];

#define NUM_BUFFER_PACKETS          10
#define CIRCULAR_TELEM_BUFFER_SZ    PONG_MAX_PACKET_SIZE*NUM_BUFFER_PACKETS
uint8_t circular_telem_buffer[CIRCULAR_TELEM_BUFFER_SZ];
uint8_t temp_telem_buffer[PONG_MAX_PACKET_SIZE] = { 0 };
uint8_t daisy_telem_buffer[CIRCULAR_TELEM_BUFFER_SZ] = { 0 };
volatile int16_t curr_circular_buffer_pos               = 0;
volatile int16_t curr_telem_start[NUM_BUFFER_PACKETS]   = { 0 };
volatile int16_t curr_telem_len[NUM_BUFFER_PACKETS]     = { 0 };
volatile int16_t last_telem_packet_pos                  = 0;

...

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    static uint8_t RxRollover  = 0;
    static uint16_t RxBfrPos   = 0;
    // UART Rx Complete Callback;
    // Rx Complete is called by: DMA (automatically), if it rolls over
    // and when an IDLE Interrupt occurs
    // DMA Interrupt allays occurs BEFORE the idle interrupt can be fired because
    // idle detection needs at least one UART clock to detect the bus is idle. So
    // in the case, that the transmission length is one full buffer length
    // and the start buffer pointer is at 0, it will be also 0 at the end of the
    // transmission. In this case the DMA rollover will increment the RxRollover
    // variable first and len will not be zero.
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE)) {                    // Check if it is an "Idle Interrupt"
        __HAL_UART_CLEAR_IDLEFLAG(huart);                             // clear the interrupt

        uint16_t start = RxBfrPos;                                        // Rx bytes start position (=last buffer position)
        RxBfrPos = DMA_RX_BUFFER_SIZE - (uint16_t)huart->hdmarx->Instance->NDTR;// determine actual buffer position
        uint16_t len = DMA_RX_BUFFER_SIZE;                                // init len with max. size

        if(RxRollover < 2)  {
            if(RxRollover) {                                                        // rolled over once
                if(RxBfrPos <= start) {
                    len = RxBfrPos + DMA_RX_BUFFER_SIZE - start;  // no bytes overwritten
                } else {
                    len = DMA_RX_BUFFER_SIZE + 1;                 // bytes overwritten error
                }
            } else {
                len = RxBfrPos - start;                           // no bytes overwritten
            }
        } else {
            len = DMA_RX_BUFFER_SIZE + 2;                         // dual rollover error
        }

        if(len && (len <= DMA_RX_BUFFER_SIZE)) {
            uint16_t bytes_in_first_part = len;
            uint16_t bytes_in_second_part = 0;
            if (RxBfrPos < start) { // if data loops in buffer
                bytes_in_first_part = DMA_RX_BUFFER_SIZE - start;
                bytes_in_second_part= len - bytes_in_first_part;
            }

            // handle telem for yourself immediately
            memcpy(temp_telem_buffer, DMA_RX_Buffer+start, bytes_in_first_part);
            memcpy(temp_telem_buffer+bytes_in_first_part, DMA_RX_Buffer, bytes_in_second_part);
            uint8_t cmd_status = receive_data(huart, temp_telem_buffer, len);

            // handle telem for others in buffer
            /*
            if (cmd_status == CLB_RECEIVE_DAISY_TELEM) {
                curr_telem_start[last_telem_packet_pos]= curr_circular_buffer_pos;
                copyDataToBuffer(temp_telem_buffer, len);
                curr_telem_len[last_telem_packet_pos]  = len;
                last_telem_packet_pos = (last_telem_packet_pos + 1) % NUM_BUFFER_PACKETS;
            }
            */
        } else {
            // buffer overflow error:
            HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
			init_board(OWN_BOARD_ADDR); //Fixes an issue with CLB_board_addr changing
			last_telem_packet_pos = 0;
			HAL_UART_Receive_DMA(&COM_UART, DMA_RX_Buffer, DMA_RX_BUFFER_SIZE); // dma buffer overflowed
        }

        RxRollover = 0;                                                    // reset the Rollover variable
    } else {
        // no idle flag? --> DMA rollover occurred
        RxRollover++;       // increment Rollover Counter
    }
}

void enableDMA() {
    // UART DMA
    __HAL_UART_ENABLE_IT(&COM_UART, UART_IT_IDLE);   // enable idle line interrupt
    HAL_UART_Receive_DMA(&COM_UART, DMA_RX_Buffer, DMA_RX_BUFFER_SIZE);
}

int main() {
    Auto generated init functions
    ...
    enableDMA();

    while (1) {
        // initialize 
    }
}
```

### stm32f4xx_it.c
Note: you should modify the UART channel that you configured for dma (this may not be uart6)
```
/**
  * @brief This function handles USART6 global interrupt.
  */
void USART6_IRQHandler(void)
{
  /* USER CODE BEGIN USART6_IRQn 0 */

  /* USER CODE END USART6_IRQn 0 */
  HAL_UART_IRQHandler(&huart6);
  /* USER CODE BEGIN USART6_IRQn 1 */
  if (__HAL_UART_GET_FLAG(&huart6, UART_FLAG_IDLE)) {
    HAL_UART_RxCpltCallback(&huart6);
  }

  /* USER CODE END USART6_IRQn 1 */
}
```

## Sample code for saving telem packet to flash
```
CLB_Packet_Header header;
header.packet_type = 0; // default packet
header.target_addr = 7; // computer
header.origin_addr = your_board_addr_here;
header.priority = 1; // medium
header.do_cobbs = 1; // enable cobs
header.timestamp= SYS_MICROS;
init_data(NULL, -1, &header);   // pack all telem data

// packs data to flash
uint8_t buffer[253] = {0};
CLB_send_data_info info;
info.flash_arr_used = 0; 
info.flash_arr_sz = 253; 
info.flash_arr  = buffer;
send_data(&info, CLB_Flash);

uint8_t buffer_sz = info.flash_arr_used;

// Write to Flash flash disabled for now
write_to_flash(flash, buffer, buffer_sz);
```

## Handling reception of custom commands

The list of commands currently available to the board can be found in the `pack_cmd_defines.h` file in the firmware-libraries/SerialComms/inc/ directory. These commands and the order in which their arguments are in are defined in the firmware-libraries/SerialComms/python/ directory. In addition, we have developed custom scripts for autogenerating the `pack_cmd_defines.h` file as well as the `telem.c` file if you would like to add more commands. the `telem.c` file contains a list of all available function as well as function arguments that are initialized at the beginning of each function. 

It is the programmers job to actually implement additional functionality for each of these commands. In addition, the `telem.c` should be placed in the ${Project_Directory}/Core/src directory in order allow easy access to global variables. It will be common practice to have to include external global variables from the main.c file in order to adequately handle most commands.
             
## Developer Guide

### Test Procedure

To verify that you can properly send packets, repeat the following test procedures. 

The most basic test is to verify that you can receive a telemetry packet. To do so, simply connect the board to the server by running `python server.py` (or `python3 server.py` if your default python folder is not python), which can be found in the gui repo. Then, just make sure that the packet size received matches the packet size that you have set and that the values look reasonable.

A simply test to check if commands are going through by sending a command to blink an led on your board. This is left an exercise to the developer.
