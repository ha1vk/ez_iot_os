#cmake版本要求
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_TOOLCHAIN_FILE "ToolChain.cmake")

#项目名称
PROJECT(EZXML_TEST)
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

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/  test_xml)
AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/../../components/xml  ez_xml)

SET(lib_rt -lm -lrt)

#头文件搜索路径
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/
                    ${PROJECT_SOURCE_DIR}/../../components/xml)

#添加需要编译的源文件
ADD_EXECUTABLE(test_xml_bin ${test_xml} ${ez_xml})

#添加链接库
target_link_libraries(test_xml_bin
                     ${lib_rt})

SET_TARGET_PROPERTIES(test_xml_bin PROPERTIES OUTPUT_NAME helloxml)