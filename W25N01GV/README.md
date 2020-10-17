# W25N01GV Flash Memory
This repository contains the MASA firmware library for the W25N01GV flash memory IC.

## Quick Start: Testing if Flash Works
1) Place `W25N01GV.h` in the `/Core/Inc` folder of your project, and place `W25N01GV.c` in the `/Core/Src` folder. Easiest way to do this is copy and paste (at least at this point in time). When it gets merged to the master branch, use the `git submodule` method described in the top-level directory's README.
3) `#include "W25N01GV.h"` in main.c
2) Initialize a `W25N01GV_Flash` struct and run the `is_flash_id_correct()` function, which returns `1` if SPI commands are correctly being sent and received.
3) Make sure SPI is at less than 100MHz, uses 8bit framing, MSB first, CPOL low, and CPHA 1 edge
```
#include "W25N01GV.h"
:
:
int main(void)
{
    :
    :
    W25N01GV_Flash flash;
    init_flash(&flash, &hspi2, GPIOB, GPIO_PIN_5);
    // Make sure you use the right spi bus and CS pin
    
    while(1) {
        bool flash_working = is_flash_id_correct(&flash);
        // Check if it's true and blink an LED or something
    }
}
```
## SPI Configuration
`TODO`

## Sample Code
This section provides samples of code to execute the functions provided in the .h file.
### Initialization
Always run the `init_flash()` function first, using the correct `SPI_HandleTypeDef` struct, `GPIO_TypeDef` pin array, and `uint16_t` pin number.
```
W25N01GV_Flash flash;
init_flash(&flash, &<spi_bus_name>, <GPIO_array>, <GPIO_pin>);
```
### Reading from Flash
Read the entire flash memory array in 2KB chunks at a time.
NOTE: The total number of pages available to read is one greater than the maximum value of a `uint16_t`, so you either have to declare the page counter with at least 32bits or break out of the loop when the counter reaches `NUM_PAGES`, or use some other control logic to avoid an infinite loop.
```
uint32_t page = 0;
uint8_t read_buffer[2048];
reset_flash_read_pointer(&flash);

while (page < NUM_PAGES) {
    read_next_2KB_from_flash(&flash, read_buffer);
    // Data gets read into read_buffer, do something with it here
}
```
### Writing to Flash
Write an array of `uint8_t` bytes to flash.
NOTE: If there isn't enough space to write the data, anything over capacity will get cut off and won't be written.
```
// data is a uint8_t array, num_bytes is the number of elements in data
write_to_flash(&flash, data, num_bytes);
```
### Checking SPI Functionality
You can check if you can successfully send and receive data to flash over the SPI bus by using `is_flash_id_correct()` to "ping" flash. This sample turns an LED on if it's successful, and turns it off if it's not. The GPIO pin array and number used here are the ones for the green onboard LED on the STM32F446RE Nucleo board.
```
while (1) {
    if (is_flash_id_correct(&flash)) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
    }
    else {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    }
}
```
### Checking for Bad Memory Blocks
Up to 20 out of the 1024 memory blocks are allowed to be defective when shipped. This is marked at the first byte of every defective block, so you need to scan each block before erasing and overwriting data. To do this, use the function `scan_bad_blocks()` after initialization. If `scan_bad_blocks()` returns a nonzero value, there is at least 1 corrupted memory block and you should consider using a different chip.
```
uint16_t bad_block_addresses[1024];
uint16_t num_bad_blocks = scan_bad_blocks(&flash, bad_block_addresses);
```