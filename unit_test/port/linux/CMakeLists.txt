CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

####### Prepare building environment ##########
get_filename_component(ENV_EZOS_PATH ../../../ ABSOLUTE)
get_filename_component(ENV_CURRENT_PATH ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
get_filename_component(ENV_PROJECT_NAME ${ENV_CURRENT_PATH} NAME)

SET(ENV_UT_PATH ${ENV_CURRENT_PATH}/../..)
SET(ENV_UTCASE_PATH ${ENV_CURRENT_PATH}/../../case)
SET(ENV_UTPORT_PATH ${ENV_CURRENT_PATH})
SET(ENV_PLATFORM_PATH ${ENV_EZOS_PATH}/platform)
SET(ENV_CONFIG_PATH ${ENV_EZOS_PATH}/platform/bsp/${ENV_PROJECT_NAME}/config)
SET(ENV_LIB_PATH ${ENV_EZOS_PATH}/platform/bsp/${ENV_PROJECT_NAME}/build)
SET(ENV_COMPONENT_PATH ${ENV_EZOS_PATH}/components)
SET(ENV_EZIOT_PATH ${ENV_EZOS_PATH}/eziot)

include(${ENV_CONFIG_PATH}/ezos_config.cmake)
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

list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_ezos.c)

if(CONFIG_EZIOT_COMPONENT_UTEST_ENABLE)
    message(STATUS "CONFIG_EZIOT_COMPONENT_UTEST_ENABLE")
    list(APPEND UT_SRCS ${ENV_UTPORT_PATH}/ut_entry.c)
    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_hello.c)
    # list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_gateway.c)

    list(APPEND UT_INC_DIRS ${ENV_CONFIG_PATH})
    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/utest/inc)
    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/ezlog/inc)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/utest)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/cli)

    list(APPEND UT_DEPENS -lrt)
    list(APPEND UT_DEPENS -lm)
    list(APPEND UT_DEPENS -lpthread)
    list(APPEND UT_DEPENS -lutest)
    list(APPEND UT_DEPENS -lcli)

    list(APPEND UT_DEFINITIONS -DUTEST_TC_USE_CONSTRUCTOR)
else()
    message(FATAL_ERROR "The utest component must be enabled!")
endif()

if(CONFIG_EZIOT_COMPONENT_CLI_ENABLE)
    message(STATUS "CONFIG_EZIOT_COMPONENT_CLI_ENABLE")
    list(APPEND UT_DEPENDS -lcli)

    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezos)
    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/cli/inc)
    list(APPEND UT_INC_DIRS ${ENV_PLATFORM_PATH}/inc)
    list(APPEND UT_DEPENS -lezos)
endif()

if(CONFIG_EZIOT_COMPONENT_EZLOG_ENABLE)

    message(STATUS "CONFIG_EZIOT_COMPONENT_EZLOG_ENABLE")
    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_ezlog.c)

    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/ezlog/inc)
    list(APPEND UT_INC_DIRS ${ENV_PLATFORM_PATH}/inc)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezos)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezlog)

    list(APPEND UT_DEPENS -lezos)
    list(APPEND UT_DEPENS -lezlog)

endif()

if(CONFIG_EZOS_KV_ENABLE)

    message(STATUS "CONFIG_EZOS_KV_ENABLE")

    list(APPEND UT_INC_DIRS ${ENV_PLATFORM_PATH}/inc)
    list(APPEND UT_INC_DIRS ${ENV_UTPORT_PATH})

    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_ezos_kv.c)
    list(APPEND UT_SRCS ${ENV_UTPORT_PATH}/kv_imp.c)

    list(APPEND UT_DEPENS -lezos)

endif()

if(CONFIG_EZIOT_COMPONENT_WEBCLIENT_ENABLE)

    message(STATUS "CONFIG_COMPONENT_EZIOT_WEBCLIENT_ENABLE")

    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/webclient/inc)

    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_webclient.c)

    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/webclient)

    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezos)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezlog)

    list(APPEND UT_DEPENS -lezos)
    list(APPEND UT_DEPENS -lezlog)

    list(APPEND UT_DEPENS -lwebclient)
    list(APPEND UT_DEPENS -lezlog)
endif()
if(CONFIG_EZIOT_CORE_ENABLE)

    message(STATUS "CONFIG_EZIOT_CORE_ENABLE")

    list(APPEND UT_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_core/inc)

    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_online.c)

    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezos)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezlog)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/cJSON)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/mqtt)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezxml)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/mbedtls)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ez_iot_core)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/eztimer)

    list(APPEND UT_DEPENS -lezos)
    list(APPEND UT_DEPENS -lezlog)
    list(APPEND UT_DEPENS -lcJSON)
    list(APPEND UT_DEPENS -lmqtt)
    list(APPEND UT_DEPENS -lezxml)
    list(APPEND UT_DEPENS -lmbedtls)
    list(APPEND UT_DEPENS -lez_iot_core)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/eztimer)

endif()


if(CONFIG_EZIOT_COMPONENT_FLASHDB_ENABLE)
    message(STATUS "CONFIG_EZIOT_COMPONENT_FLASHDB_ENABLE")
    list(APPEND UT_SRCS ${ENV_UTPORT_PATH}/kv_imp.c)

    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/FlashDB/inc)
    list(APPEND UT_INC_DIRS ${ENV_UTPORT_PATH})

    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/FlashDB)

    list(APPEND UT_DEPENS -lFlashDB)
    list(APPEND UT_DEFINITIONS -DUTEST_TC_USE_CONSTRUCTOR)
