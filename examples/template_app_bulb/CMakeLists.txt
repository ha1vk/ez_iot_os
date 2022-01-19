if(EZIOT_EXAMPLES_BULB_ENABLE)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

####### Prepare building environment ##########
get_filename_component(ENV_EZOS_PATH ../.. ABSOLUTE)
get_filename_component(ENV_CURRENT_PATH ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
get_filename_component(BULB_PROJECT_NAME ${ENV_CURRENT_PATH} NAME)
SET(ENV_PLATFORM_NAME "linux")

SET(ENV_EXAMPLE_BULB_PATH ${ENV_CURRENT_PATH}/../..)
SET(ENV_UTCASE_PATH ${ENV_CURRENT_PATH}/../../case)
SET(ENV_BULBPORT_PATH ${ENV_CURRENT_PATH})
SET(ENV_PLATFORM_PATH ${ENV_EZOS_PATH}/platform)
SET(ENV_CONFIG_PATH ${ENV_EZOS_PATH}/platform/bsp/${ENV_PLATFORM_NAME}/config)
SET(ENV_LIB_PATH ${ENV_EZOS_PATH}/platform/bsp/${ENV_PLATFORM_NAME}/build)
SET(ENV_COMPONENT_PATH ${ENV_EZOS_PATH}/components)
SET(ENV_EZIOT_PATH ${ENV_EZOS_PATH}/eziot)

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

set(CMAKE_BUILD_TYPE "Debug")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-unused-variable -std=c++11")
else()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wno-unused-variable")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

##############################################根据开关定义编译的文件和链接的库#############################################

    message(STATUS "CONFIG_EZIOT_CORE_ENABLE")

    list(APPEND BULB_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_core/inc)
	list(APPEND BULB_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_bm/base/inc)
	list(APPEND BULB_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_bm/tsl/inc)
    list(APPEND BULB_INC_DIRS ${ENV_CONFIG_PATH})
    list(APPEND BULB_INC_DIRS ${ENV_COMPONENT_PATH}/utest/inc)
    list(APPEND BULB_INC_DIRS ${ENV_COMPONENT_PATH}/ezlog/inc)
    list(APPEND BULB_INC_DIRS ${ENV_PLATFORM_PATH}/inc)
	list(APPEND BULB_INC_DIRS ${ENV_COMPONENT_PATH}/FlashDB/inc)

	
	
    list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/ezos)
	list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/ez_iot_bm)
    list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/ezlog)
    list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/cJSON)
    list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/mqtt)
    list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/ezxml)
    list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/mbedtls)
    list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/ez_iot_core)
	
	list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/FlashDB)
	

	list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/webclient)
    list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/ezutil)
    list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/ezlist)
	list(APPEND BULB_DEPEN_DIRS ${ENV_LIB_PATH}/ez_iot_bm)
	
	
	
	list(APPEND BULB_DEPENS -lpthread)
	list(APPEND BULB_DEPENS -lcJSON)
    list(APPEND BULB_DEPENS -lmqtt)
    list(APPEND BULB_DEPENS -lezxml)
	list(APPEND BULB_DEPENS -lezos)
	list(APPEND BULB_DEPENS -lmbedtls)
	list(APPEND BULB_DEPENS -lez_iot_core)   
	
    list(APPEND BULB_DEPENS -lezlog)
    list(APPEND BULB_DEPENS -lmbedtls)	
	list(APPEND BULB_DEPENS -lFlashDB)
	
	list(APPEND BULB_DEPENS -lwebclient)
    list(APPEND BULB_DEPENS -lezutil)
    list(APPEND BULB_DEPENS -lezlist)
	list(APPEND BULB_DEPENS -lez_iot_bm)
		
	
##############################################根据开关定义编译的文件和链接的库#############################################

#预编译宏


#头文件搜索路径
list(APPEND BULB_INC_DIRS ${ENV_CURRENT_PATH}/include)
INCLUDE_DIRECTORIES(${BULB_INC_DIRS})

#依赖库目录
LINK_DIRECTORIES(${BULB_DEPEN_DIRS})

#源文件
AUX_SOURCE_DIRECTORY(. DIR_SRCS)
ADD_EXECUTABLE(bulb_app ${DIR_SRCS}  )


list(REVERSE BULB_DEPENS)
message(STATUS "${BULB_DEPENS}")
#链接依赖库
target_link_libraries(bulb_app ${BULB_DEPENS})
endif()