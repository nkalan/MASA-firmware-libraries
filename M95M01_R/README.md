# User Guide (Last updated April 4, 2021)

* [M95M01-R datasheet](https://www.st.com/resource/en/datasheet/m95m01-r.pdf)

### Usage notes for user

* The eeprom chip is split up into 256-Byte pages. If the user writes to a specific address in a page with enough data that the data will run past the end of the page, the chip will "rollover" to the beginning of the page  
    Example: If writing 10 bytes to address 0xFC, the first 4 bytes will be written in addresses 0xFC - 0xFF. Then, since 0xFF is the end of the page, the remaining 6 bytes will "rollover" to addresses 0x00 - 0x05.


### Suggested SPI settings
Note: the chip has a maximum clock speed of 16MHz. The prescalar shoudl be adjusted accordingly.   
The chip can take two different SPI settings (specified in datasheet), but these are the settings tested on.
* Mode: master
* Direction: 2 line
* Frame Format: Motorola
* Data Size: 8 bits
* First Bit: MSB First
* Clock Polarity: Low
* Clock Phase: 1 Edge

### CS (Chip select) Pin settings
Note: These settings worked for testing. Leif does not know if these are the best settings.
* GPIO mode: Output push-pull
* GPIO pull: no pull
* GPIO speed: low

### Possible additions to make to library

* Error handling
