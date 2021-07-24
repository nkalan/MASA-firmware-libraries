# L6470 stepper motor IC library User Guide (Last updated Jul 23 2021)

TODO: write better readme

[L6470 Datasheet](https://www.st.com/content/ccc/resource/technical/document/datasheet/a5/86/06/1c/fa/b2/43/db/CD00255075.pdf/files/CD00255075.pdf/_jcr_content/translations/en.CD00255075.pdf)

[EVAL6470H dev board Datasheet](https://www.st.com/en/motor-drivers/l6470.html)

[STM32 firmware library for dSPIN L6470](https://www.st.com/en/embedded-software/stsw-spin004.html)

[SM-17HS4023 Motor Datasheet (for connecting the motor)](https://html.alldatasheet.com/html-pdf/1179365/ETC/SM-17HS4023/139/2/SM-17HS4023.html)

## SPI configuretion
* Frame Format: Motorola
* Data Size: 8 bits
* First Bit: MSB First
* Maximum Serial Clock Frequency: 5MHz

* Configuration for STM32CudeIDE:
* Prescaler: 16
* CPOL: High
* CPHA: 2 Edge

## Speed and its conversions

Starting on datasheet pg. 42, there are a bunch of formulas for converting between step/tick and step/s, which is what the chip uses. For a more user-friendly interface, we use degree/sec. This is where the step angle (degrees per step), as pass into `L6470_init_motor`, comes into play.
- `L6470_goto_motor_pos` runs at the maximum speed, which is in register h07; it is default h41 (991.8 step/s) and can be changed 
* To convert: step/tick = deg_sec / 1.8 / 15.25 (which is 27.45).
* Max is 1023 step/tick = 15610 step/s = 28098 degree/s (datasheet)
