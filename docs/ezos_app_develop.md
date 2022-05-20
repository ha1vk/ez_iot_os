# 应用开发指南

​        应用软件主要侧重于产品功能的实现，在开发方法上和组件近似。本文档将详细介绍应用的构成、规范及开发流程，帮助应用开发人员进行应用开发。

## 目录结构

```shell
ez-iot-os
|
+-- apps
|   +-- ezapp_template      // 应用模板
|   +-- ezapp_bulb          // 灯泡应用
|   +-- ezapp_other         // 其他应用
|
```

## 应用模板

### 目录模板

```
+-- ezapp_template
    +-- bussiness
    |   +-- inc
    |   +-- ezcloud_tsl_protocol.c     // 物模型协议实现
    |   
    +-- ezcloud                        // 云端基础功能接入
    |   +-- inc                        // 头文件
    |   +-- ezcloud_base.c             // 基础功能（绑定、解绑、校时等）
    |   +-- ezcloud_link.c             // 设备认证、连接
    |   +-- ezcloud_ota.c              // 升级
    |   +-- ezcloud_tsl.c              // 物模型通道
    |
    +-- network
    |   +-- inc
    |   +-- network.c                  // 网络功能（配网、wifi连接等）
    |
    +-- port                           // 芯片相关实现
    |   +-- inc
    |   |   +-- hal_config.h           // 本地存储接口抽象
    |   |   +-- hal_ota.h              // 升级接口抽象
    |   |
    |   +-- linux                      // linux通用实现
    |   +-- other                      // 其他芯片/系统实现
    |
    +-- product
    |   +-- inc
    |   +-- device_info.c              // 设备license解析
    |
    +-- Kconfig                        // 应用配置
```



- 应用也有芯片相关的抽象接口，如：
  - 应用入口。不同芯片的应用入口会不一致，所以应用入口需要抽象实现。
  - 升级功能。同一个芯片可能分区表或者升级方式不同，所以升级的实现需要放在应用层。
  - 存储功能。同样可能因分区表或有无文件系统的影响，所以存储的实现需要放在应用层。
  - 未基线化的接口，部分接口能力暂未基线化，需要放在应用层抽象，如ntp、pwm等。
  - 构建方法。不同芯片的BSP构建方法有差异，需要做适配。

### 构建入口

```cmake
# @author xurongjun (xurongjun@ezvizlife.com)
# @brief The entrance of ezos components building
# @version 0.1
# @date 2021-09-29

cmake_minimum_required(VERSION 3.5)

####### Prepare building environment ##########

# Get SDK path
if(IS_DIRECTORY $ENV{EZOS_PATH})
    set(EZOS_PATH $ENV{EZOS_PATH})
else()
    get_filename_component(EZOS_PATH ../../../../ ABSOLUTE)
endif()

# Check SDK Path
if(NOT EXISTS ${EZOS_PATH})
    message(FATAL_ERROR "EZOS path Error, Please set EZOS_PATH variable")
endif()

include(${EZOS_PATH}/tools/cmake/compile.cmake)

# Project Name, default the same as project directory name
get_filename_component(parent_dir ${CMAKE_PARENT_LIST_FILE} DIRECTORY)
get_filename_component(project_dir_name ${parent_dir} NAME)

set(PROJECT_NAME ${project_dir_name})
set(PROJECT_KCONFIG_PATH "${parent_dir}/../..")

do_lib_building("ezapp")
###############################################
```



- EZOS_PATH：需要找到EZOS根目录，可从环境变量、相对路径或绝对路径查找。
- PROJECT_KCONFIG_PATH：应用Kconfig目录
- do_lib_building：仅编译库文件



### 构建脚本

​        参考组件开发文档，基本一致。

### 配置脚本

​        参考组件开发文档，基本一致。

## 应用开发

### 开发规范

- 为后续应用易移植性，应用层尽量避免直接调用BSP接口，应使用EZOS封装的接口。
- 无法避免调用直接调用BSP接口的（如和存储分区相关等），应抽象至port目录，使得移植工作可量化。

### 拷贝应用模板

​        拷贝EZOS应用模板，以此为基础进行应用开发。应用目录名称应能较好反映应用品类，如ezapp_bulb（灯泡），ezapp_switch（开关）等，以下已hello为例。

```shell
cp -r ~/ez_iot_os/apps/ezapp_template ~/ezapp_hello
```

### 实现设备业务

ezapp_template已经实现部分功能，如：

- license读取和解析
- 配网
- 设备绑定、解绑
- 设备上线
- 设备升级
- 灯泡物模型协议



**功能改造**

如应用模板所实现的功能未能满足你的业务需求，如设备升级需要对老数据做兼容处理，或者设备解绑需要重置某项参数，你需要找到对应的源码并对其进行改造。



**协议改造**

如你的实现的不是灯泡应用，应根据萤石云平台所定的物模型协议，对**ezcloud_tsl_protocol.c**文件进行改造，实现新定义的物模型协议。



## 应用移植

