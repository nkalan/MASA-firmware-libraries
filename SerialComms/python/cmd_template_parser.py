"""
Author: Marshall Stone (syzdup)

Description: 

Autogenerates 3 files (pack_cmd_defines.h, pack_cmd_defines.c, telem.c),
cmd_template_parser.py takes in three arguments:    --f "filename" 
                                                    --n "board number"
                                                    --o "output file directory"
Usage: 

python3 cmd_template_parser.py -f telem_cmd_template.csv -n 0 -o "../../../Src
"""
from os import path
import pandas as pd
import numpy as np
import argparse
import sys

#writes a function to the c file
def function_writer(row_number):
    #selects entire row (function along with all args and argtypes)
    function_name = functions.iloc[row_number]
    num_args = int(function_name[2])
    c_file.write("void " + function_name[1] + "(uint8_t* data, uint8_t* status){\n\n\t")
    col_num = 4
    data_num = 0
    for x in range(num_args): 
        c_file.write(function_name[col_num] + " " + function_name[col_num - 1] + " = ")
        if function_name[col_num] == "uint8_t":
            try:
                c_file.write("(data[" + str(data_num) 
                                + "])/" + str(int(function_name[col_num + 1])) 
                                + ";\n\t")
                data_num += 1
            except ValueError:
                c_file.write("(data[" + str(data_num) + "])/" + "1;\n\t")
                data_num += 1
        elif function_name[col_num] == "uint16_t":
            try:
                c_file.write("(data[" + str(data_num + 1) 
                                + "]<<data[" + str(data_num) 
                                + "])/" + str(int(function_name[col_num + 1])) 
                                + ";\n\t")
                data_num += 2
            except ValueError:
                c_file.write("(data[" + str(data_num + 1) 
                            + "]<<data[" + str(data_num) + "])/" + "1;\n\t")
                data_num += 2
        elif function_name[col_num] == "uint32_t":
            try:
                c_file.write("(data[" + str(data_num + 3) 
                                + "]<<data[" + str(data_num + 2) 
                                + "]<<data[" + str(data_num + 1) 
                                + "]<<data[" + str(data_num + 0) + "])/" 
                                + str(int(function_name[col_num + 1])) + ";\n\t")
                data_num += 4
            except ValueError:
                c_file.write("(data[" + str(data_num + 3) 
                                + "]<<data[" + str(data_num + 2) 
                                + "]<<data[" + str(data_num + 1) 
                                + "]<<data[" + str(data_num + 0) 
                                + "])/" + "1;\n\t")
                data_num += 4
        elif function_name[col_num] == "float":
            try:
                c_file.write("(data[" + str(data_num + 3) 
                                + "]<<data[" + str(data_num + 2) 
                                + "]<<data[" + str(data_num + 1) 
                                + "]<<data[" + str(data_num + 0) + "])/" 
                                + str(float(function_name[col_num + 1])) + ";\n\t")
                data_num += 4
            except ValueError:
                c_file.write("(data[" + str(data_num + 3) 
                                + "]<<data[" + str(data_num + 2) 
                                + "]<<data[" + str(data_num + 1) 
                                + "]<<data[" + str(data_num + 0) 
                                + "])/" + "1.0;\n\t")
                data_num += 4
        else:
            try:
                c_file.write("(data[" + str(data_num) + "])/" + str(int(function_name[col_num + 1])) + ".0;\n\t")
                data_num += 1
            except ValueError:
                c_file.write("(data[" + str(data_num) + "])/" + "1;\n\t")
                data_num += 1

        col_num += 3
    c_file.write("\n}\n")





#argument parsing
parser = argparse.ArgumentParser(
    description="Command Template Parser Script"
)

parser.add_argument('-f', '--file_name', help="CSV Filename", type=str)
parser.add_argument('-n', '--board_num', help="Board number", type=str)
parser.add_argument('-o', '--output_dir', help="Output Directory", type=str)

args = parser.parse_args()
file_name = args.file_name
board_num = args.board_num
output_dir= args.output_dir

try:
    functions = pd.read_csv(file_name)
except FileNotFoundError:
    print("Input file not found. Exiting now.")
    sys.exit()

#looks for keyword user and reads to end of file (FOR H FILE MAIN)
user_generated_code_h = ""  
line_num_h = 0 
try:
    with open("../../../Inc/pack_cmd_defines.h", 'r') as h_file:
        file_read = h_file.readlines()
        for line in file_read:
            line_num_h += 1
            if "user" in line:
                user_generated_code_h = file_read[line_num_h-1:]
