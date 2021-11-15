

file(GLOB business_module_dirs *)

foreach(module_path ${business_module_dirs})
    if(IS_DIRECTORY ${module_path})
        get_filename_component(module_name ${module_path} NAME)

        message(STATUS "business module name:${module_name}")
        include(${module_path}/CMakeLists.txt)
    endif()
endforeach()

register_component()