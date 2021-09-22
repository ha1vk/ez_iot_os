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

if(DEFINED PRECOMPILE_MACRO)
	ADD_DEFINITIONS("${PRECOMPILE_MACRO}")
endif()

IF (${RTOS})
	ADD_DEFINITIONS("-DSYS_RTOS")
	INCLUDE(${PROJECT_SOURCE_DIR}/../../../../private_CMakeLists.txt)
ENDIF (${RTOS})

IF (${P2PV3})
	ADD_DEFINITIONS("-DP2PV3_SUPPORT")
ENDIF (${P2PV3})



AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/http/webclient  src_webclient)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/json  src_json)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/logger  src_logger)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/mbedtls  src_mbedtls)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/utility/double_link_list  src_double_link_list)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/xml  src_xml)
	
#头文件搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
					${PROJECT_SOURCE_DIR}/http
					${PROJECT_SOURCE_DIR}/json
					${PROJECT_SOURCE_DIR}/logger
					${PROJECT_SOURCE_DIR}/mbedtls
					${PROJECT_SOURCE_DIR}/utility
					${PROJECT_SOURCE_DIR}/xml
					${PROJECT_SOURCE_DIR}/../../include
					${PROJECT_SOURCE_DIR}/../../platform/inc)

#同时生成静态库和动态库，target的名字要唯一，但是生成的库名可以重定义
ADD_LIBRARY(ez_iot_components_static STATIC ${src_webclient} ${src_json} ${src_logger} ${src_mbedtls} ${src_double_link_list} ${src_xml})
SET_TARGET_PROPERTIES(ez_iot_components_static PROPERTIES OUTPUT_NAME ez_iot_components)

IF (${DYNAMIC})
ADD_LIBRARY(ez_iot_components_shared SHARED ${src_webclient} ${src_json} ${src_logger} ${src_mbedtls} ${src_double_link_list} ${src_xml})

SET_TARGET_PROPERTIES(ez_iot_components_shared PROPERTIES OUTPUT_NAME ez_iot_components)
ENDIF (${DYNAMIC})

#设置生成的库文件存放路径为lib
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib)
