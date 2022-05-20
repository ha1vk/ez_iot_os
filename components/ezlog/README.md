# 组件模板

## 1.简介

ezlog组件是基于开源工程[EasyLogger](https://github.com/armink/EasyLogger) 移植封装的高性能的 C/C++ 日志库，非常适合对资源敏感的软件项目，例如： IoT 产品、可穿戴设备、智能家居等等。相比 log4c、zlog 这些知名的 C/C++ 日志库， EasyLogger 的功能更加简单，提供给用户的接口更少，但上手会很快，更多实用功能支持以插件形式进行动态扩展

## 2.资源占用

| RAM资源占用项目 | 占用大小(bytes) |
| --------------- | --------------- |
| rom             | <1.6K           |
| ram             | <0.3K           |

## 3.主要特性

- 支持用户自定义输出方式（例如：终端、文件、数据库、串口、485、Flash...）；
- 日志内容可包含级别、时间戳、线程信息、进程信息等；
- 日志输出被设计为线程安全的方式，并支持 **异步输出** 及 **缓冲输出** 模式；
- 支持多种操作系统（[RT-Thread](http://www.rt-thread.org/)、UCOS、Linux、Windows...），也支持裸机平台；
- 日志支持 **RAW格式** ，支持 **hexdump** ；
- 支持按 **标签** 、 **级别** 、 **关键词** 进行动态过滤；
- 各级别日志支持不同颜色显示；
- 扩展性强，支持以插件形式扩展新功能。

> 名词解释：
>
> - 1、RAW格式：未经过格式化的原始日志。
> - 2、标签：在软件中可以按照文件、模块、功能等方面，对需要打印的日志设定标签，实现日志分类。

## 4. 使用

### 4.1配置项

```
CONFIG_EZIOT_COMPONENT_EZLOG_ENABLE=y
CONFIG_EZIOT_COMPONENT_EZLOG_OUTPUT_ENABLE=y

# CONFIG_EZIOT_COMPONENT_EZLOG_ASSERT_ENABLE is not set

CONFIG_EZIOT_COMPONENT_EZLOG_LINE_BUF_SIZE=1024
CONFIG_EZIOT_COMPONENT_EZLOG_LINE_NUM_MAX_LEN=5
CONFIG_EZIOT_COMPONENT_EZLOG_FILTER_TAG_MAX_LEN=16
CONFIG_EZIOT_COMPONENT_EZLOG_FILTER_KW_MAX_LEN=16
CONFIG_EZIOT_COMPONENT_EZLOG_FILTER_TAG_LVL_MAX_NUM=5
CONFIG_EZIOT_COMPONENT_EZLOG_NEWLINE_SIGN_CRLF=y

# CONFIG_EZIOT_COMPONENT_EZLOG_NEWLINE_SIGN_LF is not set

# CONFIG_EZIOT_COMPONENT_EZLOG_NEWLINE_SIGN_CR is not set

CONFIG_EZIOT_COMPONENT_EZLOG_COLOR_ENABLE=y

# CONFIG_EZIOT_COMPONENT_EZLOG_ASYNC_OUTPUT_ENABLE is not set

# CONFIG_EZIOT_COMPONENT_EZLOG_BUF_OUTPUT_ENABLE is not set
```

### 4.2 日志输出级别

```
#define EZ_ELOG_LVL_ASSERT 0  ///< 致命错误，导致整个程序无法继续运行
#define EZ_ELOG_LVL_ERROR 1   ///< 某个业务出错，不影响其他业务
#define EZ_ELOG_LVL_WARN 2    ///< 打印业务过程中必要的关键信息，尽量简短（WARN<=会记入文件）
#define EZ_ELOG_LVL_INFO 3    ///< 较详细的信息（不允许刷屏）
#define EZ_ELOG_LVL_DEBUG 4   ///< 更为详细的信息，每行带有行号（不允许刷屏）
#define EZ_ELOG_LVL_VERBOSE 5 ///< 不限制打印，每行带有行号，默认不开启。（不允许刷屏）
```

### 4.3 程序使用

```
 //详见源码ut_ezlog.c文件，调用如下三个api 后就可以采用统一的日志进行打印
 ezlog_init();
 ezlog_start();
 ezlog_filter_lvl(3);
```

## 5 依赖

- ezos