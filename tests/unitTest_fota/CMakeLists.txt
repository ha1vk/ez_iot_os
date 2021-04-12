#cmake版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")
SET(CMAKE_C_COMPILER, TOOCHAIN_C_COMPILER)
SET(CMAKE_CXX_COMPILER, TOOCHAIN_CXX_COMPILER)

#项目名称
PROJECT(DEV_EZPATCH_UNITTEST)
SET(USE_STATIC_LIB_LINKAGE ON)

#添加编译选项
ADD_DEFINITIONS("-Wall")
ADD_DEFINITIONS("-DNETBSD -D_LARGEFILE_SOURCE -D_LARGE_FILES -DFTP_COMBINE_CWDS")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Wno-unused-variable ")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wno-unused-variable ")
else()
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wno-unused-variable")
endif(CMAKE_BUILD_TYPE STREQUAL "Debug")

##############################################根据开关定义编译的文件和链接的库#############################################
#SET(src_unittest_comm unittest_entry.cc localcfg.cc utils.c  ./third_party/json/cJSON.c ../../components//mbedtls/md5.c )

if(UNITTEST_EZPATCH_ENABLE)
    message(STATUS "UNITTEST_EZPATCH_ENABLE")
    SET(unittest_ezpatch unittest_ezpatch.cc)
endif()

##############################################根据开关定义编译的文件和链接的库#############################################

#########################################################添加依赖库######################################################
SET(lib_rt -lpthread -lm -lrt)
SET(lib_gtest -lgtest)
SET(lib_ezbspatch -lezbspatch -lbz2)
#########################################################添加依赖库######################################################

#SET(src_files  ${unittest_ezpatch} ${src_unittest_comm})

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/../third_party_lib/json  json)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/../third_party_lib/md5  md5)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/  src)

#头文件搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../third_party_lib/googletest/include
                    ${PROJECT_SOURCE_DIR}/../third_party_lib/googletest/include/gtest
                    ${PROJECT_SOURCE_DIR}/../inc
                    ${PROJECT_SOURCE_DIR}/../../eziot/extensions/ota/ota_diff_patch/inc
                    ${PROJECT_SOURCE_DIR}/../third_party_lib/json
                    ${PROJECT_SOURCE_DIR}/../third_party_lib/md5

                    )

#添加依赖库目录
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../lib
                 ${PROJECT_SOURCE_DIR}/../third_party_lib/googletest/lib)

#添加需要编译的源文件
ADD_EXECUTABLE(testunit_ez_bspatch  ${md5} ${json} ${src})

#添加链接库
target_link_libraries(testunit_ez_bspatch
                        ${lib_gtest}
                        ${lib_ezbspatch}
                        ${lib_rt}
                        )