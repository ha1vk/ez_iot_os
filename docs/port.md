# EZOS组件及芯片移植文档

| **版本号** | **修订内容** | **修订人** | **修订日期** |
| ---------- | ------------ | ---------- | ------------ |
| 0.1        | 文档初版     | 徐荣军     | 2021/10/13   |

## 软件架构图

![软件架构图](.\figures\ez_idf_arch.png) 



## 构建框架组织关系

​		因芯片本身的资源、开发包接口及业务的差异，SDK需要在预编译的时候进行差异化处理。ezos基于cmake+kconfig的构建框架，将iot核心模块、业务模块、三方组件进行解耦，每个模块可将差异项配置在各自Kconfig中，最终由构建框架汇聚进行统一配置，生成定制化SDK。

```c
ez-iot-os
|   Kconfig----------4------------------------------|   // 构建脚本一级菜单
|       ↑------------------------------<--------|   |
+-- components                                  |   |   // 组件目录
|   +-- component1                              |   |   // 新增组件
|       +-- inc                                 |   |   // 全局头文件
|       +-- inc_private                         |   |   // 局部头文件
|       +-- CMakeLists.txt<-----------------|   |   |   // 组件构建脚本
|       +-- Kconfig	<----------------------<|<-<|<--|   // 组件配置
+-- eziot                                   |   |   |
|   +-- core<------------------------------<|<-<|<--|   // 云连接核心模块
|   +-- link<------------------------------<|<-<|<--|   // 云连接基础模块
|   +-- extensions                          |   |   |   // 云连接业务模块
|       +-- extension1                      |   |   |   // 新增业务模块
|           +-- inc                         |   |   |   // 同上
|           +-- inc_private                 |   |   |
|           +-- CMakeLists.txt<-------------|   |   |
|           +-- Kconfig<-------------------<|<-<|<--|
+-- platform                                |   |       // 移植目录
|   +-- mcu                                 |   |       // 适配的平台或mcu
|       +-- linux                           |   |
|           +-- config                      |   |
|           +-- .config                     |   |       // kconfig配置文件
|           +-- .config.old                 |   |       // kconfig上一次配置
|           +-- ezos_gconfig.cmake          |   |       // cmake配置文件
|               +-- ezos_gconfig.h          |   |       // 源码全局配置文件
|           +-- osal                        |   |
|               +-- CMakeLists.txt<---------|   |       // osal构建脚本
|           +-- CMakeLists.txt----1-----↓   |   |       // 全局构建脚本入口
|       +-- windows                     |   |   |
|       +-- ...                         |   |   |
|   +-- osal                            |   |   |       // libc、posix接口抽象
|   +-- hal                             |   |   |       // wifi、串口等各类驱动抽象
+-- tools                               |   |   |
|   +-- cmake   ↓---------------<-------|   |   |       // cmake构建框架
|       +-- compile.cmake-------2-----------|   |
|               ↓                               |
|       +-- project.py----------3-|             |
|   +-- kconfig <---------<-------|------>------|       // kconfig lib
|
```

## 快速移植

### 移植原则

- 除非必要，尽量不要修改三方开源代码。
- TODO

### 移植组件

​		组件是相对独立的基础库、实用工具或者三方开源代码，其特点是功能独立，和其他模块耦合度低，如json库、加密库、网络检测工具等。在ezos中，组件的目录为ez-iot-os->components，新增组件需放在此目录下。以下用component1为例。

#### 新增组件目录

​		建议组件目录如下：

```c
+-- component1
	+-- inc				// 全局头文件（可以被其他组件inlcude）
   	+-- inc_private		// 组件内部头文件（无法被其他组件inlcude）
   	+-- src				// 组件源码
   	+-- CMakeLists.txt	// 组件构建脚本
   	+-- Kconfig			// 组件配置
```



#### 新增配置选项

​		Kconfig默认配置如下，每个组件必须支持使能选项（非使能情况下，不参与构建）。如组件内部有更多配置项，可以在此基础上追加，Kconfig语法参考：https://www.rt-thread.org/document/site/#/development-tools/kconfig/kconfig.md。

```python
#配置项命名规则为CONFIG_类别_模块名_配置点，CONFIG由Kconfig lib补齐，当前类别有COMPONENT、IOT，模块名有CORE、LINK、EXT、组件名、模块名。

menu "component1 configuration"
    config COMPONENT_COMPONENT1_ENABLED
        bool "Enable component1"
        default n
endmenu
```



#### 新增构建脚本

​		以此下列脚本为模板，根据组件的情况进行更改。如目录有差异、有依赖组件、需追加预编译宏、需链接已编译库、需追加链接参数等。

