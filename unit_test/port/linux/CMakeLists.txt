CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

####### Prepare building environment ##########
get_filename_component(ENV_EZOS_PATH ../../../ ABSOLUTE)
get_filename_component(ENV_CURRENT_PATH ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
get_filename_component(ENV_PROJECT_NAME ${ENV_CURRENT_PATH} NAME)

SET(ENV_UT_PATH ${ENV_CURRENT_PATH}/../..)
SET(ENV_UTCASE_PATH ${ENV_CURRENT_PATH}/../../case)
SET(ENV_UTPORT_PATH ${ENV_CURRENT_PATH})
SET(ENV_CONFIG_PATH ${ENV_EZOS_PATH}/platform/mcu/${ENV_PROJECT_NAME}/config)
SET(ENV_LIB_PATH ${ENV_EZOS_PATH}/platform/mcu/${ENV_PROJECT_NAME}/build)
SET(ENV_COMPONENT_PATH ${ENV_EZOS_PATH}/components)

include(${ENV_CONFIG_PATH}/ezos_gconfig.cmake)
###############################################

############### Config toolchain ##############
if(CONFIG_TOOLCHAIN_PATH)
    if(NOT IS_DIRECTORY ${CONFIG_TOOLCHAIN_PATH})
        message(FATAL_ERROR "TOOLCHAIN_PATH set error:${CONFIG_TOOLCHAIN_PATH}")
    endif()
    set(TOOLCHAIN_PATH ${CONFIG_TOOLCHAIN_PATH})
    message(STATUS "TOOLCHAIN_PATH:${CONFIG_TOOLCHAIN_PATH}")
    set(CMAKE_C_COMPILER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}gcc")
    set(CMAKE_CXX_COMPILER "${CONFIG_TOOLCHAIN_PATH}/${CONFIG_TOOLCHAIN_PREFIX}g++")
else()
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")
endif()
###############################################

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-unused-variable  -std=c++11")
else()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wno-unused-variable")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

##############################################根据开关定义编译的文件和链接的库#############################################

if(CONFIG_EZIOT_COMPONENT_UTEST_ENABLE)
    message(STATUS "CONFIG_EZIOT_COMPONENT_UTEST_ENABLE")
    list(APPEND UT_SRCS ${ENV_UTPORT_PATH}/ut_entry.c)
    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_hello.c)

    list(APPEND UT_INC_DIRS ${ENV_CONFIG_PATH})
    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/utest/inc)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/utest)

    list(APPEND UT_DEPENS -lrt)
    list(APPEND UT_DEPENS -lm)
    list(APPEND UT_DEPENS -lpthread)
    list(APPEND UT_DEPENS -lutest)

    list(APPEND UT_DEFINITIONS -DUTEST_TC_USE_CONSTRUCTOR)
else()
    message(FATAL_ERROR "The utest component must be enabled!")
endif()

if(CONFIG_EZIOT_COMPONENT_FLASHDB_ENABLE)
    message(STATUS "CONFIG_EZIOT_COMPONENT_FLASHDB_ENABLE")
    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_flashdb.c)
    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/kv_imp.c)

    list(APPEND UT_INC_DIRS ${ENV_UTPORT_PATH})
    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/utest/inc)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/flashdb)

    list(APPEND UT_DEPENS -lflashdb)

    list(APPEND UT_DEFINITIONS -DUTEST_TC_USE_CONSTRUCTOR)
endif()

##############################################根据开关定义编译的文件和链接的库#############################################

#预编译宏
ADD_DEFINITIONS(${UT_DEFINITIONS})

#头文件搜索路径
INCLUDE_DIRECTORIES(${UT_INC_DIRS})

#依赖库目录
LINK_DIRECTORIES(${UT_DEPEN_DIRS})

#源文件
ADD_EXECUTABLE(ut_iotsdk ${UT_SRCS})

#链接依赖库
target_link_libraries(ut_iotsdk ${UT_DEPENS})