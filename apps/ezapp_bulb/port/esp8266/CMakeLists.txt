# @author xurongjun (xurongjun@ezvizlife.com)
# @brief The entrance of ezos components building
# @version 0.1
# @date 2021-09-29

cmake_minimum_required(VERSION 2.8)

####### Prepare building environment ##########

# Get SDK path
if(NOT EZOS_PATH)
    get_filename_component(EZOS_PATH ../../../../ ABSOLUTE)
endif()

# Check SDK Path
if(NOT EXISTS ${EZOS_PATH})
    message(FATAL_ERROR "EZOS path Error, Please set EZOS_PATH variable")
endif()

include(${EZOS_PATH}/tools/cmake/compile.cmake)

# Project Name, default the same as project directory name
get_filename_component(parent_dir ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
get_filename_component(project_dir_name ${parent_dir} NAME)

set(PROJECT_NAME ${project_dir_name})
set(PROJECT_KCONFIG_PATH "${parent_dir}/../..")

do_lib_building("ezapp")
###############################################