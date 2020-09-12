# W25N01GV Flash Memory
This repository contains the MASA firmware library for the W25N01GV flash memory IC

# Quick Start
## Testing if Flash Works
1) Place `W25N01GV.h` in the `/Core/Inc` folder of your project, and place `W25N01GV.c` in the `/Core/Src` folder. Easiest way to do this is copy and paste (at least at this point in time).
3) `#include "W25N01GV.h"` in main.c
2) Initialize a `W25N01GV_Flash` struct and run the `is_flash_id_correct()` function, which returns true if SPI commands are correctly being sent and received.
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
## Checking for Bad Memory Blocks
Up to 20 out of the 1024 memory blocks are allowed to be defective when shipped. This is marked at the first byte of every defective block, so you need to scan each block before erasing and overwriting data. To do this, use the function `scan_bad_blocks()` after initialization.
```
uint16_t bad_block_addresses[1024];
uint16_t num_bad_blocks = scan_bad_blocks(&flash, bad_block_addresses);
```
Ideally, `scan_bad_blocks()` should return `0` and `bad_block_addresses` should be unchanged, which is what I found while testing, but if it isn't then @ me on Slack and I'll figure something out.
# Implementing W25N01GV Firmware
`TODO: write guide on implementing firmware`