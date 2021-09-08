#cmake版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

#项目名称
PROJECT(DEV_MODEL)
SET(USE_STATIC_LIB_LINKAGE ON)

#添加编译选项
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DSDK_HARDWARE_PLATFORM=\"${HARDWARE_PLATFORM}\"")
ADD_DEFINITIONS("-DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS")

if(DEFINED PRECOMPILE_MACRO)
	ADD_DEFINITIONS("${PRECOMPILE_MACRO}")
endif()

if(DEFINED PLATFORM_C_FLAGS)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PLATFORM_C_FLAGS}")
else()
	SET(CMAKE_C_FLAGS "-O2 -g  -Wall")
endif()	

if(DEFINED PLATFORM_CXX_FLAGS)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PLATFORM_CXX_FLAGS}")
else()
	SET(CMAKE_CXX_FLAGS "-O2 -g -Wall -fno-rtti -fno-exceptions ")
endif()

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/  ez_model)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/
                    ${PROJECT_SOURCE_DIR}/../inc
					${PROJECT_SOURCE_DIR}/../../../../components/json
					${PROJECT_SOURCE_DIR}/../../../../components/utility/double_link_list
					${PROJECT_SOURCE_DIR}/../../../../components/logger/inc
                    ${PROJECT_SOURCE_DIR}/../../../../include
					${PROJECT_SOURCE_DIR}/../../../core/inc)

#同时生成静态库和动态库，target的名字要唯一，但是生成的库名可以重定义
ADD_LIBRARY(ez_model_static STATIC ${ez_model})
SET_TARGET_PROPERTIES(ez_model_static PROPERTIES OUTPUT_NAME ez_iot_model)

IF (${DYNAMIC})
ADD_LIBRARY(ez_model_shared SHARED ${ez_model})
SET_TARGET_PROPERTIES(ez_model_shared PROPERTIES OUTPUT_NAME ez_iot_model)
ENDIF (${DYNAMIC})

#设置生成的库文件存放路径为lib
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../lib/linux)