​        假设我已经完成了ezapp_hello的开发，但目前仅能在linux上运行。现在接到一个任务，需要在esp8266上实现相同功能，这时应考虑应用移植。第一步需要完成OS层接口适配，参考：[移植指南](./docs/ezos_porting_guide.md)。第二步则需要做应用移植。



### 新建bsp目录

```
+-- ezapp_hello
    +-- port                           // 芯片相关实现
        +-- inc
        |   +-- hal_config.h           // 本地存储接口抽象
        |   +-- hal_ota.h              // 升级接口抽象
        |
        +-- linux                      // linux 通用实现
        +-- esp8266                    // esp8266通用实现
            +-- inc                    // 组件对外头文件
            +-- main                   // 源码实现
            |   +-- CMakeLists.txt     // EZOS应用构建脚本
            |   +-- component.mk       // 乐鑫IDF应用构建脚本
            |   +-- hal_config.c       // 本地存储抽象接口实现
            |   +-- hal_flash.c        // KV抽象接口实现
            |   +-- hal_ota.c          // 升级抽象接口实现
            |   +-- main.c             // 应用入口
            |
            +-- CMakeLists.txt         // EZOS应用构建入口
            +-- Makefile               // 乐鑫IDF应用构建入口
            +-- partitions.csv         // 乐鑫IDF分区表
            +-- sdkconfig              // 乐鑫IDF配置表
```

### 实现应用层入口

```
+-- esp8266                    // esp8266通用实现
    +-- main
    |   +-- main.c             // 应用入口
```



**main.c**

```c
static void boot_info_show(void)
{
    //TODO 输出固件关键信息

    ezos_printf("ezapp, easy your life!\r\n");
    ezos_printf("fwver:%s\r\n", dev_info_get_fwver());

    // 初始化日志模块，默认日志等级为WARN
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(EZ_ELOG_LVL_DEBUG);
}

static void board_init(void)
{
    //TODO 初始化芯片关键驱动、中断
}

static void factory_data_load(void)
{
    //TODO 初始化存储模块, 加载设备出厂配置

    const ez_int32_t max_len = 2048;
    ez_int32_t read_len = 0;
    ez_char_t *buf = ezos_malloc(max_len);
    hal_config_init();

    ezos_bzero(buf, max_len);
    read_len = hal_config_lic_load((ez_char_t *)buf, max_len);
    if (!dev_info_init(buf, read_len))
    {
        ezlog_a(TAG_APP, "Invalid lic!");
    }

    ezos_free(buf);
}

static int integrity_check()
{
    // TODO 出厂数据校验/签名校验
    // 暂时不校验

    // TODO 是否完成产测

    return 0;
}

static void app_global_init(void)
{
    // TODO 初始化各业务模块以及组件

    network_init();
    network_wifi_prov_update();
    if (network_wifi_prov_need())
    {
        network_wifi_prov_do();
        network_wifi_prov_waitfor();
    }

    network_connect_start();
    ezcloud_link_start();

    // 2.定时计划启动

    // 3.场景启动等等
}

static void app_entry_done()
{
    //TODO 回收资源或开启监控
}

int app_main(void)
{
    boot_info_show();

    board_init();

    factory_data_load();

    if (0 != integrity_check())
    {
        goto done;
    }

    app_global_init();
done:
    app_entry_done();

    return 0;
}
```



### 实现抽象接口

不同芯片，甚至相同芯片的不同应用，升级分区和存储分区都不尽相同，所以每适配一个芯片，都要对其对相关接口进行调整。

```
+-- esp8266
    +-- main
    |   +-- main.c
    |   +-- hal_config.c       // 本地存储抽象接口实现
    |   +-- hal_flash.c        // KV抽象接口实现
    |   +-- hal_ota.c          // 升级抽象接口实现
```



### 实现构建脚本

```
+-- esp8266
    +-- main
    |   +-- main.c
    |   +-- hal_config.c
    |   +-- hal_flash.c
    |   +-- hal_ota.c
    |   +-- CMakeLists.txt     // EZOS应用构建脚本
```



**构建脚本CMakeLists.txt**：

```cmake
################# Add include #################
list(APPEND ADD_INCLUDE "../inc")
set(PRIVATE_SRC_ROOT "../../..")
list(APPEND ADD_INCLUDE "${PRIVATE_SRC_ROOT}/bussiness/inc")
list(APPEND ADD_INCLUDE "${PRIVATE_SRC_ROOT}/ezcloud/inc")
list(APPEND ADD_INCLUDE "${PRIVATE_SRC_ROOT}/network/inc")
list(APPEND ADD_INCLUDE "${PRIVATE_SRC_ROOT}/port/inc")
list(APPEND ADD_INCLUDE "${PRIVATE_SRC_ROOT}/product/inc")
###############################################

############## Add source files ###############
aux_source_directory(${PRIVATE_SRC_ROOT}/bussiness ADD_SRCS)
aux_source_directory(${PRIVATE_SRC_ROOT}/ezcloud ADD_SRCS)
aux_source_directory(${PRIVATE_SRC_ROOT}/network ADD_SRCS)
aux_source_directory(${PRIVATE_SRC_ROOT}/port ADD_SRCS)
aux_source_directory(${PRIVATE_SRC_ROOT}/product ADD_SRCS)
###############################################

###### Add required/dependent components ######
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
register_component(${CONFIG_EZIOT_COMPONENT_APP_NAME})
```



