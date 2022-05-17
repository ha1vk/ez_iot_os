# @author xurongjun (xurongjun@ezvizlife.com)
# @brief EZOS osal implement building
# @version 0.1
# @date 2021-09-29

############### Add include ###################

if(CONFIG_EZIOT_OS_ENABLE)

    list(APPEND ADD_INCLUDE "../../../inc")

    if(IS_DIRECTORY ${CONFIG_EZIOT_OS_BSP_ROOT})
        # 使用Kconfig配置路径
        set(OS_BSP_ROOT ${CONFIG_EZIOT_OS_BSP_ROOT})
    else()
        # 使用相对路径、绝对路径或者环境变量
        set(OS_BSP_ROOT $ENV{IDF_PATH})
    endif()

    message(STATUS "BSP ROOT: ${OS_BSP_ROOT}")

    if(IS_DIRECTORY ${OS_BSP_ROOT})
        list(APPEND ADD_PRIVATE_INCLUDE
            ../inc_private
            ${OS_BSP_ROOT}/components/freertos/include 
            ${OS_BSP_ROOT}/components/freertos/include/freertos 
            ${OS_BSP_ROOT}/components/freertos/include/freertos/private 
            ${OS_BSP_ROOT}/components/freertos/port/esp8266/include 
            ${OS_BSP_ROOT}/components/freertos/port/esp8266/include/freertos 
            ${OS_BSP_ROOT}/components/esp8266/include 
            ${OS_BSP_ROOT}/components/heap/include 
            ${OS_BSP_ROOT}/components/heap/port/esp8266/include 
            ${OS_BSP_ROOT}/components/nvs_flash/test_nvs_host 
            ${OS_BSP_ROOT}/components/lwip/include/lwip
            ${OS_BSP_ROOT}/components/lwip/include/lwip/apps
            ${OS_BSP_ROOT}/components/lwip/lwip/src/include
            ${OS_BSP_ROOT}/components/lwip/lwip/src/include/posix
            ${OS_BSP_ROOT}/components/lwip/port/esp8266/include
            ${OS_BSP_ROOT}/components/lwip/port/esp8266/include/port

            ${OS_BSP_ROOT}/components/freertos/include 
            ${OS_BSP_ROOT}/components/freertos/port/esp8266/include/freertos 
            ${OS_BSP_ROOT}/components/freertos/port/esp8266/include 
            ${OS_BSP_ROOT}/components/esp8266/include 
            ${OS_BSP_ROOT}/components/heap/include 
            ${OS_BSP_ROOT}/components/heap/port/esp8266/include
            ${OS_BSP_ROOT}/components/freertos/include/freertos/private
            ${OS_BSP_ROOT}/components/http_parser/include 
            ${OS_BSP_ROOT}/components/cjson/cJSON
            ${OS_BSP_ROOT}/components/lwip/lwip/src/include
            ${OS_BSP_ROOT}/components/lwip/port/esp8266/include 
            ${OS_BSP_ROOT}/components/lwip/include/lwip/apps 
            ${OS_BSP_ROOT}/components/tcpip_adapter/include
            ${OS_BSP_ROOT}/components/log/include

            ${OS_BSP_ROOT}/components/app_update/include
            ${OS_BSP_ROOT}/components/nvs_flash/include
            ${OS_BSP_ROOT}/components/spi_flash/include
            ${OS_BSP_ROOT}/components/vfs/include
        )
    endif()
    ###############################################

    ############ Add source files #################
    # list(APPEND ADD_SRCS  "src/main.c"
    #                       "src/test.c"
    #     )
    aux_source_directory(. ADD_SRCS)       # collect all source file in src dir, will set var ADD_SRCS
    #append_srcs_dir(ADD_SRCS "src")        # append source file in src dir to var ADD_SRCS
    # list(REMOVE_ITEM COMPONENT_SRCS "src/test2.c")
    ###############################################

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
    list(APPEND ADD_DEFINITIONS  "-D__ESP_FILE__=__FILE__")
    ###############################################

    ############ Add static libs ##################
    #### Update parent's variables like CMAKE_C_LINK_FLAGS
    #set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS}" )
    ###############################################

    register_component()

endif()