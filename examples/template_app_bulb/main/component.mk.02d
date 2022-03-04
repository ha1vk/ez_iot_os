#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

LIBS =
LIBS_DIR := $(COMPONENT_PATH)/../../lib/esp8266

$(warning ${LIBS})
PWD := $(shell pwd)
ENV_PLATFORM_NAME = esp8266

ENV_EZOS_PATH = ../../..
ENV_PLATFORM_PATH = ${ENV_EZOS_PATH}/platform
ENV_LIB_PATH = $(PWD)/../../../../platform/bsp/${ENV_PLATFORM_NAME}/build
ENV_COMPONENT_PATH = ${ENV_EZOS_PATH}/components
ENV_EZIOT_PATH = ${ENV_EZOS_PATH}/eziot
ENV_CONFIG_PATH = ${ENV_EZOS_PATH}/platform/bsp/${ENV_PLATFORM_NAME}/config

BULB_INC_DIRS += ${ENV_EZIOT_PATH}/ez_iot_core/inc
BULB_INC_DIRS += ${ENV_EZIOT_PATH}/ez_iot_bm/base/inc
BULB_INC_DIRS += ${ENV_EZIOT_PATH}/ez_iot_bm/tsl/inc
BULB_INC_DIRS += ${ENV_EZIOT_PATH}/ez_iot_bm/ota/inc

BULB_INC_DIRS += ${ENV_COMPONENT_PATH}/utest/inc
BULB_INC_DIRS += ${ENV_COMPONENT_PATH}/ezlog/inc
BULB_INC_DIRS += ${ENV_COMPONENT_PATH}/eztimer/inc
BULB_INC_DIRS += ${ENV_PLATFORM_PATH}/inc
BULB_INC_DIRS += ${ENV_COMPONENT_PATH}/FlashDB/inc
BULB_INC_DIRS += ${ENV_COMPONENT_PATH}/FlashDB/port/fal/inc
BULB_INC_DIRS += ${ENV_COMPONENT_PATH}/ezconn/inc
BULB_INC_DIRS += ${ENV_CONFIG_PATH}


BULB_INC_DIRS += ../../../inc

BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/ezos
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/ez_iot_bm
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/ezlog
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/cJSON
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/mqtt
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/ezxml
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/mbedtls
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/ezconn
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/ez_iot_core

BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/FlashDB
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/webclient
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/eztimer
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/ezutil
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/ezlist
BULB_DEPEN_DIRS += -L${ENV_LIB_PATH}/ez_iot_bm



BULB_DEPENS += -lpthread
BULB_DEPENS += -lcJSON
BULB_DEPENS += -lmqtt
BULB_DEPENS += -lezxml
BULB_DEPENS += -lezos
BULB_DEPENS += -lmbedtls
BULB_DEPENS += -lezconn
BULB_DEPENS += -lez_iot_core 

BULB_DEPENS += -lezlog
BULB_DEPENS += -lFlashDB

BULB_DEPENS += -lwebclient
BULB_DEPENS += -leztimer
BULB_DEPENS += -lezutil
BULB_DEPENS += -lezlist
BULB_DEPENS += -lez_iot_bm

COMPONENT_PRIV_INCLUDEDIRS := $(BULB_INC_DIRS)



COMPONENT_ADD_LDFLAGS +=  -Wl,--whole-archive,--start-group  -L$(COMPONENT_PATH)/lib $(BULB_DEPEN_DIRS) $(BULB_DEPENS) -L$(LIBS_DIR) ${LIBS} -mlongcalls -Wl,--end-group,-no-whole-archive -Wl,--gc-sections

COMPONENT_ADD_LDFLAGS +=  $(BULB_DEPEN_DIRS) $(BULB_DEPENS)
COMPONENT_SRCDIRS += kv_ts_db/esp8266 
COMPONENT_SRCDIRS += config
COMPONENT_SRCDIRS += lan_net_ctrl

COMPONENT_PRIV_INCLUDEDIRS += ./kv_ts_db/esp8266
COMPONENT_PRIV_INCLUDEDIRS += ./bulb/inc
COMPONENT_PRIV_INCLUDEDIRS += ./lan_net_ctrl

COMPONENT_SRCDIRS += bulb



