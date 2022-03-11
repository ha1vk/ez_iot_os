if(CONFIG_EZIOT_COMPONENT_FLASHDB_ENABLE)

    ################# Add include #################
    list(APPEND ADD_INCLUDE "inc"
        )
    # list(APPEND ADD_PRIVATE_INCLUDE "inc_private")
    if(CONFIG_EZIOT_COMPONENT_FLASHDB_USING_FAL_MODE)
    list(APPEND ADD_INCLUDE  "port/fal/inc")
    endif()

    ###############################################

    ############## Add source files ###############
    list(APPEND ADD_SRCS  "src/fdb.c"
                          "src/fdb_utils.c")

    if(CONFIG_EZIOT_COMPONENT_FLASHDB_USING_KVDB)
    list(APPEND ADD_SRCS  "src/fdb_kvdb.c")
    endif()

    if(CONFIG_EZIOT_COMPONENT_FLASHDB_USING_TSDB)
    list(APPEND ADD_SRCS  "src/fdb_tsdb.c")
    endif()

    if(CONFIG_EZIOT_COMPONENT_FLASHDB_USING_FAL_MODE)
    list(APPEND ADD_SRCS  "port/fal/src/fal_flash.c")
    list(APPEND ADD_SRCS  "port/fal/src/fal_partition.c")
    list(APPEND ADD_SRCS  "port/fal/src/fal_rtt.c")
    list(APPEND ADD_SRCS  "port/fal/src/fal.c")
    endif()

    if(CONFIG_EZIOT_COMPONENT_FLASHDB_USING_FILE_LIBC_MODE OR CONFIG_EZIOT_COMPONENT_FLASHDB_USING_FILE_POSIX_MODE)
    list(APPEND ADD_SRCS  "src/fdb_file.c")
    endif()

    # aux_source_directory(src ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
    # append_srcs_dir(ADD_SRCS "src")     # append source file in src dir to var ADD_SRCS
    # list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
    ###############################################

    ###### Add required/dependent components ######
    # list(APPEND ADD_REQUIREMENTS component1)
    list(APPEND ADD_REQUIREMENTS ezos)
    if(CONFIG_EZIOT_COMPONENT_FLASHDB_USING_FAL_MODE)
    list(APPEND ADD_REQUIREMENTS main)  # fal mode needs to include fal_cfg.h files when compiling, main is defalut application component
    endif()
    ###############################################

    ############ Add static libs ##################
    # if(CONFIG_COMPONENT1_INCLUDE_STATIC_LIB)
    #     list(APPEND ADD_STATIC_LIB "lib/libtest.a")
    # endif()
    ###############################################

    ############ Add dynamic libs ##################
    # list(APPEND ADD_DYNAMIC_LIB "lib/arch/v831/libmaix_nn.so"
    #                             "lib/arch/v831/libmaix_cam.so"
    # )
    ###############################################

    #### Add compile option for this component ####
    #### Just for this component, won't affect other 
    #### modules, including component that depend 
    #### on this component
    # list(APPEND ADD_DEFINITIONS_PRIVATE -DAAAAA=1)

    #### Add compile option for this component
    #### and components denpend on this component
    # list(APPEND ADD_DEFINITIONS -DAAAAA222=1 -DAAAAA333=1)
    ###############################################

    ############ Add static libs ##################
    #### Update parent's variables like CMAKE_C_LINK_FLAGS
    # set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group libmaix/libtest.a -ltest2 -Wl,--end-group" PARENT_SCOPE)
    ###############################################

    # register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
    register_component()

endif()