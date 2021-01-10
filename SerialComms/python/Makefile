PYTHON = python3
GLOBALS_H_FILE = ../../../Inc/globals.h
GLOBALS_C_FILE = ../../../Src/globals.c
TELEM_PARSE_FILE = ${GUI_PATH}/telemParse.py
TELEM_CSV_FILE = ./telem_data_template.csv
PT_CAL_CSV_FILE	= ./pt_calibration.csv 
PT_CAL_H_FILE	= ../../../Inc/pt_calibrations.h 
PT_CAL_C_FILE	= ../../../Src/pt_calibrations.c
TELEM_CMD_FILE	= ./telem_cmd_template.csv
TELEM_CMD_FILE_DIRECTORY = ${GUI_PATH}

# Note: You need to configure the GUI_PATH variable
# 		in your shell's ~/.bash_profile or ~/.bashrc
# 		in order for the build script to work. You 
# 		should configure it to be the path to your 
# 		MASA gui/python directory

build:
	${PYTHON} ./telem_file_generator.py \
		${TELEM_CSV_FILE} ${TELEM_PARSE_FILE} \
		${GLOBALS_H_FILE} ${GLOBALS_C_FILE}

	${PYTHON} ./cmd_file_generator.py \
		${TELEM_CMD_FILE} \
		${TELEM_CMD_FILE_DIRECTORY}

cal:
	${PYTHON} ./pt_calibration_generator.py \
		${PT_CAL_CSV_FILE}	\
		${PT_CAL_H_FILE} ${PT_CAL_C_FILE}
