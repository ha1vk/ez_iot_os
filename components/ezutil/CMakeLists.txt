

file(GLOB ezutil_dirs *)

foreach(ezutil_path ${ezutil_dirs})
    if(IS_DIRECTORY ${ezutil_path})
        get_filename_component(ezutil_name ${ezutil_path} NAME)

        include(${ezutil_path}/CMakeLists.txt)
    endif()
endforeach()

register_component()