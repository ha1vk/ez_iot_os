#cmake版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_C_COMPILER "gcc")
SET(CMAKE_CXX_COMPILER "g++")

#配置编译选项,"Debug" || "Release"
SET(CMAKE_BUILD_TYPE "Debug")


#项目名称
PROJECT(BAPATCH_LIB)
SET(USE_STATIC_LIB_LINKAGE ON)

#添加编译选项
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -Wall -Wno-unused-variable")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -Wno-unused-variable")

SET(src_files
    ez_bspatch.c
)

#头文件搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../bzip2-1.0.6
                    ${PROJECT_SOURCE_DIR}/../../inc
                    )

#添加需要编译的源文件
ADD_LIBRARY(ezbspatch STATIC ${src_files})
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../../../../../lib)
