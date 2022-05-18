# @author xurongjun (xurongjun@ezvizlife.com)
# @brief The entrance of ezos components building
# @version 0.1
# @date 2021-09-29

cmake_minimum_required(VERSION 2.8)

####### Prepare building environment ##########

# Get SDK path
if(IS_DIRECTORY $ENV{EZOS_PATH})
    set(EZOS_PATH $ENV{EZOS_PATH})
else()
    get_filename_component(EZOS_PATH ../ ABSOLUTE)
endif()

# Check SDK Path
if(NOT EXISTS ${EZOS_PATH})
    message(FATAL_ERROR "EZOS path Error, Please set EZOS_PATH variable")
endif()

include(${EZOS_PATH}/tools/cmake/compile.cmake)

set(PROJECT_NAME linux)
set(PROJECT_KCONFIG_PATH "${parent_dir}")

do_project_building("ezapp_example")
###############################################