"""
MASA telemetry format and global variables generator script

Michigan Aeronautical Science Association
Author: Nathaniel Kalantar (nkalan@umich.edu)
Modified from Engine Controller 3 code
Created: November 9, 2020
Updated: December 18, 2020
"""

import time
import sys
import csv

# Used to split() lines into columns
COLUMN_DELIMITER = ','

# Expected number of bytes for a given variable type
type_byte_lengths = {
    "char"		:	1,
    "uint8_t"	:	1,
    "int8_t"	:	1,
    "uint16_t"	:	2,
    "int16_t"	:	2,
    "uint32_t"	:	4,
    "int32_t"	:	4,
    "uint64_t"	:	8,
    "int64_t"	:	8
}

# Upper bound for a given variable type
type_range_positive = {
    "char"		:	255,
    "uint8_t"	:	(2**8)-1,
    "int8_t"	:	((2**8)/2)-1,
    "uint16_t"	:	(2**16)-1,
    "int16_t"	:	((2**16)/2)-1,
    "uint32_t"	:	(2**32)-1,
    "int32_t"	:	((2**32)/2)-1,
    "uint64_t"	:	(2**64)-1,
    "int64_t"	:	((2**64)/2)-1
}

# Lower bound for a given variable type
type_range_negative = {
    "char"		:	0,
    "uint8_t"	:	0,
    "int8_t"	:	-((2**8)/2),
    "uint16_t"	:	0,
    "int16_t"	:	-((2**16)/2),
    "uint32_t"	:	0,
    "int32_t"	:	-((2**32)/2),
    "uint64_t"	:	0,
    "int64_t"	:	-((2**64)/2)
}

# Used for parser_data_dict_str and telem_parser.py
# See https://docs.python.org/3/library/struct.html for details on byte formatting
type_unpack_arg = {
    "char"		:	"\"<c\"",
    "uint8_t"	:	"\"<B\"",
    "int8_t"	:	"\"<b\"",
    "uint16_t"	:	"\"<H\"",
    "int16_t"	:	"\"<h\"",
    "uint32_t"	:	"\"<I\"",
    "int32_t"	:	"\"<i\"",
    "uint64_t"	:	"\"<L\"",
    "int64_t"	:	"\"<l\"",
}


