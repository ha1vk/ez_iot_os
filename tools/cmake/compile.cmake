# @author xurongjun (xurongjun@ezvizlife.com)
# @brief
# @version 0.1
# @date 2021-09-29

# Convert to cmake path(for Windows)
file(TO_CMAKE_PATH "${EZOS_PATH}" EZOS_PATH)

get_filename_component(parent_dir ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
get_filename_component(current_dir ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
get_filename_component(parent_dir_name ${parent_dir} NAME)

# global variables
set(g_dynamic_libs "" CACHE INTERNAL "g_dynamic_libs")
set(g_ezos_inc_dirs "" CACHE INTERNAL "g_ezos_inc_dirs")
set(g_ezos_lib_dirs "" CACHE INTERNAL "g_ezos_lib_dirs")
set(g_ezos_lib_depens "" CACHE INTERNAL "g_ezos_lib_depens")

# Set project dir, so just projec can include this cmake file!!!
set(PROJECT_SOURCE_DIR ${parent_dir})
set(PROJECT_PATH       ${PROJECT_SOURCE_DIR})
set(PROJECT_BINARY_DIR "${parent_dir}/build")
set(PROJECT_DIST_DIR   "${parent_dir}/dist")

message(STATUS "EZOS_PATH:${EZOS_PATH}")
message(STATUS "PROJECT_PATH:${PROJECT_SOURCE_DIR}")

include(${EZOS_PATH}/tools/cmake/tools.cmake)

function(register_component)
    get_filename_component(component_dir ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
    get_filename_component(component_name ${component_dir} NAME)
    message(STATUS "[register component: ${component_name} ], path:${component_dir}")

    # Get params: DYNAMIC/SHARED
    foreach(name ${ARGN})
        string(TOUPPER ${name} name_upper)
        if(${name_upper} STREQUAL "DYNAMIC" OR ${name_upper} STREQUAL "SHARED")
            set(to_dynamic_lib true)
        else()
            set(component_name ${name})
        endif()
    endforeach()

    if(to_dynamic_lib)
        message("-- component ${component_name} will compiled to dynamic lib")
        # Add dynamic file path to g_dynamic_libs variable
        set(dynamic_libs ${g_dynamic_libs})
        list(APPEND dynamic_libs "${PROJECT_BINARY_DIR}/${component_name}/lib${component_name}${DL_EXT}")
        set(g_dynamic_libs ${dynamic_libs}  CACHE INTERNAL "g_dynamic_libs")
    else()
        message("-- component ${component_name} will compiled to static lib")
        # Add static file path to g_ezos_lib_dirs variable
        set(lib_dirs ${g_ezos_lib_dirs})
        list(APPEND lib_dirs "-L${PROJECT_BINARY_DIR}/${component_name}")
        set(g_ezos_lib_dirs ${lib_dirs}  CACHE INTERNAL "g_ezos_lib_dirs")
    endif()

    message("-- component ${component_name} dependencies:${ADD_REQUIREMENTS}")

    # Add src to lib
    if(ADD_SRCS)
        if(to_dynamic_lib)
            add_library(${component_name} SHARED ${ADD_SRCS})
        else()
            add_library(${component_name} STATIC ${ADD_SRCS})
        endif()
        set(include_type PUBLIC)
    else()
        if(to_dynamic_lib)
            add_library(${component_name} SHARED)
            set(include_type PUBLIC)
        else()
            add_library(${component_name} INTERFACE)
            set(include_type INTERFACE)
        endif()
    endif()

    # Add include
    foreach(include_dir ${ADD_INCLUDE})
        get_filename_component(abs_dir ${include_dir} ABSOLUTE BASE_DIR ${component_dir})
        if(NOT IS_DIRECTORY ${abs_dir})
            message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE}: ${include_dir} not found!")
        endif()

        set(inc_dirs ${g_ezos_inc_dirs})
        list(APPEND inc_dirs "${abs_dir}")
        set(g_ezos_inc_dirs ${inc_dirs} CACHE INTERNAL "g_ezos_inc_dirs")

        target_include_directories(${component_name} ${include_type} ${abs_dir})
    endforeach()

    # Add private include
    foreach(include_dir ${ADD_PRIVATE_INCLUDE})
        if(${include_type} STREQUAL INTERFACE)
            message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE}: ADD_PRIVATE_INCLUDE set but no source file！")
        endif()
        get_filename_component(abs_dir ${include_dir} ABSOLUTE BASE_DIR ${component_dir})
        if(NOT IS_DIRECTORY ${abs_dir})
            message(FATAL_ERROR "${CMAKE_CURRENT_LIST_FILE}: ${include_dir} not found!")
        endif()

        target_include_directories(${component_name} PRIVATE ${abs_dir})
    endforeach()

    # Add global config include
    if(${include_type} STREQUAL INTERFACE)
        target_include_directories(${component_name} INTERFACE ${ezos_config_dir})
    else()
        target_include_directories(${component_name} PUBLIC ${ezos_config_dir})
    endif()

    # Add requirements
    target_link_libraries(${component_name} ${include_type} ${ADD_REQUIREMENTS})

    # Add definitions public
    foreach(difinition ${ADD_DEFINITIONS})
        target_compile_options(${component_name} PUBLIC ${difinition})
    endforeach()

    # Add definitions private
    foreach(difinition ${ADD_DEFINITIONS_PRIVATE})
        target_compile_options(${component_name} PRIVATE ${difinition})
    endforeach()

    # Add static lib
    if(ADD_STATIC_LIB)
        foreach(lib ${ADD_STATIC_LIB})
            if(NOT EXISTS "${lib}")
                prepend(lib_full "${component_dir}/" ${lib})
                if(NOT EXISTS "${lib_full}")
                    message(FATAL_ERROR "Can not find ${lib} or ${lib_full}")
                endif()
                set(lib ${lib_full})
            endif()
            target_link_libraries(${component_name} ${include_type} ${lib})
        endforeach()
    endif()
    # Add dynamic lib
    if(ADD_DYNAMIC_LIB)
        set(dynamic_libs ${g_dynamic_libs})
        foreach(lib ${ADD_DYNAMIC_LIB})
            if(NOT EXISTS "${lib}")
                prepend(lib_full "${component_dir}/" ${lib})
                if(NOT EXISTS "${lib_full}")
                    message(FATAL_ERROR "Can not find ${lib} or ${lib_full}")
                endif()
                set(lib ${lib_full})
            endif()
            list(APPEND dynamic_libs ${lib})
            get_filename_component(lib_dir ${lib} DIRECTORY)
            get_filename_component(lib_name ${lib} NAME)
            target_link_libraries(${component_name} ${include_type} -L${lib_dir} ${lib_name})
        endforeach()
        set(g_dynamic_libs ${dynamic_libs}  CACHE INTERNAL "g_dynamic_libs")
    endif()
endfunction()

function(is_path_component ret param_path)
    set(res 1)
    get_filename_component(abs_dir ${param_path} ABSOLUTE)

    if(NOT IS_DIRECTORY "${abs_dir}")
        set(res 0)
    endif()

    get_filename_component(base_dir ${abs_dir} NAME)
    string(SUBSTRING "${base_dir}" 0 1 first_char)

    if(NOT first_char STREQUAL ".")
        if(NOT EXISTS "${abs_dir}/CMakeLists.txt")
            set(res 0)
        endif()
    else()
        set(res 0)
    endif()

    set(${ret} ${res} PARENT_SCOPE)
endfunction()

function(get_python python version info_str)
    set(res 1)
    execute_process(COMMAND python3 --version RESULT_VARIABLE cmd_res OUTPUT_VARIABLE cmd_out)
    if(${cmd_res} EQUAL 0)
        set(${python} python3 PARENT_SCOPE)
        set(${version} 3 PARENT_SCOPE)
        set(${info_str} ${cmd_out} PARENT_SCOPE)
    else()
        execute_process(COMMAND python --version RESULT_VARIABLE cmd_res OUTPUT_VARIABLE cmd_out)
        if(${cmd_res} EQUAL 0)
            set(${python} python PARENT_SCOPE)
            set(${version} 2 PARENT_SCOPE)
            set(${info_str} ${cmd_out} PARENT_SCOPE)
        endif()
    endif()
endfunction(get_python python)

# 1.Compile the ezos component without linking
# 2.Generate global build configuration by menuconfig
# 3.Generate link dependencies info
macro(do_lib_building name)
    get_filename_component(current_dir ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
    set(PROJECT_SOURCE_DIR ${current_dir})
    set(PROJECT_BINARY_DIR "${current_dir}/libs")

    # Find components in ezos's components folder, register components
    file(GLOB component_dirs ${EZOS_PATH}/components/*)
    foreach(component_dir ${component_dirs})
        is_path_component(is_component ${component_dir})
        if(is_component)
            message(STATUS "Find component: ${component_dir}")
            get_filename_component(base_dir ${component_dir} NAME)
            list(APPEND components_dirs ${component_dir})
            if(EXISTS ${component_dir}/Kconfig)
                message(STATUS "Find component Kconfig of ${base_dir}")
                list(APPEND components_kconfig_files ${component_dir}/Kconfig)
            endif()
            if(EXISTS ${component_dir}/.config_defaults)
                message(STATUS "Find component defaults config of ${base_dir}")
                list(APPEND kconfig_defaults_files_args --defaults "${component_dir}/.config_defaults")
            endif()
        endif()
    endforeach()

    # Find components in eziot folder, register components
    file(GLOB component_dirs ${EZOS_PATH}/eziot/*)
    foreach(component_dir ${component_dirs})
        is_path_component(is_component ${component_dir})
        if(is_component)
            message(STATUS "Find component: ${component_dir}")
            get_filename_component(base_dir ${component_dir} NAME)
            list(APPEND components_dirs ${component_dir})
            if(EXISTS ${component_dir}/Kconfig)
                message(STATUS "Find component Kconfig of ${base_dir}")
                list(APPEND components_kconfig_files ${component_dir}/Kconfig)
            endif()
            if(EXISTS ${component_dir}/.config_defaults)
                message(STATUS "Find component defaults config of ${base_dir}")
                list(APPEND kconfig_defaults_files_args --defaults "${component_dir}/.config_defaults")
            endif()
        endif()
    endforeach()

    # Find components in bsp folder
    file(GLOB project_component_dirs ${EZOS_PATH}/platform/bsp/${PROJECT_NAME}/ezos)
    foreach(component_dir ${project_component_dirs})
        is_path_component(is_component ${component_dir})
        if(is_component)
            message(STATUS "find component: ${component_dir}")
            get_filename_component(base_dir ${component_dir} NAME)
            list(APPEND components_dirs ${component_dir})

            if(EXISTS ${component_dir}/Kconfig)
                message(STATUS "Find component Kconfig of ${base_dir}")
                list(APPEND components_kconfig_files ${component_dir}/Kconfig)
            endif()
        endif()
    endforeach()

    # Find components in project folder
    file(GLOB project_component_dirs ${PROJECT_SOURCE_DIR}/*)
    foreach(component_dir ${project_component_dirs})
        is_path_component(is_component ${component_dir})
        if(is_component)
            message(STATUS "find component: ${component_dir}")
            get_filename_component(base_dir ${component_dir} NAME)
            list(APPEND components_dirs ${component_dir})

            if(EXISTS ${component_dir}/Kconfig)
                message(STATUS "Find component Kconfig of ${base_dir}")
                list(APPEND components_kconfig_files ${component_dir}/Kconfig)
            endif()
            if(EXISTS ${component_dir}/config_defaults.mk)
                message(STATUS "Find component defaults config of ${base_dir}")
                list(APPEND kconfig_defaults_files_args --defaults "${component_dir}/config_defaults.mk")
            endif()
        endif()
    endforeach()

    # Find default config file
    if(EXISTS ${PROJECT_SOURCE_DIR}/.config_defaults)
        message(STATUS "Find project defaults config(.config_defaults)")
        list(APPEND kconfig_defaults_files_args --defaults "${PROJECT_SOURCE_DIR}/.config_defaults")
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/.config)
        message(STATUS "Find project defaults config(.config)")
        list(APPEND kconfig_defaults_files_args --defaults "${PROJECT_SOURCE_DIR}/.config")
    endif()

    # Generate config file from Kconfig
    get_python(python python_version python_info_str)
    if(NOT python)
        message(FATAL_ERROR "python not found, please install python firstly(python3 recommend)!")
    endif()

    message(STATUS "python command: ${python}, version: ${python_info_str}")
    string(REPLACE ";" " " components_kconfig_files "${kconfig_defaults_files_args}")
    string(REPLACE ";" " " components_kconfig_files "${components_kconfig_files}")

    set(generate_config_cmd ${python}  ${EZOS_PATH}/tools/kconfig/genconfig.py
                        --kconfig "${EZOS_PATH}/Kconfig"
                        ${kconfig_defaults_files_args}
                        --menuconfig False
                        --env "EZOS_PATH=${EZOS_PATH}"
                        --env "PROJECT_KCONFIG_PATH=${PROJECT_KCONFIG_PATH}"
                        --env "PROJECT_PATH=${PROJECT_SOURCE_DIR}"
                        --env "PROJECT_NAME=${PROJECT_NAME}"
                        --output config ${PROJECT_PATH}/config/.config
                        --output makefile ${PROJECT_PATH}/config/ezos_config.mk
                        --output cmake  ${PROJECT_PATH}/config/ezos_config.cmake
                        --output header ${PROJECT_PATH}/config/ezos_config.h
                        )

    set(generate_config_cmd_gui ${python}  ${EZOS_PATH}/tools/kconfig/genconfig.py
                        --kconfig "${EZOS_PATH}/Kconfig"
                        ${kconfig_defaults_files_args}
                        --menuconfig True
                        --env "EZOS_PATH=${EZOS_PATH}"
                        --env "PROJECT_KCONFIG_PATH=${PROJECT_KCONFIG_PATH}"
                        --env "PROJECT_PATH=${PROJECT_SOURCE_DIR}"
                        --env "PROJECT_NAME=${PROJECT_NAME}"
                        --output config ${PROJECT_PATH}/config/.config
                        --output makefile ${PROJECT_PATH}/config/ezos_config.mk
                        --output cmake  ${PROJECT_PATH}/config/ezos_config.cmake
                        --output header ${PROJECT_PATH}/config/ezos_config.h
                        )

    execute_process(COMMAND ${generate_config_cmd} RESULT_VARIABLE cmd_res)

    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "Check Kconfig content")
    endif()

    # Include confiurations
    set(ezos_config_dir "${PROJECT_PATH}/config")
    include(${ezos_config_dir}/ezos_config.cmake)
    if(WIN32)
        set(EXT ".exe")
        set(DL_EXT ".dll")
    else()
        set(EXT "")
        set(DL_EXT ".so")
    endif()

    # Config toolchain
    if(CONFIG_TOOLCHAIN_PATH)
        if(WIN32)
            file(TO_CMAKE_PATH ${CONFIG_TOOLCHAIN_PATH} CONFIG_TOOLCHAIN_PATH)
        endif()
        if(NOT IS_DIRECTORY ${CONFIG_TOOLCHAIN_PATH})
            message(FATAL_ERROR "TOOLCHAIN_PATH set error:${CONFIG_TOOLCHAIN_PATH}")
        endif()
        set(TOOLCHAIN_PATH ${CONFIG_TOOLCHAIN_PATH})
        message(STATUS "TOOLCHAIN_PATH:${CONFIG_TOOLCHAIN_PATH}")
        set(CMAKE_C_COMPILER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}gcc${EXT}")
        set(CMAKE_CXX_COMPILER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}g++${EXT}")
        set(CMAKE_ASM_COMPILER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}gcc${EXT}")
        set(CMAKE_LINKER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}ld${EXT}")
    else()
        set(CMAKE_C_COMPILER "gcc${EXT}")
        set(CMAKE_CXX_COMPILER "g++${EXT}")
        set(CMAKE_ASM_COMPILER "gcc${EXT}")
        set(CMAKE_LINKER  "ld${EXT}")
    endif()

    include(${EZOS_PATH}/tools/cmake/compile_flags.cmake)

    message(STATUS "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
    message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
    message(STATUS "CMAKE_C_LINK_FLAGS: ${CMAKE_C_LINK_FLAGS}")
    message(STATUS "CMAKE_CXX_LINK_FLAGS: ${CMAKE_CXX_LINK_FLAGS}")

    # Add dependence: update configfile, append time and git info for global config header file
    # we didn't generate build info for cmake and makefile for if we do, it will always rebuild cmake
    # everytime we execute make
    set(gen_build_info_config_cmd ${python}  ${EZOS_PATH}/tools/kconfig/update_build_info.py
                                  --configfile header ${PROJECT_PATH}/config/ezos_build_info_time.h ${PROJECT_PATH}/config/ezos_build_info_version.h
                                  )
    add_custom_target(update_build_info COMMAND ${gen_build_info_config_cmd})

    # Sort component according to priority.conf config file
    set(component_priority_conf_file "${PROJECT_PATH}/compile/priority.conf")
    set(sort_components ${python}  ${EZOS_PATH}/tools/cmake/sort_components.py
                                   ${component_priority_conf_file} ${components_dirs}
                        )

    execute_process(COMMAND ${sort_components} OUTPUT_VARIABLE component_dirs_sorted RESULT_VARIABLE cmd_res)
    if(cmd_res EQUAL 2)
        message(STATUS "No components priority config file")
        set(component_dirs_sorted ${components_dirs})
    elseif(cmd_res EQUAL 0)
        message(STATUS "Config components priority success")
    else()
        message(STATUS "Components priority config fail ${component_dirs_sorted}, check config file:${component_priority_conf_file}")
    endif()

    # Call CMakeLists.txt
    foreach(component_dir ${component_dirs_sorted})
        get_filename_component(base_dir ${component_dir} NAME)
        if(${base_dir} STREQUAL "main")
            set(base_dir ${name}) #rename
        endif()
        add_subdirectory(${component_dir} ${base_dir})

        if(TARGET ${base_dir})
            add_dependencies(${base_dir} update_build_info) # add build info dependence

            set(lib_depens ${g_ezos_lib_depens})
            list(APPEND lib_depens "-l${base_dir}")
            set(g_ezos_lib_depens ${lib_depens}  CACHE INTERNAL "g_ezos_lib_depens")
        else()
            message(STATUS "component ${base_dir} not enabled")
        endif()
    endforeach()

    string(REPLACE ";" " " g_ezos_inc_dirs "${g_ezos_inc_dirs}")
    string(REPLACE ";" " " g_ezos_lib_dirs "${g_ezos_lib_dirs}")
    string(REPLACE ";" " " g_ezos_lib_depens "${g_ezos_lib_depens}")

    # Remove duplicate dynamic libs from var g_dynamic_libs
    set(dynamic_libs_abs "")
    foreach(item ${g_dynamic_libs})
    get_filename_component(item ${item} ABSOLUTE)
    list(APPEND dynamic_libs_abs ${item})
    endforeach()
    set(g_dynamic_libs ${dynamic_libs_abs})
    list(REMOVE_DUPLICATES g_dynamic_libs)

    set(update_config_cmd ${python}  ${EZOS_PATH}/tools/kconfig/genconfig.py
                        --kconfig "${EZOS_PATH}/Kconfig"
                        ${kconfig_defaults_files_args}
                        --menuconfig False
                        --env "EZOS_PATH=${EZOS_PATH}"
                        --env "PROJECT_KCONFIG_PATH=${PROJECT_KCONFIG_PATH}"
                        --env "PROJECT_PATH=${PROJECT_SOURCE_DIR}"
                        --env "PROJECT_NAME=${PROJECT_NAME}"
                        --output config ${PROJECT_PATH}/config/.config
                        --output makefile ${PROJECT_PATH}/config/ezos_config.mk
                        --output cmake  ${PROJECT_PATH}/config/ezos_config.cmake
                        --output header ${PROJECT_PATH}/config/ezos_config.h
                        --addkv "ADD_EZOS_INC_DIRS" ${g_ezos_inc_dirs}
                        --addkv "ADD_EZOS_LIB_DIRS" ${g_ezos_lib_dirs}
                        --addkv "ADD_EZOS_LIB_DEPENS" ${g_ezos_lib_depens}
                        )

    execute_process(COMMAND ${update_config_cmd} RESULT_VARIABLE cmd_res)
    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "Update Kconfig content")
    endif()

    # Add menuconfig target for makefile
    add_custom_target(menuconfig COMMAND ${generate_config_cmd_gui})

endmacro()

# Compile the ezos component and linking
macro(do_project_building name)
    get_filename_component(current_dir ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
    set(PROJECT_SOURCE_DIR ${current_dir})
    set(PROJECT_BINARY_DIR "${current_dir}/build")

    # Find components in SDK's components folder, register components
    file(GLOB component_dirs ${EZOS_PATH}/components/*)
    foreach(component_dir ${component_dirs})
        is_path_component(is_component ${component_dir})
        if(is_component)
            message(STATUS "Find component: ${component_dir}")
            get_filename_component(base_dir ${component_dir} NAME)
            list(APPEND components_dirs ${component_dir})
            if(EXISTS ${component_dir}/Kconfig)
                message(STATUS "Find component Kconfig of ${base_dir}")
                list(APPEND components_kconfig_files ${component_dir}/Kconfig)
            endif()
            if(EXISTS ${component_dir}/.config_defaults)
                message(STATUS "Find component defaults config of ${base_dir}")
                list(APPEND kconfig_defaults_files_args --defaults "${component_dir}/.config_defaults")
            endif()
        endif()
    endforeach()

    # Find components in eziot folder, register components
    file(GLOB component_dirs ${EZOS_PATH}/eziot/*)
    foreach(component_dir ${component_dirs})
        is_path_component(is_component ${component_dir})
        if(is_component)
            message(STATUS "Find component: ${component_dir}")
            get_filename_component(base_dir ${component_dir} NAME)
            list(APPEND components_dirs ${component_dir})
            if(EXISTS ${component_dir}/Kconfig)
                message(STATUS "Find component Kconfig of ${base_dir}")
                list(APPEND components_kconfig_files ${component_dir}/Kconfig)
            endif()
            if(EXISTS ${component_dir}/.config_defaults)
                message(STATUS "Find component defaults config of ${base_dir}")
                list(APPEND kconfig_defaults_files_args --defaults "${component_dir}/.config_defaults")
            endif()
        endif()
    endforeach()

    file(GLOB project_component_dirs ${PROJECT_SOURCE_DIR}/*)
    foreach(component_dir ${project_component_dirs})
        is_path_component(is_component ${component_dir})

        if(is_component)
            message(STATUS "find component: ${component_dir}")
            get_filename_component(base_dir ${component_dir} NAME)
            list(APPEND components_dirs ${component_dir})
            if(${base_dir} STREQUAL "main")
                set(main_component 1)
            endif()
            if(EXISTS ${component_dir}/Kconfig)
                message(STATUS "Find component Kconfig of ${base_dir}")
                list(APPEND components_kconfig_files ${component_dir}/Kconfig)
            endif()
            if(EXISTS ${component_dir}/config_defaults.mk)
                message(STATUS "Find component defaults config of ${base_dir}")
                list(APPEND kconfig_defaults_files_args --defaults "${component_dir}/config_defaults.mk")
            endif()
        endif()
    endforeach()

   if(NOT main_component)
       message(FATAL_ERROR "=================\nCan not find main component(folder) in project folder!!\n=================")
   endif()

    # Find components in bsp folder
    file(GLOB project_component_dirs ${EZOS_PATH}/platform/bsp/${PROJECT_NAME}/ezos)
    foreach(component_dir ${project_component_dirs})
        is_path_component(is_component ${component_dir})
        if(is_component)
            message(STATUS "find component: ${component_dir}")
            get_filename_component(base_dir ${component_dir} NAME)
            list(APPEND components_dirs ${component_dir})

            if(EXISTS ${component_dir}/Kconfig)
                message(STATUS "Find component Kconfig of ${base_dir}")
                list(APPEND components_kconfig_files ${component_dir}/Kconfig)
            endif()
        endif()
    endforeach()

    # Find default config file
    if(EXISTS ${PROJECT_SOURCE_DIR}/.config_defaults)
        message(STATUS "Find project defaults config(.config_defaults)")
        list(APPEND kconfig_defaults_files_args --defaults "${PROJECT_SOURCE_DIR}/.config_defaults")
    endif()

    if(EXISTS ${PROJECT_SOURCE_DIR}/.config)
        message(STATUS "Find project defaults config(.config)")
        list(APPEND kconfig_defaults_files_args --defaults "${PROJECT_SOURCE_DIR}/.config")
    endif()

    # Generate config file from Kconfig
    get_python(python python_version python_info_str)
    if(NOT python)
        message(FATAL_ERROR "python not found, please install python firstly(python3 recommend)!")
    endif()
    message(STATUS "python command: ${python}, version: ${python_info_str}")
    string(REPLACE ";" " " components_kconfig_files "${kconfig_defaults_files_args}")
    string(REPLACE ";" " " components_kconfig_files "${components_kconfig_files}")

    set(generate_config_cmd ${python}  ${EZOS_PATH}/tools/kconfig/genconfig.py
                        --kconfig "${EZOS_PATH}/Kconfig"
                        ${kconfig_defaults_files_args}
                        --menuconfig False
                        --env "EZOS_PATH=${EZOS_PATH}"
                        --env "PROJECT_KCONFIG_PATH=${PROJECT_KCONFIG_PATH}"
                        --env "PROJECT_PATH=${PROJECT_SOURCE_DIR}"
                        --env "PROJECT_NAME=${PROJECT_NAME}"
                        --output config ${PROJECT_PATH}/config/.config
                        --output makefile ${PROJECT_PATH}/config/ezos_config.mk
                        --output cmake  ${PROJECT_PATH}/config/ezos_config.cmake
                        --output header ${PROJECT_PATH}/config/ezos_config.h
                        )

    set(generate_config_cmd_gui ${python}  ${EZOS_PATH}/tools/kconfig/genconfig.py
                        --kconfig "${EZOS_PATH}/Kconfig"
                        ${kconfig_defaults_files_args}
                        --menuconfig True
                        --env "EZOS_PATH=${EZOS_PATH}"
                        --env "PROJECT_KCONFIG_PATH=${PROJECT_KCONFIG_PATH}"
                        --env "PROJECT_PATH=${PROJECT_SOURCE_DIR}"
                        --env "PROJECT_NAME=${PROJECT_NAME}"
                        --output config ${PROJECT_PATH}/config/.config
                        --output makefile ${PROJECT_PATH}/config/ezos_config.mk
                        --output cmake  ${PROJECT_PATH}/config/ezos_config.cmake
                        --output header ${PROJECT_PATH}/config/ezos_config.h
                        )

    execute_process(COMMAND ${generate_config_cmd} RESULT_VARIABLE cmd_res)
    if(NOT cmd_res EQUAL 0)
        message(FATAL_ERROR "Check Kconfig content")
    endif()

    # Include confiurations
    set(ezos_config_dir "${PROJECT_PATH}/config")
    include(${ezos_config_dir}/ezos_config.cmake)
    if(WIN32)
        set(EXT ".exe")
        set(DL_EXT ".dll")
    else()
        set(EXT "")
        set(DL_EXT ".so")
    endif()

    # Config toolchain
    if(CONFIG_TOOLCHAIN_PATH)
        if(WIN32)
            file(TO_CMAKE_PATH ${CONFIG_TOOLCHAIN_PATH} CONFIG_TOOLCHAIN_PATH)
        endif()
        if(NOT IS_DIRECTORY ${CONFIG_TOOLCHAIN_PATH})
            message(FATAL_ERROR "TOOLCHAIN_PATH set error:${CONFIG_TOOLCHAIN_PATH}")
        endif()
        set(TOOLCHAIN_PATH ${CONFIG_TOOLCHAIN_PATH})
        message(STATUS "TOOLCHAIN_PATH:${CONFIG_TOOLCHAIN_PATH}")
        set(CMAKE_C_COMPILER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}gcc${EXT}")
        set(CMAKE_CXX_COMPILER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}g++${EXT}")
        set(CMAKE_ASM_COMPILER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}gcc${EXT}")
        set(CMAKE_LINKER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}ld${EXT}")
    else()
        set(CMAKE_C_COMPILER "gcc${EXT}")
        set(CMAKE_CXX_COMPILER "g++${EXT}")
        set(CMAKE_ASM_COMPILER "gcc${EXT}")
        set(CMAKE_LINKER  "ld${EXT}")
    endif()

    set(CMAKE_C_COMPILER_WORKS 1)
    set(CMAKE_CXX_COMPILER_WORKS 1)

    include(${EZOS_PATH}/tools/cmake/compile_flags.cmake)

    message(STATUS "CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
    message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
    message(STATUS "CMAKE_C_LINK_FLAGS: ${CMAKE_C_LINK_FLAGS}")
    message(STATUS "CMAKE_CXX_LINK_FLAGS: ${CMAKE_CXX_LINK_FLAGS}")

    # Add dependence: update configfile, append time and git info for global config header file
    # we didn't generate build info for cmake and makefile for if we do, it will always rebuild cmake
    # everytime we execute make
    set(gen_build_info_config_cmd ${python}  ${EZOS_PATH}/tools/kconfig/update_build_info.py
                                  --configfile header ${PROJECT_PATH}/config/ezos_build_info_time.h ${PROJECT_PATH}/config/ezos_build_info_version.h
                                  )
    add_custom_target(update_build_info COMMAND ${gen_build_info_config_cmd})

    # Sort component according to priority.conf config file
    set(component_priority_conf_file "${PROJECT_PATH}/compile/priority.conf")
    set(sort_components ${python}  ${EZOS_PATH}/tools/cmake/sort_components.py
                                   ${component_priority_conf_file} ${components_dirs}
                        )
    execute_process(COMMAND ${sort_components} OUTPUT_VARIABLE component_dirs_sorted RESULT_VARIABLE cmd_res)
    if(cmd_res EQUAL 2)
        message(STATUS "No components priority config file")
        set(component_dirs_sorted ${components_dirs})
    elseif(cmd_res EQUAL 0)
        message(STATUS "Config components priority success")
    else()
        message(STATUS "Components priority config fail ${component_dirs_sorted}, check config file:${component_priority_conf_file}")
    endif()

    # Call CMakeLists.txt
    foreach(component_dir ${component_dirs_sorted})
        get_filename_component(base_dir ${component_dir} NAME)
        add_subdirectory(${component_dir} ${base_dir})
        if(TARGET ${base_dir})
            add_dependencies(${base_dir} update_build_info) # add build info dependence
            message(STATUS "component ${base_dir}, ${update_build_info}")

        else()
            message(STATUS "component ${base_dir} not enabled")
        endif()
    endforeach()

    # Remove duplicate dynamic libs from var g_dynamic_libs
    set(dynamic_libs_abs "")
    foreach(item ${g_dynamic_libs})
        get_filename_component(item ${item} ABSOLUTE)
        list(APPEND dynamic_libs_abs ${item})
    endforeach()
    set(g_dynamic_libs ${dynamic_libs_abs})
    list(REMOVE_DUPLICATES g_dynamic_libs)

    # Add menuconfig target for makefile
    add_custom_target(menuconfig COMMAND ${generate_config_cmd_gui})

    # Create dummy source file exe_src.c to satisfy cmake's `add_executable` interface!
    set(exe_src ${CMAKE_BINARY_DIR}/exe_src.c)
    add_executable(${name} "${exe_src}")
    add_custom_command(OUTPUT ${exe_src} COMMAND ${CMAKE_COMMAND} -E touch ${exe_src} VERBATIM)
    add_custom_target(gen_exe_src DEPENDS "${exe_src}")
    add_dependencies(${name} gen_exe_src)

    # Add main component(lib)
    target_link_libraries(${name} main)

endmacro()