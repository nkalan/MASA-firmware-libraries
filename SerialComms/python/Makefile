PYTHON = python3

# Global Defines C files
GLOBALS_H_FILE = ../../../Inc/globals.h
GLOBALS_C_FILE = ../../../Src/globals.c

# GUI Telem Data Templates
TELEM_PARSE_FILE = ${GUI_PATH}/parsers/telemParse.py
GSE_TELEM_PARSE_FILE = ${GUI_PATH}/parsers/gseControllerParse.py
PRESS_TELEM_PARSE_FILE = ${GUI_PATH}/parsers/pressBoardParse.py
FLIGHT_TELEM_PARSE_FILE = ${GUI_PATH}/parsers/flightCompParse.py

# Telem Data CSV files
TELEM_CSV_FILE = ./telem_data_template.csv
GSE_TELEM_CSV_FILE = ./telem_data_gsecontroller.csv
PRESS_TELEM_CSV_FILE = ./telem_data_pressboard.csv
FLIGHT_TELEM_CSV_FILE= ./telem_data_flightcomp.csv

# Calibration Files
PT_CAL_CSV_FILE	= ./pt_calibration.csv
PT_CAL_H_FILE	= ../../../Inc/pt_calibrations.h 
PT_CAL_C_FILE	= ../../../Src/pt_calibrations.c

# Telem Command CSV Files
TELEM_CMD_FILE	= ./telem_cmd_template.csv
TELEM_CMD_FILE_DIRECTORY = ${GUI_PATH}
TELEM_C_FILE_DIRECTORY = ../../../Src/

# Board Number Assignments
GSE_BOARD_NUM		= 0
FLIGHT_BOARD_NUM	= 1
PRESS_BOARD_NUM		= 3

# Note: You need to configure the GUI_PATH variable
# 		in your shell's ~/.bash_profile or ~/.bashrc
# 		in order for the build script to work. You 
# 		should configure it to be the path to your 
# 		MASA gui/python directory

press:
	${PYTHON} ./telem_file_generator.py \
	${PRESS_TELEM_CSV_FILE} ${PRESS_TELEM_PARSE_FILE} \
	${GLOBALS_H_FILE} ${GLOBALS_C_FILE} ${PRESS_BOARD_NUM}
	
	${PYTHON} ./cmd_file_generator.py \
		${TELEM_CMD_FILE} \
		${TELEM_CMD_FILE_DIRECTORY}

	${PYTHON} ./cmd_template_parser.py \
		-f ${TELEM_CMD_FILE} \
		-n ${PRESS_BOARD_NUM} \
		-o ${TELEM_C_FILE_DIRECTORY}

flight:
	${PYTHON} ./telem_file_generator.py \
		${FLIGHT_TELEM_CSV_FILE} ${FLIGHT_TELEM_PARSE_FILE} \
		${GLOBALS_H_FILE} ${GLOBALS_C_FILE} ${FLIGHT_BOARD_NUM}

	${PYTHON} ./cmd_file_generator.py \
		${TELEM_CMD_FILE} \
		${TELEM_CMD_FILE_DIRECTORY}

	${PYTHON} ./cmd_template_parser.py \
		-f ${TELEM_CMD_FILE} \
		-n ${FLIGHT_BOARD_NUM} \
		-o ${TELEM_C_FILE_DIRECTORY}

gse:
	${PYTHON} ./telem_file_generator.py \
		${GSE_TELEM_CSV_FILE} ${GSE_TELEM_PARSE_FILE} \
		${GLOBALS_H_FILE} ${GLOBALS_C_FILE} ${GSE_BOARD_NUM}

	${PYTHON} ./cmd_file_generator.py \
		${TELEM_CMD_FILE} \
		${TELEM_CMD_FILE_DIRECTORY}

	${PYTHON} ./cmd_template_parser.py \
		-f ${TELEM_CMD_FILE} \
		-n ${GSE_BOARD_NUM} \
		-o ${TELEM_C_FILE_DIRECTORY}

#cal:
#	${PYTHON} ./pt_calibration_generator.py \
#		${PT_CAL_CSV_FILE}	\
#		${PT_CAL_H_FILE} ${PT_CAL_C_FILE}