"""
Main program
Reads in the .csv template and generates files from it
"""
def main():

    # Open the telem template file
    #filename = sys.argv[1]
    #filename = "test1.csv"
    filename = "telem_data_template.csv"
    template_file = open(filename)
    print("Reading " + filename + "...")

    # Autogeneration label and timestamp
    begin_autogen_tag = "/// BEGIN AUTOGENERATED SECTION - MODIFICATIONS TO THIS CODE WILL BE OVERWRITTEN\n"
    end_autogen_tag = "/// END AUTOGENERATED SECTION - USER CODE GOES BELOW THIS LINE\n"
    autogen_label = "/// Autogenerated by telem_defines_globals_generate.py on " + time.ctime()

    # Read through globals.c and store the user-written section, so it doesn't get overwritten
    globals_c_user_string = ""
    try:
        globals_c = open("../src/globals.c")
        user_section_found = False
        for line in globals_c:
            if (user_section_found):
                globals_c_user_string += line
            if (line == end_autogen_tag):
                user_section_found = True
    except:
        print("globals.c not found. Generating new file...")


    # For pack_telem_defines.h
    pack_telem_defines_h_string = begin_autogen_tag + "\n/// pack_telem_defines.h\n" + \
                                autogen_label + "\n\n" + \
                                "#include \"globals.h\"\n" + \
                                "#include <stdint.h>\n" + \
                                "\nextern void pack_telem_data(uint8_t* dst);\n" + \
                                "\n"

    # For pack_telem_defines.c
    pack_telem_defines_c_string = begin_autogen_tag + "\n/// pack_telem_defines.c\n" + \
                                autogen_label + "\n\n" + \
                                "#include \"../inc/pack_telem_defines.h\"\n\nvoid pack_telem_data(uint8_t* dst){\n"

    # For globals.h and globals.c
    globals_h_string = begin_autogen_tag + "\n/// globals.h\n" + autogen_label + "\n\n" + "#include <stdint.h>" + "\n\n"
    globals_c_string = begin_autogen_tag + "\n/// globals.c\n" + autogen_label + "\n\n" + "#include \"globals.h\"" + "\n\n"

    global_arrays_generated = dict()  # Maps firmware_variable to list of [firmware_type, highest_index]

    # For byte_packet_template.txt_sprintf-call.c
    format_string = ""
    argument_string = ""

    # For telem_parser.py
    parser_data_dict_str = ""
    parser_units_dict_str = ""
    parser_csv_header = "Time (s),"
    parser_self_init_str = ""
    parser_log_string = "\t\tself.log_string = str(time.clock()) + ','"
    parser_items_list = list()

    col = dict()  # Dictionary mapping column names to indices
    packet_byte_length = 0	# Total bytes in packet (running total)

    num_items = 0  # Doesn't use enumerate to get the number of items because not all lines get telem'd (should_generate column)
    for csv_row_num, line in enumerate(template_file):
        split_string = line.strip().split(COLUMN_DELIMITER)  # strip() because last column sometimes has trailing '\n'

        # Create a dictionary mapping column names to column indices
        if(csv_row_num == 0):	# First line of the file
            c = 0
            for arg in split_string:
                col[arg] = c  # Last column header sometimes has '\n' at the end
                c += 1
        
        else:  # Rest of the file with real data

            # Parse the row and store each datum into a variable
            name                = split_string[col['name']]
            firmware_variable   = split_string[col['firmware_variable']]
            min_val	            = split_string[col['min_val']]
            max_val	            = split_string[col['max_val']]
            unit                = split_string[col['unit']]
            firmware_type       = split_string[col['firmware_type']]
            printf_format       = split_string[col['printf_format']]
            type_cast           = split_string[col['type_cast']]
            xmit_scale	        = split_string[col['xmit_scale']]

            # These are used to generate the TelemParser gui class
            python_variable_override  = split_string[col['python_variable_override']]
            python_type  = split_string[col['python_type']]
            python_globals	 = split_string[col['python_globals']]
            python_init  = split_string[col['python_init']]
            
            # Only read the marked rows
            should_generate = split_string[col['should_generate']]

            # Only write the value to pack_telem_defines.h if should_generate is 'y'
            try:
                assert(should_generate == 'y' or should_generate == 'n')
            except:
                print("[row " + str(csv_row_num + 1) + "] " + "Error: should_generate can only be 'y' or 'n'")
            
            if (should_generate != 'y'):
                continue

            """ Add line(s) to pack_telem_defines.h """

            # Check the type cast
            try:
                assert type_cast in type_byte_lengths.keys()
            except:
                print("[row " + str(csv_row_num + 1) + "] " + "Invalid type cast. Valid Types are:\n" + str(type_byte_lengths.keys()))

            ## Check xmit limits: make sure the float scaler doesn't make it go out of range
            try:
                if(min_val):
                    assert (float(min_val)*float(xmit_scale) >= type_range_negative[type_cast])
            except:
                print("[row " + str(csv_row_num + 1) + "] " + "Invalid type cast for given range on item " + name)
                print("Min val: " + str(float(min_val)*float(xmit_scale)))
                print("Type limit: " + str(type_range_negative[type_cast]))
            try:
                if(max_val):
                    assert (float(max_val)*float(xmit_scale) <= type_range_positive[type_cast])
            except:
                print("[row " + str(csv_row_num + 1) + "] " + "Invalid type cast for given range on item " + name)
                print("Max val: " + str(float(max_val)*float(xmit_scale)))
                print("Type limit: " + str(type_range_positive[type_cast]))
            ## End check xmit limits

            # Increment the teletry byte count
            byte_length = type_byte_lengths[type_cast]
            packet_byte_length += byte_length

            # Split the variable into byte-sized TELEM_ITEMs and add them to the #defines list in pack_telem_defines.h
            for b in range(0, byte_length):
                pack_telem_defines_h_string += "#define\tTELEM_ITEM_" + str(packet_byte_length - byte_length + b) + \
                "\t((" + type_cast + ") (" + str(firmware_variable) + "*" + str(xmit_scale) + ")) >> " + str(8*b) + " \n"


            """ Update globals.h / globals.c strings """

            # Check if the variable should be generated as an array
            open_bracket_index = firmware_variable.find("[")

            # If it should be in an array ('[' was found in the variable name)
            if (open_bracket_index != -1):

                # Get the variable name without brackets and the array index
                array_name = firmware_variable[0 : open_bracket_index]
                close_bracket_index = firmware_variable.find("]")
                array_index_str = firmware_variable[open_bracket_index + 1 : close_bracket_index] # Leave it as a string to error check it

                # Error check the array bracket formatting
                try:
                    assert(close_bracket_index == len(firmware_variable) - 1)  # ']' should be the last character
                    assert(array_index_str.isdecimal()) # Make sure it's a number between the brackets
                except:
                    print("[row " + str(csv_row_num + 1) + "] " + "Invalid firmware variable name: items in arrays must be written as var_name[index], index >= 1")

                # Update the array size in the globals dictionary if it's bigger than the current array size
                array_index_decimal = int(array_index_str)
                if (array_name in global_arrays_generated):
                    # Make sure you keep all variables of the same array as the same type
                    try:
                        assert(firmware_type == global_arrays_generated[array_name][0])
                    except:
                        print("[row " + str(csv_row_num + 1) + "] " + "Error: All variables of the same array must be declared as the same type.")
                    # End array type check
                        
                    global_arrays_generated[array_name][1] = max(array_index_decimal, global_arrays_generated[array_name][1])
                # Otherwise create the list and set the array size manually
                else:
                    global_arrays_generated[array_name] = [firmware_type, array_index_decimal]
            
            # If it's not supposed to be an array (normal variable)
            else:
                #Add it to the file string
                globals_h_string += "extern " + firmware_type + " " + firmware_variable + ";\n"
                globals_c_string += firmware_type + " " + firmware_variable + " = 0;\n"
                
            """
            Note: nothing to do with this program, not sure why I wrote it here
            Pressure transducer calibration generator script:

            3 column csv file
            1: channel id (what channel it's connected to on the board)
            2: slope
            3: offest

            generate a .h file
            in the press board firmware, use the calibrations in that header file
            """

            # Update the byte_packet_template.txt_sprintf-call.c strings
            """
            format_string = format_string + printf_format.rstrip('\n') +','
            argument_string = argument_string + ("," + firmware_variable)
            """

            # Update the telem_parser.py strings
            
            # Parse Globals  # Unused
            #if(python_globals):
                #globals_string = globals_string + "\t\tglobal " + python_globals + '\n'
                #parser_self_init_str += "\t\tself." + python_globals + " = " + python_init + "\n"
           
            python_variable = firmware_variable  # If the template csv says to give a variable a different name in the gui parser, do it here
            if(python_variable_override):
                python_variable = python_variable_override

            parser_items_list.append(python_variable)

            parser_data_dict_str +=	"\t\tself.dict[self.items[" + str(num_items) + "]] = " + python_type + \
                                "((float(struct.unpack(" + type_unpack_arg[type_cast] + ", packet[" + \
                                str(packet_byte_length - byte_length) + ":" + str(packet_byte_length) + "])[0]))/" + xmit_scale + ")\n"
            
            parser_units_dict_str += "self.units[self.items[" + str(num_items) + "]] = " + unit + "\n"

            parser_csv_header += python_variable + ' (' + unit + '),'
            parser_log_string += " + str(self.dict[self.items[" + str(num_items) + "]])" + " + ','" 
            
            num_items += 1
        #end else

    # end for

    # Parse through the global array dictionary and add them to the globals.c/.h strings
    # array_info is (array_name, (firmware_type, highest_index))
    for array_info in global_arrays_generated.items():
        globals_h_string += "extern " + array_info[1][0] + " " + array_info[0] + "[" + str(array_info[1][1] + 1) + "];\n"
        globals_c_string += array_info[1][0] + " " + array_info[0] + "[" + str(array_info[1][1] + 1) + "] = {0};\n"
    globals_c_string += "\n" + end_autogen_tag

    # Add the number of TELEM_ITEMs to pack_telem_defines, and declare pack_telem_data() as non-extern. TODO: why?
    pack_telem_defines_h_string += "#define\tCLB_NUM_TELEM_ITEMS\t" + str(packet_byte_length) + "\n"
    pack_telem_defines_h_string += "\nvoid pack_telem_data(uint8_t* dst);\n"

    # Fill up pack_telem_defines.c with unpacking code
    for m in range(0, packet_byte_length):
        pack_telem_defines_c_string += "\t*(dst + " + str(m) + ") = TELEM_ITEM_" + str(m) + ";\n"
    pack_telem_defines_c_string += "}"

    # Updating telem_parser.py strings
    parser_csv_header += "\\n\"\n"

    parser_self_init_str += "\t\tself.log_string = \"\"\n" + \
                            "\t\tself.num_items = " + str(num_items) + "\n" + \
                            "\t\t\n" + \
                            "\t\tself.dict = {}\n" + \
                            "\t\t\n" + \
                            "\t\tself.items = [''] * self.num_items\n"

    # Add the initialization for the items dict to the telem parser
    for index, var in enumerate(parser_items_list):
        parser_self_init_str += "\t\tself.items[" + str(index) + "] = \'" + var + "\' \n"

    """ Writing to files """

    #parsed_printf_file = open((filename + "_sprintf-call_.c"), "w+")
    telem_parser = open("telem_parser.py", "w+")
    pack_telem_defines_h = open("../inc/pack_telem_defines.h", "w+")  # Generates file in current folder
    pack_telem_defines_c = open("../src/pack_telem_defines.c", "w+")
    globals_h = open("../inc/globals.h", "w+")
    globals_c = open("../src/globals.c", "w+")

    #parsed_printf_file.write("snprintf(line, sizeof(line), \"" + format_string + "\\r\\n\"" + argument_string + ");")

    telem_parser.write(	begin_autogen_tag + "\n\n/// telem_parser.py\n" + autogen_label + "\n\nimport time\nimport struct\n\nclass TelemParser:\n\n" + \
                                "\tdef __init__(self):\n\t\tself.csv_header = \"" + \
                                parser_csv_header + parser_self_init_str + "\n"
                                "\tdef parse_packet(self, packet):\n" + \
                                parser_data_dict_str + \
                                parser_log_string)

    pack_telem_defines_h.write(pack_telem_defines_h_string)
    pack_telem_defines_c.write(pack_telem_defines_c_string)

    globals_h.write(globals_h_string)
    
    globals_c.write(globals_c_string)
    globals_c.write(globals_c_user_string)  # Add the user-written section back in

    print(filename + " Successfully Parsed!")
    print(" --- Packet statistics --- ")
    print("Packet items: " + str(num_items))
    print("Packet length (bytes): " + str(packet_byte_length))

if __name__ == '__main__':
    main()