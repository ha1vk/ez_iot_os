#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

PWD := $(shell pwd)
include ${PWD}/../../config/.config

$(warning $(CONFIG_TOOLCHAIN_PATH),$(CONFIG_ADD_PRIVATE_CFLAGS))  