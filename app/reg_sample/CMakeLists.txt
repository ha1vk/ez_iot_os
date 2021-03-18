#cmake版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

#项目名称
PROJECT(DEV_SDK_TEST)
SET(USE_STATIC_LIB_LINKAGE ON)

#添加编译选项
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -g -Wall -Wno-unused-variable")
else()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -Wno-unused-variable")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

SET(lib_ez_core  -lez_iot_core)
SET(lib_ez_link  -lez_iot_link)
SET(lib_ez_log  -lez_iot_log)
#SET(lib_ez_iot_link  -lez_iot_ota)

#可以将eziotlink单独编译成库,以独立库的形式进行加载调用,这里展示以源码编译的方式生成示例sample

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/  reg_sample)

SET(lib_rt -lpthread -lm -lrt)

#头文件搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/
                    ${PROJECT_SOURCE_DIR}/../../inc
                    ${PROJECT_SOURCE_DIR}/../../include)
#添加依赖库目录
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../lib/gcc)

#添加需要编译的源文件
ADD_EXECUTABLE(reg_sample_bin ${reg_sample})

#添加链接库
target_link_libraries(reg_sample_bin
                        ${lib_ez_core}
                        ${lib_ez_link}
                        ${lib_ez_log}
                        ${lib_rt})

SET_TARGET_PROPERTIES(reg_sample_bin PROPERTIES OUTPUT_NAME hello_sample)