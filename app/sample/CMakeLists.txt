#cmake版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

#项目名称
PROJECT(SAMPLE_TEST)
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
SET(lib_ez_platform  -lez_os_platform)
SET(lib_ez_components   -lez_iot_components)
SET(lib_rt -lpthread -lm -lrt)


if(TEST_BASE_ENABLE)
    message(STATUS "TEST_BASE_ENABLE")
    #SET(test_base_src base_sample.c)
    AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/extensions/base  base_sample)
    SET(lib_base -lez_iot_base)
    ADD_DEFINITIONS("-DTEST_BASE_ENABLE")
endif()

if(TEST_OTA_ENABLE)
    message(STATUS "TEST_OTA_ENABLE")
    #SET(test_base_src base_sample.c)
    AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/extensions/ota  ota_sample)
    SET(lib_ota -lez_iot_ota)
    ADD_DEFINITIONS("-DTEST_OTA_ENABLE")
endif()

if(TEST_MODEL_ENABLE)
    message(STATUS "TEST_MODEL_ENABLE")
    #SET(test_base_src base_sample.c)
    AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/extensions/model  model_sample)
    SET(lib_model -lez_iot_model)
    ADD_DEFINITIONS("-DTEST_MODEL_ENABLE")
endif()

#可以将eziotlink单独编译成库,以独立库的形式进行加载调用,这里展示以源码编译的方式生成示例sample

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/  main_sample)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/utility  inireader)

#头文件搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/utility
                    ${PROJECT_SOURCE_DIR}/extensions/base
                    ${PROJECT_SOURCE_DIR}/extensions/ota
                    ${PROJECT_SOURCE_DIR}/extensions/model
                    ${PROJECT_SOURCE_DIR}/../../inc
                    ${PROJECT_SOURCE_DIR}/../../include
                    ${PROJECT_SOURCE_DIR}/../../components/src/json
                    ${PROJECT_SOURCE_DIR}/../../components/src/logger
                    ${PROJECT_SOURCE_DIR}/../../components/src)
#添加依赖库目录
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../lib/gcc)

#添加需要编译的源文件
ADD_EXECUTABLE(main_sample_bin ${main_sample} ${inireader} ${base_sample} ${ota_sample} ${model_sample})

#添加链接库
target_link_libraries(main_sample_bin
                        ${lib_model}
                        ${lib_ota}
                        ${lib_base}
                        ${lib_ez_core}
                        ${lib_rt}
                        ${lib_ez_components}
                        ${lib_ez_platform}
                        )

SET_TARGET_PROPERTIES(main_sample_bin PROPERTIES OUTPUT_NAME hellosample)