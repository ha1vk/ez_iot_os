# /*******************************************************************************
#  * Copyright ? 2017-2021 Ezviz Inc.
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
#  * XuRongjun (xurongjun@ezvizlife.com) - Compile esp8266 application with ezos
#  *******************************************************************************/

PWD := $(shell pwd)
include ${PWD}/../../config/ezos_gconfig.mk

# 包含hal头文件目录
COMPONENT_INCLUDES += ${PWD}/../../../inc

# 添加EZOS头文件目录
COMPONENT_INCLUDES += ${PWD}/../../config
COMPONENT_INCLUDES += $(CONFIG_ADD_EZOS_INC_DIRS)

# 添加EZOS库文件目录
COMPONENT_ADD_LDFLAGS += $(CONFIG_ADD_EZOS_LIB_DIRS)
COMPONENT_ADD_LDFLAGS += $(CONFIG_ADD_EZOS_LIB_DEPENS)