cmake_minimum_required(VERSION 3.16)

# Use STM32 gcc toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/stm32_gcc.cmake)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
  libserialcomms
  LANGUAGES C ASM
  VERSION 1.0
)

# Check for stm32f4xx_hal_conf.h path
if(NOT DEFINED STM32F4XX_HAL_CONF_H)
  message(FATAL_ERROR "STM32F4XX_HAL_CONF_H must be set to a 'stm32fxx_hal_conf.h'.")
elseif(NOT EXISTS ${STM32F4XX_HAL_CONF_H})
  message(FATAL_ERROR "${STM32F4XX_HAL_CONF_H} does not exist.")
endif()
if(NOT DEFINED PACK_CMD_DEFINES_H)
  message(FATAL_ERROR "PACK_CMD_DEFINES_H must be set to a 'pack_cmd_defines.h'.")
elseif(NOT EXISTS ${PACK_CMD_DEFINES_H})
  message(FATAL_ERROR "${PACK_CMD_DEFINES_H} does not exist.")
endif()
if(NOT DEFINED PACK_TELEM_DEFINES_H)
  message(FATAL_ERROR "PACK_TELEM_DEFINES_H must be set to a 'pack_telem_defines.h'.")
elseif(NOT EXISTS ${PACK_TELEM_DEFINES_H})
  message(FATAL_ERROR "${PACK_TELEM_DEFINES_H} does not exist.")
endif()

file(COPY ${PACK_CMD_DEFINES_H} DESTINATION ${CMAKE_SOURCE_DIR}/inc)
file(COPY ${PACK_TELEM_DEFINES_H} DESTINATION ${CMAKE_SOURCE_DIR}/inc)

# Fetch stm32 libs
stm32_fetch_cmsis(F4)
stm32_fetch_hal(F4)

# Copy in *_hal_conf.h
fetchcontent_getproperties(STM32-HAL-F4 SOURCE_DIR STM32-HAL-F4_SOURCE_DIR)
file(COPY ${STM32F4XX_HAL_CONF_H} DESTINATION ${STM32-HAL-F4_SOURCE_DIR}/Inc)

# Enable require HAL and CMSIS components
set(HAL_COMP_LIST RCC GPIO CORTEX)
set(CMSIS_COMP_LIST "")
list(APPEND CMSIS_COMP_LIST STM32F4)
list(APPEND HAL_COMP_LIST STM32F4)

# Find stm32 libs
find_package(CMSIS COMPONENTS "${CMSIS_COMP_LIST}" REQUIRED)
find_package(HAL COMPONENTS "${HAL_COMP_LIST}" REQUIRED)

set(SOURCES src/comms.c ${STM32F4XX_HAL_CONF_H})
add_library(libserialcomms ${SOURCES})

target_include_directories(libserialcomms PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(
  libserialcomms
  HAL::STM32::F4::RCC
  HAL::STM32::F4::GPIO
  HAL::STM32::F4::CORTEX
  CMSIS::STM32::F407VG
  STM32::NoSys
)
stm32_print_size_of_target(libserialcomms)