except FileNotFoundError:
    pass

#write to header file
with open("../../../Inc/pack_cmd_defines.h", 'w') as header_file:

    #grab list of function names from csv
    function_names = functions['function_name']
    function_num = str(function_names.count())
    

    #defines and includes and stuff for beginning of h file go here
    header_file.write("#ifndef PACK_CMD_DEFINES_H\n#define PACK_CMD_DEFINES_H\n#define NUM_CMD_ITEMS " + 
                      function_num + "\n#include <stdint.h>\n\n")


    #for each function name in file, write out a function definition
    board_supported = functions['supported_target_addr']
    function_point = 0
    for name in function_names:
        if board_num in str(board_supported[function_point]):
            try:
                header_file.write("void " + name + "(uint8_t* data, uint8_t* status);\n\n")
                function_point += 1
            except TypeError:
                #skips over nan values 
                pass

    header_file.write("typedef void (*Cmd_Pointer)(uint8_t* x, uint8_t* y);\n\n")
    header_file.write("Cmd_Pointer cmds_ptr[NUM_CMD_ITEMS];\n\n")
    header_file.write("// Note: to call a function do\n/**\n* (*cmds_ptr[0])(array ptr here)\n"
                      "*\n* The actual cmd functions will be defined in a separate c file by the firwmare\n"
                      "* developer for each board. They simply need to include this header file\n* in the c" 
                      "file in which they define the function. This allows the developer\n* to import/use any"
                      " variables or typedefs from the hal library. This file is\n* simply the jumptable that"
                      " gives the comms library an easy way to call\n*" 
                      " custom functions without additional knowledge of where this file is defined\n*\n*/")

    #write closing bracket and endif to file
    header_file.write("\n\n\n#endif\n")

    #write user generated code to h file
    line_index_h = 0
    for line in user_generated_code_h:
        header_file.write(user_generated_code_h[line_index_h])
        line_index_h += 1
    print("Finished generating pack_cmd_defines.h...")




#check for user code in pointer file
user_generated_code_pointer = ""  
line_num_pointer = 0 
try:
    with open("../../../Src/pack_cmd_defines.c", 'r') as pointer_file:
        file_read_pointer = pointer_file.readlines()
        for line in file_read_pointer:
            line_num_pointer += 1
            if "user" in line:
                user_generated_code_pointer = file_read_pointer[line_num_pointer-1:]
except FileNotFoundError:
    pass

#write to pointer file
with open("../../../Src/pack_cmd_defines.c", 'w+') as header_c_test:
    header_c_test.write("#include \"pack_cmd_defines.h\"\n")
    header_c_test.write("Cmd_Pointer cmds_ptr[NUM_CMD_ITEMS] = {\n\n")
    function_name_index = 0
    board_supported = functions['supported_target_addr']
    for  name in function_names:
        #fix check for board support 
        if board_num in str(board_supported[function_name_index]):
            try:
                if int(function_names.count()-1) != int(function_name_index):
                    header_c_test.write(name + ",\n")
                    function_name_index += 1
                else:
                    header_c_test.write(name + "\n")

            except TypeError:
                #skips over nan values
                pass
    header_c_test.write("};\n\n")

    #write user gen code to pointer file
    line_index_pointer = 0
    for line in user_generated_code_pointer:
        header_c_test.write(user_generated_code_pointer[line_index_pointer])
        line_index_pointer += 1
    print("Finished generating pack_cmd_defines.c...")

#looks for keyword user and reads to end of file (FOR C FILE MAIN)
user_generated_code = ""  
line_num = 0 
try:
    with open(path.join(output_dir, 'telem.c'), 'r') as c_file:
        file_read = c_file.readlines()
        for line in file_read:
            line_num += 1
            if "user" in line:
                user_generated_code = file_read[line_num-1:]
except FileNotFoundError:
    print("Creating new telem.c file in output directory...")
    pass         

#write out c file template
with open(output_dir+"/telem.c", 'w') as c_file:
    board_supported = functions['supported_target_addr']
    
    #write functions from csv file to c_file
    c_file.write("#include <stdint.h>\n\n")
    for x in range(int(function_num)):
        if board_num in str(board_supported[x]):
            function_writer(x)

    #write user generated code to c_file
    line_index = 0
    for line in user_generated_code:
        c_file.write(user_generated_code[line_index])
        line_index += 1
    print("Finished generating telem.c...\n")
