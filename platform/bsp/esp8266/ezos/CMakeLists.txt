# @author xurongjun (xurongjun@ezvizlife.com)
# @brief EZOS osal implement building
# @version 0.1
# @date 2021-09-29

############### Add include ###################
list(APPEND ADD_INCLUDE "../../../inc")
set(IDF_PATH "/opt/esp_sdk/ESP8266_RTOS_SDK")
#export IDF_PATH=/opt/esp_sdk/esp-idf
list(APPEND ADD_PRIVATE_INCLUDE
    ../inc_private
    ${IDF_PATH}/components/freertos/include 
    ${IDF_PATH}/components/freertos/include/freertos 
    ${IDF_PATH}/components/freertos/include/freertos/private 
    ${IDF_PATH}/components/freertos/port/esp8266/include 
    ${IDF_PATH}/components/freertos/port/esp8266/include/freertos 
    ${IDF_PATH}/components/esp8266/include 
    ${IDF_PATH}/components/heap/include 
    ${IDF_PATH}/components/heap/port/esp8266/include 
    ${IDF_PATH}/components/nvs_flash/test_nvs_host 
    ${IDF_PATH}/components/lwip/include/lwip
    ${IDF_PATH}/components/lwip/include/lwip/apps
    ${IDF_PATH}/components/lwip/lwip/src/include
    ${IDF_PATH}/components/lwip/lwip/src/include/posix
    ${IDF_PATH}/components/lwip/port/esp8266/include
    ${IDF_PATH}/components/lwip/port/esp8266/include/port
        
    # ap模块
    #${IDF_PATH}/components/esp_http_server/include 
    ${IDF_PATH}/components/freertos/include 
    ${IDF_PATH}/components/freertos/port/esp8266/include/freertos 
    ${IDF_PATH}/components/freertos/port/esp8266/include 
    ${IDF_PATH}/components/esp8266/include 
    ${IDF_PATH}/components/heap/include 
    ${IDF_PATH}/components/heap/port/esp8266/include
    ${IDF_PATH}/components/freertos/include/freertos/private
    ${IDF_PATH}/components/http_parser/include 
    ${IDF_PATH}/components/cjson/cJSON
    ${IDF_PATH}/components/lwip/lwip/src/include
    ${IDF_PATH}/components/lwip/port/esp8266/include 
    ${IDF_PATH}/components/lwip/include/lwip/apps 
    ${IDF_PATH}/components/tcpip_adapter/include
    #${IDF_PATH}/components/newlib/include/
    #校时模块
    ${IDF_PATH}/components/log/include

    #升级模块
    ${IDF_PATH}/components/app_update/include
    ${IDF_PATH}/components/nvs_flash/include
    ${IDF_PATH}/components/spi_flash/include
    #${IDF_PATH}/components/esp_http_client/include
    ${IDF_PATH}/components/vfs/include

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
list(APPEND ADD_DEFINITIONS -D_POSIX_TIMERS -D_POSIX_MONOTONIC_CLOCK )
list(APPEND ADD_DEFINITIONS  "-D__ESP_FILE__=__FILE__")
###############################################

############ Add static libs ##################
#### Update parent's variables like CMAKE_C_LINK_FLAGS
#set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS}" )
###############################################

register_component()
