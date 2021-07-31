"""
MASA callibration format and global variables generator script

Michigan Aeronautical Science Association
Authors: Sidharth Anantha (ananthas@umich.edu)
Modified from telem_file_generator.py
Created: July 25, 2021
"""

import file_generator_byte_info as byte_info
import time
import sys
import json
import pandas as pd 
import numpy as np 


def main():
    print("Hi")
    # error_ocurred variable will be used to avoid the program print out that it was
    # successful without having to terminate the program when the first error ocurrs
    # Load in json file
    with open('./board_addr.txt') as board_addr_files:
        board_addresses = json.load(board_addr_files)

    # Open the telem template file
    filename = sys.argv[1]
    template_file = open(filename)
    print("Reading " + filename + "...")

    output_file_parser_name = "TelemParse"
    output_file = "./telemParse.py"
    output_globals_h_file = "../inc/globals.h"
    output_globals_c_file = "../src/globals.c"

    # Script should read in from a csv and store the data
    callibration_parameters = pd.readcsv("calibration_data_flightec.csv")
    print(callibration_parameters)
    print("hello")