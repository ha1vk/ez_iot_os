if(CONFIG_EZIOT_COMPONENT_EZUTIL_S2J_ENABLE)

    ################# Add include #################
    list(APPEND ADD_INCLUDE "${ezutil_name}/inc"
        )
    ###############################################

    ############## Add source files ###############
    # list(APPEND ADD_SRCS  "${ezutil_name}/src/lib1.c"
        # )
    # aux_source_directory(${ezutil_name}/src ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
    # append_srcs_dir(ADD_SRCS "${ezutil_name}/src")     # append source file in src dir to var ADD_SRCS
    append_srcs_dir(ADD_SRCS "${ezutil_name}/src")     # append source file in src dir to var ADD_SRCS
    # list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
    ###############################################

    ###### Add required/dependent components ######
    list(APPEND ADD_REQUIREMENTS ezos)
    list(APPEND ADD_REQUIREMENTS cJSON)
    ###############################################

    ############ Add static libs ##################
    # if(CONFIG_COMPONENT1_INCLUDE_STATIC_LIB)
    #     list(APPEND ADD_STATIC_LIB "${ezutil_name}/lib/libtest.a")
    # endif()
    ###############################################

    ############ Add dynamic libs ##################
    # list(APPEND ADD_DYNAMIC_LIB "${ezutil_name}/lib/arch/v831/libmaix_nn.so"
    #                             "${ezutil_name}/lib/arch/v831/libmaix_cam.so"
    # )
    ###############################################

    #### Add compile option for this component ####
    #### Just for this component, won't affect other 
    #### modules, including component that depend 
    #### on this component
    # list(APPEND ADD_DEFINITIONS_PRIVATE -DAAAAA=1)

    #### Add compile option for this component
    #### and components denpend on this component
    # list(APPEND ADD_DEFINITIONS -DAAAAA222=1
    #                             -DAAAAA333=1)
    ###############################################

    ############ Add static libs ##################
    #### Update parent's variables like CMAKE_C_LINK_FLAGS
    # set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group ${ezutil_name}/libmaix/libtest.a -ltest2 -Wl,--end-group" PARENT_SCOPE)
    ###############################################

endif()