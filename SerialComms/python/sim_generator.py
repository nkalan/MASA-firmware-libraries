"""
Script to convert a firmware test case .csv file into C code that a board
can use to simulate data and commands.

Michigan Aeronautical Science Association
Author: Nathaniel Kalantar (nkalan@umich.edu)
Created: June 10, 2021
Updated: June 10, 2021
"""

import pandas as pd  # TODO use csv or pandas, not both
import numpy as np
import sys
import time
import json


def main():

    """ Read input """
    use_args = False

    if use_args:
        # Error checking command line arguments
        if len(sys.argv) != 3:
            print("Error: incorrect input. Proper usage is: sim_generator.py <board_number> <test_case_filename>")
            return
        elif sys.argv[1] not in [0, 1, 2, 3, 4, 5]:
            print("Error: invalid board address. See board_addr.txt for valid board address mappings.")
            return

        board_num = sys.argv[1]
        test_case_filename = sys.argv[2]
    else:
        # For debugging only
        board_num = 3
        test_case_filename = "test1.csv"
    

    """ Define tests case input format """
    ENTRY_TYPE = "type (d)ata/(c)md/(s)kip"
    ENTRY_CMD_TYPE = "c"
    ENTRY_DATA_TYPE = "d"
    ENTRY_SKIP_TYPE = "s"
    ENTRY_DURATION = "duration (ms)"

    # Arbitrarily limiting number of arguments to 3
    # We've never had one with more than 2, so I don't anticipate a problem
    # TODO: figure out if this is a problem
    CMD_ENTRY_FUNCTION_NAME = "function_name"
    CMD_ENTRY_ARG0 = "arg0"
    CMD_ENTRY_ARG1 = "arg1"
    CMD_ENTRY_ARG2 = "arg2"

    # Command packet info
    CMD_PRIORITY = 1
    CMD_NUM_PACKETS = 1
    CMD_DO_COBBS = 1
    CMD_CHECKSUM = 0

    # Data structure to identify cmd vs data columns
    CMD_COLS = [CMD_ENTRY_FUNCTION_NAME, CMD_ENTRY_ARG0, CMD_ENTRY_ARG1, CMD_ENTRY_ARG2]
    NOT_DATA_COLS = [ENTRY_TYPE, ENTRY_DURATION] + CMD_COLS  # Data cols change by test case, easier to check if not in this list
    DATA_COLS = list()  # Fill this after parsing
    DATA_VALID_TYPES = ["char", "uint8_t", "int8_t", "uint16_t", "int16_t", "uint32_t", "int32_t", "float", "uint64_t", "int64_t", "double"]

    # telem_cmd_template column names
    CMD_TEMPLATE_FUNCTION_NAME = "function_name"
    CMD_TEMPLATE_NUM_ARGS = "nums args"
    CMD_TEMPLATE_PACKET_TYPE = "packet_type"
    CMD_TEMPLATE_SUPPORTED_ADDR = "supported_target_addr"
    CMD_TEMPLATE_ARG0 = "arg0"
    CMD_TEMPLATE_ARGTYPE0 = "arg_type0"
    CMD_TEMPLATE_XMIT0 = "xmit_scale0"
    CMD_TEMPLATE_ARG1 = "arg1"
    CMD_TEMPLATE_ARGTYPE1 = "arg_type1"
    CMD_TEMPLATE_XMIT1 = "xmit_scale1"
    CMD_TEMPLATE_ARG2 = "arg2"
    CMD_TEMPLATE_ARGTYPE1 = "arg_type2"
    CMD_TEMPLATE_XMIT2 = "xmit_scale2"

    # telem_data_boardname relevant column names
    DATA_TEMPLATE_VARNAME = "firmware_variable"
    DATA_TEMPLATE_TYPE = "firmware_type"
    DATA_TEMPLATE_GENERATE = "should_generate"
    DATA_SHOULD_GENERATE = "y"

    """
            + "\t\tpacket[0] = self.cmd_names_dict[cmd_info[\"function_name\"]]\t# packet_type\n" \
        + "\t\tpacket[1] = 0\t# ground computer addr\n" \
        + "\t\tpacket[2] = cmd_info[\"target_board_addr\"]\t# target_addr\n" \
        + "\t\tpacket[3] = 1\t# priority\n" \
        + "\t\tpacket[4] = 1\t# num_packets\n" \
        + "\t\tpacket[5] = 1\t# do_cobbs\n" \
        + "\t\tpacket[6] = 0\t# checksum\n" \
        + "\t\tpacket[7] = 0\t# checksum\n" \
        + "\t\tpacket[8] = (cmd_info[\"timestamp\"] >> 0) & 0xFF\t# timestamp\n" \
        + "\t\tpacket[9] = (cmd_info[\"timestamp\"] >> 8) & 0xFF\t# timestamp\n" \
        + "\t\tpacket[10] = (cmd_info[\"timestamp\"] >> 16) & 0xFF\t# timestamp\n" \
        + "\t\tpacket[11] = (cmd_info[\"timestamp\"] >> 24) & 0xFF\t# timestamp\n" \
    """

    """ Define file paths """
    test_h_filepath = "../../../Inc/firmware_test.h"
    test_c_filepath = "../../../Src/firmware_test.c"
    test_case_filepath = "firmware_test_cases/" + test_case_filename
    cmd_template_filepath = "telem_cmd_template.csv"

    # Load in json file with board addresses
    with open('./board_addr.txt') as board_addr_txt:
        board_num_mapping = json.load(board_addr_txt)

    board_name = board_num_mapping[str(board_num)]

    # Get the right telem packet file
    # Note: I'm not hardcoding the mappings here in case of the unlikely event that they change
    # Instead I'm conditioning on their names, which will never change.
    telem_template_filepath = "telem_data_"
    
    if board_name == "GSEController":
        telem_template_filepath += "gsecontroller"
    elif board_name == "FlightComputer":
        telem_template_filepath += "flightcomp"
    elif board_name == "EngineController":
        telem_template_filepath += "flightec"
    elif board_name == "PressurizationController":
        telem_template_filepath += "pressboard"
    elif board_name == "RecoveryController":
        telem_template_filepath += "recovery_undefined"  # TODO: change this once we define the nosecone board packet file
    elif board_name == "BlackBox":
        telem_template_filepath += "blackbox_undefined"  # TODO: same here
    
    telem_template_filepath += ".csv"


    """ Defining data structures for test case generation """
    # Items are tuples of (name, [args])
    cmds = list()

    # Key = test case csv column name (limited to supported packet variables.)
    # Value = list of data points
    data = dict()

    # List of duration for each time step in the file - includes data and commands
    # Length equal to num_data + num_cmds
    durations = list()

    # False for data, True for command
    # Length equal to num_data + num_cmds
    entry_type_cmd = list()

    # List of command names supported by the board
    valid_cmds = set()

    # List of packet variables that are allowed to be simulated
    valid_vars = set()

    # Equal to num_data + num_cmds, just tracks length of simulation
    num_entries = 0

    # Number of commands to simulate
    num_cmds = 0

    # Size of a variable's data array
    num_data = 0

    # The first time data is used, it must be initialized to some value.
    # If subsequent cells are left empty, they hold their previous value.
    data_initialized = False


    """ Read telem packet format info """
    telem_packet_df = pd.read_csv(telem_template_filepath, index_col=DATA_TEMPLATE_VARNAME)

    # Build a list of valid variables
    for var_name, var_entry in telem_packet_df.iterrows():
        if var_entry.loc[DATA_TEMPLATE_GENERATE].lower() == DATA_SHOULD_GENERATE:
            valid_vars.add(var_name)


    """ Read command format info """
    cmd_df = pd.read_csv(cmd_template_filepath, index_col=CMD_TEMPLATE_FUNCTION_NAME)

    # Build a list of valid commands
    for cmd_name, cmd_entry in cmd_df.iterrows():
        if str(board_num) in cmd_entry.loc[CMD_TEMPLATE_SUPPORTED_ADDR].split():
            valid_cmds.add(cmd_name)


    """ Read each entry in the test csv, error check them, and fill the data structures """
    test_df = pd.read_csv(test_case_filepath)

    # Build list of variables in the test case
    # Error check that the variables actually exist in the firmware
    for col_name in test_df.columns:
        if col_name not in NOT_DATA_COLS and col_name not in valid_vars:
            print("Error: " + col_name + " is not a valid variable on board " + str(board_num))
            return
        if col_name not in NOT_DATA_COLS:
            DATA_COLS.append(col_name)

    # Read the test case
    for test_index, test_entry in test_df.iterrows():

        # Common functionality between data and command entries
        if type(test_entry[ENTRY_TYPE]) == str and (test_entry[ENTRY_TYPE].lower() == ENTRY_DATA_TYPE or test_entry[ENTRY_TYPE].lower() == ENTRY_CMD_TYPE):

            # Check that a number is entered for duration
            if (test_entry.isnull().loc[ENTRY_DURATION]):
                print("Error on test input line " + str(test_index+2) + ": duration must be specified.")
                return
            if type(test_entry.loc[ENTRY_DURATION]) != int:
                print("Error on test input line " + str(test_index+2) + ": duration must be an integer.")
                return
            if test_entry.loc[ENTRY_DURATION] <= 0:
                print("Error on test input line "+ str(test_index+2) + ": entry duration must be greater than 0.")
                return
            
            durations.append(test_entry.loc[ENTRY_DURATION])
            num_entries += 1

            # Data entry
            if test_entry[ENTRY_TYPE].lower() == ENTRY_DATA_TYPE:
                
                # Error check that the first data entry initializes all data columns
                if not data_initialized:
                    for data_col in DATA_COLS:
                        if test_entry.isnull().loc[data_col]:
                            print("Error on test input line " + str(test_index+2) + ": first instance of data must initialize all entries.")
                            return
                        else:
                            data[data_col] = list()  # Create a list in the dictionary
                    data_initialized = True
                
                # Add the data to the dictionary list
                for data_col in DATA_COLS:
                    if test_entry.isnull().loc[data_col]:
                        data[data_col].append(data[data_col][-1])  # If there's an empty cell, repeat the previous value
                    else:
                        data[data_col].append(test_entry.loc[data_col])  # Otherwise fill it in directly
                
                # Mark that this entry is data
                entry_type_cmd.append(False)

                # Increment count
                num_data += 1

            # Command entry
            elif type(test_entry.loc[ENTRY_TYPE]) == str and test_entry.loc[ENTRY_TYPE].lower() == ENTRY_CMD_TYPE:

                # Error check that the command is specified
                if test_entry.isnull().loc[CMD_ENTRY_FUNCTION_NAME]:
                    print("Error on test input line " + str(test_index+2) + ": command function name not specified.")
                    return
                
                # Error check that the command exists for the specified board
                if test_entry.loc[ CMD_ENTRY_FUNCTION_NAME] not in valid_cmds:
                    print("Error on test input line " + str(test_index+2) + ": " + test_entry.loc[CMD_ENTRY_FUNCTION_NAME] + " is not a valid function for board " + str(board_num))
                    return

                # Error check that all args are filled in
                num_args = cmd_df.loc[test_entry.loc[CMD_ENTRY_FUNCTION_NAME]].loc[CMD_TEMPLATE_NUM_ARGS]
                
                arg0_provided = True
                arg1_provided = True
                arg2_provided = True
                if test_entry.isnull().loc[CMD_ENTRY_ARG0]:
                    arg0_provided = False
                if test_entry.isnull().loc[CMD_ENTRY_ARG1]:
                    arg1_provided = False
                if test_entry.isnull().loc[CMD_ENTRY_ARG2]:
                    arg2_provided = False

                if ((num_args == 0 and not ((not arg0_provided) and (not arg1_provided) and (not arg2_provided))) \
                or (num_args == 1 and not ((arg0_provided) and (not arg1_provided) and (not arg2_provided))) \
                or (num_args == 2 and not ((arg0_provided) and (arg1_provided) and (not arg2_provided))) \
                or (num_args == 3 and not ((arg0_provided) and (arg1_provided) and (arg2_provided)))):
                    print("Error on test input line " + str(test_index+2) + ": " + test_entry.loc[CMD_ENTRY_FUNCTION_NAME] + " requires " + str(num_args) + " arguments.")
                    return

                # Error check that the args are the correct types

                # Add function name to data structure
                cmds.append( (test_entry.loc[CMD_ENTRY_FUNCTION_NAME], list()) )

                # Add args
                if num_args > 0 and num_args <= 3:
                    cmds[-1][1].append(test_entry.loc[CMD_ENTRY_ARG0])
                if num_args > 0 and num_args <= 2:
                    cmds[-1][1].append(test_entry.loc[CMD_ENTRY_ARG1])
                if num_args > 0 and num_args <= 1:
                    cmds[-1][1].append(test_entry.loc[CMD_ENTRY_ARG2])

                # Mark that this entry is a command
                entry_type_cmd.append(True)
                
                # Increment count
                num_cmds += 1

            """
            # Skipped entry
            elif type(test_entry[ENTRY_TYPE]) == str and test_entry[ENTRY_TYPE].lower() == ENTRY_SKIP_TYPE:
                continue  # pretend like it's doesn't exist lol
            """
        
        # Invalid entry
        else:
            print("Error on entry #" + str(test_index) + ": Entry type must be \"" + str(ENTRY_CMD_TYPE) + "\" (cmd), \"" + str(ENTRY_DATA_TYPE) + "\" (data)," \
            + " or \"" + str(ENTRY_SKIP_TYPE) + "\" (skip).")
            return


    # Debugging: print parsed data
    print("Parsed commands: ")
    for cmd_tuple in cmds:
        print(cmd_tuple[0], end="")
        for arg in cmd_tuple[1]:
            print(", " + str(arg), end="")
        print()
    print("\nParsed data: ")
    for var_name, data_vals in data.items():
        print(var_name + ": " + str(data_vals))
    print("\nParsed entry types:")
    print(str(entry_type_cmd))


    """ Autogenerated file definitions """
    begin_autogen_tag = "// AUTOGENERATED FILE - MODIFICATIONS TO THIS CODE WILL BE OVERWRITTEN"
    autogen_label = "// Autogenerated by firmware-libraries/SerialComms/python/firmware_sim_generator.py on " + time.ctime()

    test_h_str = begin_autogen_tag + "\n" + autogen_label + "\n\n#ifndef FIRMWARE_SIM_INC\n#define FIRMWARE_SIM_INC\n\n#include <stdint.h>\n\n"
    test_c_str = begin_autogen_tag + "\n" + autogen_label + "\n\n#include \"firmware_test.h\"\n\n"


    """ Generate files """

    # Define which variables are being overridden by the simulation
    for var in data.keys():
        test_h_str += "#define FW_SIM_" +  var.replace("[", "_").replace("]", "").upper() + "\n"
    test_h_str += "\n"

    # Define lengths of simulation
    test_h_str += "#define FW_SIM_TOTAL_LENGTH " + str(num_entries) + "\n"
    test_h_str += "#define FW_SIM_NUM_CMDS " + str(num_cmds) + "\n"
    test_h_str += "#define FW_SIM_NUM_DATA " + str(num_data) + "\n"
    test_h_str += "\n"

    # Declare arrays
    test_h_str += "extern uint32_t FW_SIM_entry_durations[FW_SIM_TOTAL_LENGTH];\n"
    test_h_str += "extern uint8_t FW_SIM_entry_is_cmd[FW_SIM_TOTAL_LENGTH];\n"
    test_h_str += "\n"

    # Declare variable arrays
    for var_name, var_data in data.items():
        test_h_str += "extern "

        var_arr_name = var_name.replace("[", "_").replace("]", "")
        
        # Header declaration
        var_type = telem_packet_df.loc[var_name].loc[DATA_TEMPLATE_TYPE]
        if var_type not in DATA_VALID_TYPES:
            print("Error: " + var_name + " has invalid type " + var_type + ".")
        test_h_str += var_type + " FW_SIM_" + var_arr_name + "[FW_SIM_NUM_DATA];\n"
        
        # Array definition
        # TODO: check integer type casts
        test_c_str += var_type + " FW_SIM_" + var_arr_name + "[FW_SIM_NUM_DATA] = {\n"
        for num in var_data:
            test_c_str += "\t" + str(num) + ",\n"
        test_c_str = test_c_str[:-2] + "\n"  # Remove the last comma
        test_c_str += "};\n\n"
    
    test_h_str += "\n"

    # TODO: write header comments
    test_h_str += "void FW_SIM_init_simulation_variables();\n\n"
    test_h_str += "void FW_SIM_run_simulation();\n\n"

    test_h_str += "#endif\n"

    """ Write to files """

    with open(test_h_filepath, "w+") as h_file:
        h_file.write(test_h_str)
    
    with open(test_c_filepath, "w+") as c_file:
        c_file.write(test_c_str)


if __name__ == '__main__':
    main()