```shell
if(CONFIG_COMPONENT_COMPONENT1_ENABLED)

    ################# Add include #################
    list(APPEND ADD_INCLUDE "inc"
        )
    list(APPEND ADD_PRIVATE_INCLUDE "inc_private")
    ###############################################

    ############## Add source files ###############
    # list(APPEND ADD_SRCS  "src/lib1.c"
    #     )
    # aux_source_directory(src ADD_SRCS)  # collect all source file in src dir, will set var ADD_SRCS
    append_srcs_dir(ADD_SRCS "src")     # append source file in src dir to var ADD_SRCS
    # list(REMOVE_ITEM COMPONENT_SRCS "src/test.c")
    ###############################################

    ###### Add required/dependent components ######
    list(APPEND ADD_REQUIREMENTS osal)
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
    list(APPEND ADD_DEFINITIONS_PRIVATE)

    #### Add compile option for this component
    #### and components denpend on this component
    list(APPEND ADD_DEFINITIONS)
    ###############################################

    ############ Add static libs ##################
    #### Update parent's variables like CMAKE_C_LINK_FLAGS
    # set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group libmaix/libtest.a -ltest2 -Wl,--end-group" PARENT_SCOPE)
    ###############################################

    # register component, DYNAMIC or SHARED flags will make component compiled to dynamic(shared) lib
    register_component()

endif()
```



#### 执行构建

​		进入对应的芯片构建入口，这里以linux为例。

```shell
cd ./platform/bsp/linux
rm -r build
mkdir build && cd build
cmake ..
```



​		在shell 终端，可以看到这个组件已被构建系统发现。

```shell
-- Find component: /home/xurongjun/git/ez_iot_os/components/cJSON
-- Find component Kconfig of cJSON
-- Find component: /home/xurongjun/git/ez_iot_os/components/mbedtls
-- Find component Kconfig of mbedtls
-- Find component: /home/xurongjun/git/ez_iot_os/components/mqtt
-- Find component Kconfig of mqtt
-- Find component: /home/xurongjun/git/ez_iot_os/components/component1
-- Find component Kconfig of component1
```



​		使用make menuconfig命令，使能该组件并保存。

![组件使能](.\figures\menuconfig.png)



​		在shell 终端输入命令 cmake..，可以看到该组件注册成功。

```shell
-- [register component: cJSON ], path:/home/xurongjun/git/ez_iot_os/components/cJSON
-- component cJSON will compiled to static lib
-- [register component: mbedtls ], path:/home/xurongjun/git/ez_iot_os/components/mbedtls
-- component mbedtls will compiled to static lib
-- [register component: mqtt ], path:/home/xurongjun/git/ez_iot_os/components/mqtt
-- component mqtt will compiled to static lib
-- [register component: xml ], path:/home/xurongjun/git/ez_iot_os/components/component1
-- component component1 will compiled to static lib
```



​		随后在shell 终端输入命令make，完成组件构建。

### 新增业务模块

​		顾名思义，业务模块指的是和萤石业务相关的模块，如设备接入、物模型数据传输、OTA、流媒体传输等。在ezos中，业务模块的目录为ez-iot-os->eziot->extensions，建议新增需放在此目录下。因业务模块的代码组织方式和组件一致，不再一一展开详述。

#### 新增业务模块目录

​		建议业务模块目录如下：

```c
+-- extensions1
	+-- inc				// 全局头文件（可以被其他组件inlcude）
   	+-- inc_private		// 业务模块内部头文件（无法被其他组件inlcude）
   	+-- src				// 业务模块源码
   	+-- CMakeLists.txt	// 业务模块构建脚本
   	+-- Kconfig			// 业务模块配置
```

#### 新增配置选项

​		参考组件移植

#### 新增构建脚本

​		参考组件移植

#### 执行构建

​		参考组件移植



### 移植芯片

​		对于Windows、Linux平台的设备，绝大多数只会有工具链差异或者编译链接参数的差异。对于IoT模块，除了以上差异外，还会有接口的差异，有些模组厂商会自己抽象一套接口、用来屏蔽有操作系统和无操作系统的设备，libc和posix接口支持并不一定好。以上两类设备的差异性，我们选择放在platform中做适配。新增的芯片需完成osal抽象接口的实现。在ezos中，组件的目录为ez-iot-os->platform->mcu，新增组件需放在此目录下。以下用linux为例。

#### 新增mcu目录

​		建议芯片适配目录如下，建议采用芯片型号命令：

```c
|   +-- mcu									// 适配的平台或mcu
|    	+-- linux
|    		+-- config						// kconifg配置文件输入输出目录
|    			+-- .config					// kconfig配置文件
|    			+-- .config.old				// kconfig上一次配置
|    			+-- ezos_gconfig.cmake		// cmake配置文件
|    			+-- ezos_gconfig.h			// 源码全局配置文件
|		    +-- osal
|    			+-- CMakeLists.txt			// osal构建脚本
|    		+-- CMakeLists.txt				// 全局构建脚本入口
|   +-- osal								// libc、posix接口抽象头文件
```



​		config目录有两种生成方式：

1. 从已经完成适配的芯片模块拷贝（业务相近）。
2. make menuconfig完成配置后自动生成。

#### 实现osal

​		osal下的接口需要全部实现，如果该芯片是标准os接口（libc、posix），直接复制linux下的即可。

#### 新增配置选项（如果有）

​		参考组件移植

#### 新增构建脚本

​		参考组件移植

#### 执行构建

​		参考组件移植

## 高级使用方法

//TODO

## FAQ

//TODO