endif()

if(CONFIG_EZIOT_BASE_ENABLE)

    message(STATUS "CONFIG_EZIOT_BASE_ENABLE")

    list(APPEND UT_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_bm/base/inc)
    list(APPEND UT_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_bm/hub/inc)

    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_bind.c)
    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_gateway.c)

    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ez_iot_bm)
    list(APPEND UT_DEPENS -lez_iot_bm)

endif()

if(CONFIG_EZIOT_OTA_ENABLE)
     message(STATUS "CONFIG_EZIOT_OTA_ENABLE")

     list(APPEND UT_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_bm/ota/inc)
    list(APPEND UT_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_core/inc)
     list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/ezlog/inc)

     list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_ota.c)

     list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ez_iot_bm)
     list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/webclient)

     list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezos)
     list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezlog)
     list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/cJSON)
     list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/mqtt)
     list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezxml)
     list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/mbedtls)
     list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ez_iot_core)

     list(APPEND UT_DEPENS -lezos)
     list(APPEND UT_DEPENS -lezlog)
     list(APPEND UT_DEPENS -lcJSON)
     list(APPEND UT_DEPENS -lmqtt)
     list(APPEND UT_DEPENS -lezxml)
     list(APPEND UT_DEPENS -lmbedtls)
     list(APPEND UT_DEPENS -lez_iot_core)

     list(APPEND UT_DEPENS -lwebclient)
     list(APPEND UT_DEPENS -lez_iot_bm)
     list(APPEND UT_DEPENS -lezlog)
     list(APPEND UT_DEPENS -lcJSON)
 endif()

if(CONFIG_EZIOT_MODEL_ENABLE)

    message(STATUS "CONFIG_EZIOT_MODEL_ENABLE")

    list(APPEND UT_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_bm/model/inc)
    list(APPEND UT_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_core/inc)
    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/cJSON/inc)
    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/ezlog/inc)

    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_model.c)

    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ez_iot_bm)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezlist)

    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezos)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezlog)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/cJSON)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/mqtt)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezxml)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/mbedtls)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ez_iot_core)

    list(APPEND UT_DEPENS -lezos)
    list(APPEND UT_DEPENS -lezlog)
    list(APPEND UT_DEPENS -lcJSON)
    list(APPEND UT_DEPENS -lmqtt)
    list(APPEND UT_DEPENS -lezxml)
    list(APPEND UT_DEPENS -lmbedtls)
    list(APPEND UT_DEPENS -lez_iot_core)

    list(APPEND UT_DEPENS -lezlist)
    list(APPEND UT_DEPENS -lez_iot_bm)
    list(APPEND UT_DEPENS -lezlog)
    list(APPEND UT_DEPENS -lcJSON)
endif()

if(CONFIG_EZIOT_TSL_ENABLE)

    message(STATUS "CONFIG_EZIOT_TSL_ENABLE")

    list(APPEND UT_INC_DIRS ${ENV_EZIOT_PATH}/ez_iot_bm/tsl/inc)

    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_tsl.c)

    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/webclient)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezutil)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezlist)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ez_iot_bm)

    list(APPEND UT_DEPENS -lwebclient)
    list(APPEND UT_DEPENS -lezutil)
    list(APPEND UT_DEPENS -lezlist)
    list(APPEND UT_DEPENS -lez_iot_bm)

endif()

IF(CONFIG_EZIOT_COMPONENT_EZCONN_ENABLE)
	message(STATUS "CONFIG_EZIOT_COMPONENT_EZCONN_ENABLE")

    list(APPEND UT_INC_DIRS ${ENV_COMPONENT_PATH}/ezconn/inc)
    
    list(APPEND UT_SRCS ${ENV_UTCASE_PATH}/ut_ap.c)

    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezconn)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/http_server)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/eztimer)
    list(APPEND UT_DEPEN_DIRS ${ENV_LIB_PATH}/ezlog)

    list(APPEND UT_DEPENS -lezconn)
    list(APPEND UT_DEPENS -lhttp_server)
    list(APPEND UT_DEPENS -leztimer)
    list(APPEND UT_DEPENS -lezlog)
    
ENDIF()


##############################################根据开关定义编译的文件和链接的库#############################################

#预编译宏
ADD_DEFINITIONS(${UT_DEFINITIONS})

#头文件搜索路径
INCLUDE_DIRECTORIES(${UT_INC_DIRS})

#依赖库目录
LINK_DIRECTORIES(${UT_DEPEN_DIRS})

#源文件
ADD_EXECUTABLE(ut_iotsdk ${UT_SRCS})

list(REVERSE UT_DEPEN_DIRS)
# list(REMOVE_DUPLICATES UT_DEPEN_DIRS)
message(STATUS "${UT_DEPEN_DIRS}")
list(REVERSE UT_DEPENS)
list(REMOVE_DUPLICATES UT_DEPENS)
message(STATUS "${UT_DEPENS}")

# #链接依赖库
target_link_libraries(ut_iotsdk PUBLIC "-Wl,--whole-archive ${UT_DEPENS} -Wl,--no-whole-archive")
# target_link_libraries(ut_iotsdk PUBLIC "-Wl,--whole-archive,--start-group ${UT_DEPENS} -Wl,--end-group,--no-whole-archive")
