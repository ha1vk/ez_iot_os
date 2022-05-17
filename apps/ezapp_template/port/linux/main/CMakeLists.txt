################# Add include #################
set(PRIVATE_SRC_ROOT "../../..")

list(APPEND ADD_INCLUDE "${PRIVATE_SRC_ROOT}/bussiness/inc")
list(APPEND ADD_INCLUDE "${PRIVATE_SRC_ROOT}/ezcloud/inc")
list(APPEND ADD_INCLUDE "${PRIVATE_SRC_ROOT}/network/inc")
list(APPEND ADD_INCLUDE "${PRIVATE_SRC_ROOT}/port/inc")
list(APPEND ADD_INCLUDE "${PRIVATE_SRC_ROOT}/product/inc")
###############################################

############## Add source files ###############
# list(APPEND ADD_SRCS  "src/lib1.c"
#     )
# aux_source_directory(./ ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
aux_source_directory(./ ADD_SRCS)
aux_source_directory(${PRIVATE_SRC_ROOT}/bussiness ADD_SRCS)
aux_source_directory(${PRIVATE_SRC_ROOT}/ezcloud ADD_SRCS)
aux_source_directory(${PRIVATE_SRC_ROOT}/network ADD_SRCS)
aux_source_directory(${PRIVATE_SRC_ROOT}/port ADD_SRCS)
aux_source_directory(${PRIVATE_SRC_ROOT}/product ADD_SRCS)
# append_srcs_dir(ADD_SRCS "src")     # append source file in src dir to var ADD_SRCS
# list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
###############################################

###### Add required/dependent components ######
# list(APPEND ADD_REQUIREMENTS component1)
list(APPEND ADD_REQUIREMENTS cli)
list(APPEND ADD_REQUIREMENTS FlashDB)
list(APPEND ADD_REQUIREMENTS cJSON)
list(APPEND ADD_REQUIREMENTS ezconn)
list(APPEND ADD_REQUIREMENTS ezlist)
list(APPEND ADD_REQUIREMENTS ezlog)
list(APPEND ADD_REQUIREMENTS eztimer)
list(APPEND ADD_REQUIREMENTS ezutil)
list(APPEND ADD_REQUIREMENTS ezxml)
list(APPEND ADD_REQUIREMENTS http_server)
list(APPEND ADD_REQUIREMENTS mbedtls)
list(APPEND ADD_REQUIREMENTS mqtt)
list(APPEND ADD_REQUIREMENTS webclient)
list(APPEND ADD_REQUIREMENTS ez_iot_bm)
list(APPEND ADD_REQUIREMENTS ez_iot_core)
list(APPEND ADD_REQUIREMENTS ezos)
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
# set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group libmaix/libtest.a test2 -Wl,--end-group" PARENT_SCOPE)
###############################################

# register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
register_component()