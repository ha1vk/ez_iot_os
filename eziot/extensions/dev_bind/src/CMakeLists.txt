#cmake版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

#项目名称
PROJECT(DEV_SDK)
SET(USE_STATIC_LIB_LINKAGE ON)

#添加编译选项
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DSDK_HARDWARE_PLATFORM=\"${HARDWARE_PLATFORM}\"")
ADD_DEFINITIONS("-DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS")

SET(CMAKE_C_FLAGS "-O2 -g -Wall")
SET(CMAKE_CXX_FLAGS "-O2 -g -Wall -fno-rtti -fno-exceptions")

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/  dev_bind)
	
#头文件搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/
                    ${PROJECT_SOURCE_DIR}/../inc
					${PROJECT_SOURCE_DIR}/das_protocol_def
					${PROJECT_SOURCE_DIR}/../../../../components/src/json
					${PROJECT_SOURCE_DIR}/../../../../components/src/xml
					${PROJECT_SOURCE_DIR}/../../../../components/src/logger
					${PROJECT_SOURCE_DIR}/../../../../components/src/mbedtls
                    ${PROJECT_SOURCE_DIR}/../../../../include
					${PROJECT_SOURCE_DIR}/../../../../platform/inc
					${PROJECT_SOURCE_DIR}/../../../core/inc)
#同时生成静态库和动态库，target的名字要唯一，但是生成的库名可以重定义
ADD_LIBRARY(ez_iot_base_static STATIC ${dev_bind})
SET_TARGET_PROPERTIES(ez_iot_base_static PROPERTIES OUTPUT_NAME ez_iot_base)

IF (${DYNAMIC})
ADD_LIBRARY(ez_iot_base_shared SHARED ${dev_bind})
SET_TARGET_PROPERTIES(ez_iot_base_shared PROPERTIES OUTPUT_NAME ez_iot_base)
ENDIF (${DYNAMIC})

#设置生成的库文件存放路径为lib
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib)
