# @author xurongjun (xurongjun@ezvizlife.com)
# @brief The entrance of ezos components building
# @version 0.1
# @date 2021-09-29

cmake_minimum_required(VERSION 2.8)

####### Prepare building environment ##########
get_filename_component(EZOS_PATH ../../../ ABSOLUTE)
include(${EZOS_PATH}/tools/cmake/compile.cmake)

get_filename_component(parent_dir ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
get_filename_component(project_dir_name ${parent_dir} NAME)

set(PROJECT_NAME ${project_dir_name})
set(EZOS_EXAMPLES_ENABLE 1)

do_project_building(${PROJECT_NAME})
###############################################