# User Guide

* [MAX3491 Technical Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX3483-MAX3491.pdf)

## Pin Terminology

* Not required knowledge for firmware development

## Board to Computer Hardware Setup Details

### MAX3491 RS422 Chip Hardware pinouts

A common mistake is to incorrectly flip the RJ-45 ethernet cable pins and the MAX3491 chip. On the current RS422 to USB converter board that was designed by Sternie (a previous MASA member), the TX- and TX+ pins on the custom board should be connected to the RX+ and RX- pins on the RJ45 connector. The suspected reason for this is because the MAX3491 chip inverts one of the signals for the differential pair, and we need to replicate this inversion by connecting our TX+- to the opposite polarity RX-+ pairs.

## Configuring UART Settings

A higher baud rate can be selected for this library. However, if this is done, please ensure that the clock configuration for UART is at least 16 times faster than the baud rate to ensure proper behavior.

Under the Parameter Settings tab in the .ioc file, use the following configuration

* Mode: Asynchronous
* Hardware Flow Control (RS232): Disable
* Baud Rate: 115200 Bits/s
* Word Length: 8 Bits
* Parity: None
* Stop Bits: 1
* Data Direction: Receive and Transmit
* Over Sampling: 16 times

Under the NVIC Settings tab in the .ioc file, use the following configuration

* USART2 global interrupt: Enabled

## Configuring Hardware Pinouts

Write things here about connecting to the computer and other boards

## Comms Library Setup Struct Explanation

The packet header needs to specified before any commands are transmitted using this library. The packet header will contain essential information, such as the `packet_type`, which will be used to identify how this packet should be handled once it is received. The packet type definitions can be found in the /firmware-libraries/SerialComms/python/packet_telem_template.csv file. 

In addition, the `target_addr` will be used in the future for allowing you to address specific boards or specify which board transmitted the message. 

The `priority` byte can be used to set different priorities for each telem packet received if multiple are received in parallel. However, all mission critical commands should be notified using an external gpio interrupt instead to guarantee timely handling.

Finally, the `timestamp` variable is used to specify a 32 bit unsigned integer that counts the time in milliseconds that have elapsed since the board thas started up. It is important to keep the counts of this timestamp in milliseconds to ensure consistent behavior across all boards. 

Note: with a 32 bit unsigned integer for timestamps, we have log about 1100 hours, which is more than enough range.

```
// Packet Header 
typedef struct CLB_Packet_Header {
    uint8_t packet_type;        // CMD/DATA packet ID
    uint8_t target_addr;        // target board address
    uint8_t priority;           // priority of packet
    uint16_t checksum;          // checksum to ensure robustness (generated)
    uint32_t timestamp;         // timestamp for data
} CLB_Packet_Header;
```

## Autogeneration Files

## Sample Comms Library Transmit Code

### Sample code for transmitting default telem packet
```
CLB_Packet_Header header;
header.packet_type = 0;
header.target_addr = 0;
header.priority = 1;
header.timestamp = HAL_GetTick();
Packet_Config packet_config;
packet_config.do_cobbs = 1;
init_data(NULL, -1, header, packet_config);
uint8_t transmission_status = send_data(&huart2);
```

Sample code for transmitting custom telem packet
```
CLB_Packet_Header header;
header.packet_type = 1; // note the packet type is defined in the 'packet_telem_defines.h' file
header.target_addr = 0;
header.priority = 1;
header.timestamp = HAL_GetTick();
Packet_Config packet_config;
packet_config.do_cobbs = 1;
uint8_t buffer[10] = {0};
int16_t buffer_sz = 10;
init_data(buffer, buffer_sz, header, packet_config);
uint8_t transmission_status = send_data(&huart2);
```

### Sample code for receiving custom telem packet

Please keep in mind that once a telem buffer is received, it is up to the programmer to declare functions for command handling. Writing functions to handle commands is a process that will be covered in the later sections.

// initialize as global variable
uint8_t last_byte_uart;
volatile uint16_t telem_buffer_sz = 0;

// put into arbitrary intialization function
...other code for one time function calls...
HAL_UART_Receive_IT(&huart2, &last_byte_uart, 1);

// place into main.c
// Note: huartx should whichever uart peripheral in which you
//          receive telemetry packets
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huartx) {
        HAL_UART_Receive_IT(&huartx, &last_byte_uart, 1);
        telem_buffer[telem_buffer_sz++] = last_byte_uart;
    }
}

// place into main while loop
// EXPECTED_TELEM_BUFFER_SZ depends on how long the telem packet is
if (telem_buffer_sz == EXPECTED_TELEM_BUFFER_SZ) {
    receive_data(&huartx, telem_buffer, telem_buffer_sz);
    telem_buffer_sz = 0;
}

## Handling reception of custom commands

The list of commands currently available to the board can be found in the `pack_cmd_defines.h` file in the firmware-libraries/SerialComms/inc/ directory. These commands and the order in which their arguments are in are defined in the firmware-libraries/SerialComms/python/ directory. In addition, we have developed custom scripts for autogenerating the `pack_cmd_defines.h` file as well as the `telem.c` file if you would like to add more commands. the `telem.c` file contains a list of all available function as well as function arguments that are initialized at the beginning of each function. 

It is the programmers job to actually implement additional functionality for each of these commands. In addition, the `telem.c` should be placed in th ${Project_Directory}/Core/src directory in order allow easy access to global variables. It will be common practice to have to include external global variables from the main.c file in order to adequately handle most commands.

We will now describe how to call these scripts to autogenerate these files for you. TODO: Ask nathaniel and marshall how to call and use their scripts
             
## Developer Guide

### Test Procedure

To verify that you can properly send packets, repeat the following test procedures TODO:
