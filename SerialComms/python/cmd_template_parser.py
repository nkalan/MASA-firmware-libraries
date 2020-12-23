import pandas as pd
import numpy as np

#print("CSV Filename (include .csv): ", end='')
#file_name = input()

#Change to filename when not in dev

functions = pd.read_csv("telem_cmd_template.csv")
#print("Board number: ", end='')
#board_num = input()

with open("headerTest.h", 'w') as header_file:

    #Grab list of function names from csv
    function_names = functions['function_name']
    function_num = str(function_names.count())
    

    ###Defines and includes and stuff for beginning of h file go here
    header_file.write("#ifndef PACK_CMD_DEFINES_H\n#define PACK_CMD_DEFINES_H\n#define NUM_CMD_ITEMS " + function_num + "\n#include <stdint.h>\n\n")


    ###For each function name in file, write out a function definition
    for name in function_names:
        try:
            header_file.write("void " + name + "(uint8_t* data, uint8_t* status);\n\n")
        except TypeError:
            #Skips over nan values (empty csv cells)
            pass


    ###Writes pointer 
    header_file.write("typedef void (*Cmd_Pointer)(uint8_t* x, uint8_t* y);\n\n")
    header_file.write("static Cmd_Pointer cmds_ptr[NUM_CMD_ITEMS] = {\n")
    

    ###This part will write function names in list
    function_name_index = 0
    for  name in function_names:
        try:
            if int(function_names.count()-1) != int(function_name_index):
                header_file.write(name + ",\n")
                function_name_index = function_name_index + 1
            else:
                header_file.write(name + "\n")

        except TypeError:
            #Skips over nan values
            pass

    #Write closing bracket and endif to file
    header_file.write("};\n\n\n#endif")




#Autogenerate function headers for .c file 
#c_file template example:
#void digital_write(uint8_t* data, uint8_t* status) {
  #uint16_t port_number = (data[0]<<8|data[1])/xmit scale;
  #uint16_t pin_number = (data[2]<<8|data[3])/xmit scale;
  #uint8_t state = (data[4])/xmit scale;
#}
#TODO:
#user generated portion check for string "user" 

def function_writer(row_number):
    #Selects entire row (function along with all args and argtypes)
    function_name = functions.iloc[row_number]
    num_args = int(function_name[2])
    c_file.write("void " + function_name[1] + "(uint8_t* data, uint8_t* status){\n\n\t")
    col_num = 4
    data_num = 0
    for x in range(num_args): 
        c_file.write(function_name[col_num] + " " + function_name[col_num - 1] + " = ")
        if function_name[col_num] == "uint8_t":
            try:
                c_file.write("(data[" + str(data_num) + "])/" + str(int(function_name[col_num + 1])) + "\n\t")
                data_num += 1
            except ValueError:
                c_file.write("(data[" + str(data_num) + "])/" + "1\n\t")
                data_num += 1
        elif function_name[col_num] == "uint16_t":
            try:
                c_file.write("(data[" + str(data_num) + "]<<data[" + str(data_num + 1) + "])/" + str(int(function_name[col_num + 1])) + "\n\t")
                data_num += 2
            except ValueError:
                c_file.write("(data[" + str(data_num) + "]<<data[" + str(data_num + 1) + "])/" + "1\n\t")
                data_num += 2
        elif function_name[col_num] == "uint32_t":
            try:
                c_file.write("(data[" + str(data_num) + "]<<data[" + str(data_num + 1) + "]<<data[" + str(data_num + 2) + "])/" + str(int(function_name[col_num + 1])) + "\n\t")
                data_num += 3
            except ValueError:
                c_file.write("(data[" + str(data_num) + "]<<data[" + str(data_num + 1) + "]<<data[" + str(data_num + 2) + "])/" + "1\n\t")
                data_num += 3
        else:
            try:
                c_file.write("(data[" + str(data_num) + "])/" + str(int(function_name[col_num + 1])) + "\n\t")
                data_num += 1
            except ValueError:
                c_file.write("(data[" + str(data_num) + "])/" + "1\n\t")
                data_num += 1

        col_num += 3
    c_file.write("\n")

    #write out function name
    #loop through argument name, type, xmit scale and write
    
    


with open("cfileTest.c", 'w') as c_file:
    for x in range(int(function_num)):
        function_writer(x)

    #TODO:
    #Loop through rows but filter based on board number, etc.
