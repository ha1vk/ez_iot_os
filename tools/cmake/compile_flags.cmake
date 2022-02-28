
# @author xurongjun (xurongjun@ezvizlife.com)
# @brief
# @version 0.1
# @date 2021-09-29

if(CONFIG_ADD_PRIVATE_CFLAGS)
    set(CMAKE_C_FLAGS ${CONFIG_ADD_PRIVATE_CFLAGS})
else()
    set(CMAKE_C_FLAGS "-g -Wall")
endif()

if(CONFIG_ADD_PRIVATE_CXXFLAGS)
    set(CMAKE_CXX_FLAGS ${CONFIG_ADD_PRIVATE_CXXFLAGS})
else()
    set(CMAKE_CXX_FLAGS "-g -Wall")
endif()

if(CONFIG_ADD_PRIVATE_LINK_CFLAGS)
    set(CMAKE_C_LINK_FLAGS ${CMAKE_C_LINK_FLAGS}
                            ${CONFIG_ADD_PRIVATE_LINK_CFLAGS})
    string(REPLACE ";" " " CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS}")
endif()

if(CONFIG_ADD_PRIVATE_LINK_CXXFLAGS)
    set(CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS}
                            ${CONFIG_ADD_PRIVATE_LINK_CXXFLAGS})
    string(REPLACE ";" " " CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS}")
endif()