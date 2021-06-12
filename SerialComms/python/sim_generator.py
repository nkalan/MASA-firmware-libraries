"""
Script to convert a firmware test case .csv file into C code that a board
can use to simulate data and commands.

Michigan Aeronautical Science Association
Author: Nathaniel Kalantar (nkalan@umich.edu)
Created: June 10, 2021
Updated: June 10, 2021
"""


import file_generator_byte_info as byte_info
#import csv
import pandas as pd  # TODO use csv or pandas, not both
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
        board_num = 0
        test_case_filename = "telem_data_pressboard"
    

    """ Define tests case input format """
    ENTRY_TYPE = "type (d)ata/(c)md/(s)kip"
    ENTRY_CMD_TYPE = "c"
    ENTRY_DATA_TYPE = "d"
    ENTRY_SKIP_TYPE = "s"
    ENTRY_DURATION = "duration"

    # Arbitrarily limiting number of arguments to 4
    # We've never had one with more than 2, so I don't anticipate a problem
    # TODO: figure out if this is a problem
    CMD_ARG0 = "arg0"
    CMD_ARG1 = "arg1"
    CMD_ARG2 = "arg2"
    CMD_ARG3 = "arg3"

    # Command packet info
    CMD_PRIORITY = 1
    CMD_NUM_PACKETS = 1
    CMD_DO_COBBS = 1
    CMD_CHECKSUM = 0

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
    test_h_filepath = "../../Inc/firmware_test.h"
    test_c_filepath = "../../Src/firmware_test.c"
    test_case_filepath = "firmware_test_cases/" + test_case_filename
    cmd_template_filepath = "telem_cmd_template.csv"

    # Load in json file with board addresses
    with open('./board_addr.txt') as board_addr_txt:
        board_num_mapping = json.load(board_addr_txt)

    board_name = board_num_mapping[board_num]

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


    """ Read telem packet format info """
    telem_packet_df = pd.read_csv(telem_template_filepath, index_col="firmware_variable")


    """ Read command format info """
    cmd_df = pd.read_csv(cmd_template_filepath, index_col="function_name")


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


    """ Read each entry in the test csv, error check them, and fill the data structures """
    test_df = pd.read_csv(test_case_filepath)

    for test_index, test_entry in test_df.iterrows():

        # Common functionality between data and command entries
        if type(test_entry[ENTRY_TYPE]) == str and (test_entry[ENTRY_TYPE].lower() == ENTRY_DATA_TYPE or test_entry[ENTRY_TYPE].lower() == ENTRY_CMD_TYPE):


            # Data entry
            if test_entry[ENTRY_TYPE].lower() == ENTRY_DATA_TYPE:





                pass

            # Command entry
            elif type(test_entry[ENTRY_TYPE]) == str and test_entry[ENTRY_TYPE].lower() == ENTRY_CMD_TYPE:




                pass

        # Skipped entry
        elif type(test_entry[ENTRY_TYPE]) == str and test_entry[ENTRY_TYPE].lower() == ENTRY_SKIP_TYPE:
            continue  # pretend like it's doesn't exist lol

        # Invalid entry
        else:
            print("Error on entry #" + str(test_index) + ": Entry type must be \"" + str(ENTRY_CMD_TYPE) + "\" (cmd) or \"" + str(ENTRY_DATA_TYPE) + "\" (data)")

    """ Autogenerated file definitions """



    """ Generate test case header file """



    """ Generate test case source file """


if __name__ == '__main__':
    main()