# Important Information for Firmware Developers
Untested firmware changes should be pushed to their own branches. For instance, a change made to the MAX11131 IC should be pushed to the branch `release-max11131-dev`. In general the naming convention for branches is:

`feature-IC_name_here-dev`

Once the code on that branch is thoroughly tested on its feature branch, maintainer will merge into master.
The master branch should remain stable for use at all times.

## Structuring your new firmware library

It is extremely important to structure your firmware project using the steps below. Failure to do so will result in unique complications for others who want to use your firmware library, and no one likes that. 

The filestructure for your project should look like this:

### Example file structure for firmware library

```
IC_NAME/     
+--inc/      
    +--IC_NAME.h       
    +--*additional header files for project here*      
+--src/     
    +--IC_NAME.c       
    +--*additional c files for project here*       
+--README.md       
```
`IC_NAME` should be the exact name of your IC's part number (P/N) specified on the data sheet. Note that P/Ns generally start with 2-3 letters, and aren't just numbers. This ensures that firmware libraries are easy to find and use in projects. In addition, all header files for your library should be placed in the nested inc directory, and all c files should be placed in the nested src directory. Failure to do this will result in complications using your library in projects. Additionally, a README.md files is highly recommended for an firmware library, as it will inform the user of any information required to use your library in projects.

Remember to create your firmware library folder at the top level of the firmware-libraries directories. Happy firm-ing!

# Important Information for Firmware Users

## Setting up STM32CubeIDE project to use firmware libraries

1. From the project root directory on the cli, enter `cd Core`. This will take you to the correct directory to clone the firmware libraries.

2. Enter `git submodule add https://gitlab.eecs.umich.edu/masa/avionics/firmware-libraries.git`, and you should see the firmware-libraries directories appear in PROJECTROOT/Core.

3. On STM32Cube IDE, go to `Project->Properties->C/C++ Build->Settings->Tool Settings->MCU GCC Compiler->Include paths`. Here you should be able to click a small paper icon with a green plus sign underneath a box called `Include paths (-I)`. Click on this box and enter the filepath to the includes files for your desired IC in the firmware-libraries directory. Remember to hit apply changes when done. Note: the filepath may look something like `/Users/arthur/Documents/MASA/firmware-nucleo-tests/Core/firmware-libraries/MAX11131/inc`

4. Once you're finished adding the inc directory for your IC to the project, the compiler will be able to properly link your firmware library's *.c files to the required *.h files. 

5. In the main.h file for your project, simply add the main header for your IC inside the user include guards. You should be adding something that looks like `#include FOO.h`, where FOO is the name of header file for your IC.

6. Now you're all set to use your firmware library in your project's main.c file!
