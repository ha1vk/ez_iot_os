#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

LIBS := -lezconn -lhttp_server -lezos -lezlog -leztimer -lezlist -lcJSON
LIBS_DIR = $(COMPONENT_PATH)/../../../../platform/bsp/esp32/lib
#$(COMPONENT_PATH)/../../../../platform/bsp/esp32/build/ezconn
#../../../../platform/bsp/esp32/build/ezlist ../../../../platform/bsp/esp32/build/ezlog ../../../../platform/bsp/esp32/build/ezos ../../../../platform/bsp/esp32/build/eztimer ../../../../platform/bsp/esp32/build/http_server

INCLUDE_DIR = ../../../../components/ezconn/inc \
../../../../platform/inc \
../../../../components/ezlog/inc \
../../../../platform/bsp/esp32/config

#../../../../components/ezlist/inc \
#../../../../components/eztimer/inc \
#../../../../components/http_server/inc \

$($(LIBS_DIR))
#$(error $(INCLUDE_DIR))

COMPONENT_ADD_LDFLAGS += -L$(LIBS_DIR) ${LIBS} -mlongcalls
COMPONENT_PRIV_INCLUDEDIRS := $(INCLUDE_DIR)
