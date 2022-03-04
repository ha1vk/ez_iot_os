#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

export IDF_PATH=/opt/esp_sdk/ESP8266_RTOS_SDK

# build ez_iot_os
MKDIR:=$(shell mkdir libs)
DOCMAKE:=$(shell cmake -S ./ -B libs)
DOMAKE:=$(shell make -C libs)

# build esp
PROJECT_NAME := ezapp_bulb
include $(IDF_PATH)/make/project.mk