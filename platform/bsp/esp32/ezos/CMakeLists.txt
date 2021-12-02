# @author xurongjun (xurongjun@ezvizlife.com)
# @brief EZOS osal implement building
# @version 0.1
# @date 2021-09-29

############### Add include ###################
list(APPEND ADD_INCLUDE "../../../inc")
list(APPEND ADD_PRIVATE_INCLUDE
    ../inc_private
    $ENV{IDF_PATH}/components/lwip/lwip/src/include
    $ENV{IDF_PATH}/components/lwip/lwip/src/include/lwip
    $ENV{IDF_PATH}/components/lwip/port/esp32/include
    $ENV{IDF_PATH}/components/vfs/include
    $ENV{IDF_PATH}/components/esp_common/include
    $ENV{IDF_PATH}/components/xtensa/include
    $ENV{IDF_PATH}/components/xtensa/esp32/include
    $ENV{IDF_PATH}/components/esp_rom/include
    $ENV{IDF_PATH}/components/lwip/include/apps/sntp
    $ENV{IDF_PATH}/components/soc/esp32/include/

    $ENV{IDF_PATH}/components/heap/include/
    $ENV{IDF_PATH}/components/newlib/include/
    $ENV{IDF_PATH}/components/newlib/platform_include/
    $ENV{IDF_PATH}/components/freertos/include/freertos
    $ENV{IDF_PATH}/components/esp_wifi/include
    $ENV{IDF_PATH}/components/esp_event/include 
    $ENV{IDF_PATH}/components/tcpip_adapter/include
    $ENV{IDF_PATH}/components/lwip/include/apps
    $ENV{IDF_PATH}/components/driver/include
    $ENV{IDF_PATH}/components/esp_ringbuf/include
    $ENV{IDF_PATH}/components/soc/include


    $ENV{IDF_PATH}/components/freertos/include 
    $ENV{IDF_PATH}/components/esp32/include 
)
###############################################

############ Add source files #################
# list(APPEND ADD_SRCS  "src/main.c"
#                       "src/test.c"
#     )
aux_source_directory(. ADD_SRCS)       # collect all source file in src dir, will set var ADD_SRCS
#append_srcs_dir(ADD_SRCS "src")        # append source file in src dir to var ADD_SRCS
# list(REMOVE_ITEM COMPONENT_SRCS "src/test2.c")
###############################################
list(APPEND ADD_REQUIREMENTS ezlog)

###### Add required/dependent components ######
#list(APPEND ADD_REQUIREMENTS core)
#if(CONFIG_COMPONENT2_ENABLED)
#    list(APPEND ADD_REQUIREMENTS component2)
#endif()
#if(CONFIG_COMPONENT3_ENABLED)
#    list(APPEND ADD_REQUIREMENTS component3)
#endif()
###############################################

############ Add static libs ##################
# list(APPEND ADD_STATIC_LIB "lib/libtest.a")
###############################################

#### Add compile option for this component ####
#### Just for this component, won't affect other 
#### modules, including component that depend 
#### on this component
# list(APPEND ADD_DEFINITIONS_PRIVATE -DAAAAA=1)

#### Add compile option for this component
#### and components denpend on this component
list(APPEND ADD_DEFINITIONS -D_POSIX_TIMERS -D_POSIX_MONOTONIC_CLOCK)
###############################################

############ Add static libs ##################
#### Update parent's variables like CMAKE_C_LINK_FLAGS
# set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group libmaix/libtest.a -ltest2 -Wl,--end-group" PARENT_SCOPE)
###############################################

register_component()