"""
Script to convert a firmware test case .csv file into C code that a board
can use to simulate data and commands.

Michigan Aeronautical Science Association
Author: Nathaniel Kalantar (nkalan@umich.edu)
Created: June 10, 2021
Updated: July 2, 2021
"""

import pandas as pd
import numpy as np
import sys
import time
import json


class StateSet:
    def __init__(self):
        durations = list()
        data = dict()


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
    TESTS_SHEET_NAME = "Tests"
    TESTS_DESCRIPTION_COL = "Description"
    TESTS_TEST_NAME_COL = "Test Name"
    TESTS_NOT_TEST_COLS = [TESTS_DESCRIPTION_COL, TESTS_TEST_NAME_COL]

    DATA_SHEET_NAME = "Data"
    DATA_DESCRIPTION_COL = "Description"
    DATA_STATE_NAME_COL = "State Name (first instance of each state will be default)"
    DATA_DURATION_COL = "Duration (ms)"
    NOT_DATA_COLS = [DATA_DESCRIPTION_COL, DATA_STATE_NAME_COL, DATA_DURATION_COL]

    DATA_COLS = list()  # Fill this after parsing
    DATA_VALID_TYPES = ["char", "uint8_t", "int8_t", "uint16_t", "int16_t", "uint32_t", "int32_t", "float", "uint64_t", "int64_t", "double"]

    # telem_data_boardname relevant column names
    TEMPLATE_VARNAME_COL = "firmware_variable"
    TEMPLATE_TYPE_COL = "firmware_type"
    TEMPLATE_GENERATE_COL = "should_generate"
    SHOULD_GENERATE_YES = "y"


    """ Define file paths """
    test_h_filepath = "../../../Inc/firmware_test.h"
    test_c_filepath = "../../../Src/firmware_test.c"
    test_case_filepath = "firmware_test_cases/" + test_case_filename
    cmd_template_filepath = "telem_cmd_template.csv"

    # Get the right telem packet file
    telem_template_filepath = "telem_data_"
    
    # Conditioning on board address, which should never change
    if board_num == 0:
        telem_template_filepath += "gsecontroller"
    elif board_num == 1:
        telem_template_filepath += "flightcomp"
    elif board_num == 2:
        telem_template_filepath += "flightec"
    elif board_num == 3:
        telem_template_filepath += "pressboard"
    elif board_num == 4:
        telem_template_filepath += "recovery_undefined"  # TODO: change this once we define the nosecone board packet file
    elif board_num == 5:
        telem_template_filepath += "blackbox_undefined"  # TODO: same here
    
    telem_template_filepath += ".csv"


    """ Read telem packet format info """
    template_df = pd.read_csv(telem_template_filepath, index_col=TEMPLATE_VARNAME_COL)

    # Build a list of valid variables
    valid_vars = set()  # Used to error check Data input sheet

    for var_name, var_entry in template_df.iterrows():
        if var_entry.loc[TEMPLATE_GENERATE_COL].lower() == SHOULD_GENERATE_YES:
            valid_vars.add(var_name)


    """ Defining data structures for test case generation """
    # Dictionaryy containing all data to be generated into ararys
    data_dict = dict()  # {Key=StateName: Val=list of dicts {Key=varname: Val=list of values}}

    # Dictionary containing durations of each data set
    durations_dict = dict()  # {Key=StateName: Val=list of lists [ [set0 durations], [set1 durations] ]}

    # The first time data is used, it must be initialized to some value.
    # If subsequent cells are left empty, they hold their previous value.
    #data_initialized = False  Moved to data loop


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