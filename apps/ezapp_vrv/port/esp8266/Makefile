# /*******************************************************************************
#  * Copyright © 2017-2021 Ezviz Inc.
#  *
#  * All rights reserved. This program and the accompanying materials
#  * are made available under the terms of the Eclipse Public License v1.0
#  * and Eclipse Distribution License v1.0 which accompany this distribution.
#  *
#  * The Eclipse Public License is available at
#  *    http://www.eclipse.org/legal/epl-v10.html
#  * and the Eclipse Distribution License is available at
#  *   http://www.eclipse.org/org/documents/edl-v10.php.
#  * 
#  * Contributors:
#  * XuRongjun (xurongjun@ezvizlife.com) -- 适配esp idf编译框架
#  *
#  * Change Logs:
#  * Date           Author       Notes
#  * 2022-03-08     xurongjun    first version 
#  *******************************************************************************

export IDF_PATH=/opt/esp_sdk/ESP8266_RTOS_SDK

# build ez_iot_os
MKDIR:=$(shell mkdir libs)
DOCMAKE:=$(shell cmake -S ./ -B libs)
DOMAKE:=$(shell make -C libs)

# build esp idf
PROJECT_NAME := ezapp_bulb
include $(IDF_PATH)/make/project.mk