### 增加导出头文件

```
+-- esp8266
    +-- inc
    |   +-- fal_cfg.h          // kvdb分区表
    |
    +-- main
    |   +-- main.c
    |   +-- hal_config.c
    |   +-- hal_flash.c
    |   +-- hal_ota.c
    |   +-- CMakeLists.txt     // EZOS应用构建脚本
```



esp8266不支持文件系统，所以采用FlashDB的FAL模式，由于FlashDB编译时需要依赖分区表，所以应用组件：

- 将fal_cfg.h头文件导出，让外部组件可以访问此头文件。

```cmake
list(APPEND ADD_INCLUDE "../inc")
```

- 在应用组件的CMakeLists.txt中注册组件名称为CONFIG_EZIOT_COMPONENT_APP_NAME变量。

```cmake
register_component(${CONFIG_EZIOT_COMPONENT_APP_NAME})
```

- 在FlashDB的CMakeLists.txt中依赖于CONFIG_EZIOT_COMPONENT_APP_NAME组件。

```cmake
if(CONFIG_EZIOT_COMPONENT_FLASHDB_USING_FAL_MODE)
    list(APPEND ADD_REQUIREMENTS ${CONFIG_EZIOT_COMPONENT_APP_NAME})
endif()
```



这样应用组件就能先于FlashDB编译，FlashDB编译时也能访问到分区表（fal_cfg.h）



### 添加EZOS构建入口

```
+-- esp8266
    +-- inc
    |   +-- fal_cfg.h
    |
    +-- main
    |   +-- main.c
    |   +-- hal_config.c
    |   +-- hal_flash.c
    |   +-- hal_ota.c
    |   +-- CMakeLists.txt
    |
    +-- CMakeLists.txt         // EZOS应用构建入口
```

- 构建入口以上已经说过，这里不再展开详述。



### 适配IDF构建脚本

​        截止目前，已经可以编译EZOS所有的组件了，但我们不想对IDF的构建系统进行改造，需要在IDF的脚本中将EZOS所有组件引用，然后执行编译链接。IDF的构建系统是基于GNU make的，需要makefile文件 + component.mk文件。参考IDF示例，新增两个构建脚本文件，如下：

```
+-- esp8266
    +-- inc
    |   +-- fal_cfg.h
    |
    +-- main
    |   +-- main.c
    |   +-- hal_config.c
    |   +-- hal_flash.c
    |   +-- hal_ota.c
    |   +-- CMakeLists.txt
    |   +-- component.mk       // 乐鑫IDF应用构建脚本
    |
    +-- CMakeLists.txt
    +-- Makefile               // 乐鑫IDF应用构建入口
    +-- partitions.csv         // 乐鑫IDF分区表
    +-- sdkconfig              // 乐鑫IDF配置表
```



- component.mk

```makefile
PWD := $(shell pwd)
include ${PWD}/../../config/ezos_gconfig.mk

# 包含hal头文件目录
COMPONENT_INCLUDES += ${PWD}/../../../inc

# 添加EZOS头文件目录
COMPONENT_INCLUDES += ${PWD}/../../config
COMPONENT_INCLUDES += $(CONFIG_ADD_EZOS_INC_DIRS)

# 添加EZOS库文件目录
COMPONENT_ADD_LDFLAGS += $(CONFIG_ADD_EZOS_LIB_DIRS)
COMPONENT_ADD_LDFLAGS += $(CONFIG_ADD_EZOS_LIB_DEPENS)
```

- Makefile

```makefile
export IDF_PATH=/opt/esp_sdk/ESP8266_RTOS_SDK

# build ez_iot_os
MKDIR:=$(shell mkdir libs)
DOCMAKE:=$(shell cmake -S ./ -B libs)
DOMAKE:=$(shell make -C libs)

# build esp idf
PROJECT_NAME := ezapp_hello
include $(IDF_PATH)/make/project.mk
```



### 编译

```shell
cd ~/ezapp_hello/port/esp8266
make
```



编译成功：

```shell
AR build/libsodium/liblibsodium.a
LD build/ezapp_hello.elf
esptool.py v2.4.0
To flash all build output, run 'make flash' or:
python /opt/esp_sdk/ESP8266_RTOS_SDK/components/esptool_py/esptool/esptool.py --chip esp8266 --port /dev/ttyUSB0 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 2MB 0xd000 /home/xurongjun/ezapp_hello/port/esp8266/build/ota_data_initial.bin 0x0 /home/xurongjun/ezapp_hello/port/esp8266/build/bootloader/bootloader.bin 0x14000 /home/xurongjun/ezapp_hello/port/esp8266/build/ezapp_bulb.bin 0x8000 /home/xurongjun/ezapp_hello/port/esp8266/build/partitions.bin
```

