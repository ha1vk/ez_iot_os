# EZOS使用说明文档

| **版本号** | **修订内容** | **修订人** | **修订日期** |
| ---------- | ------------ | ---------- | ------------ |
| 0.1        | 文档初版     | 徐荣军     | 2021/10/13   |

## 简介

EZOS 是是萤石官方推出的物联网开发套件，支持 Windows、Linux 和 macOS 操作系统。

## 基于EZOS进行开发

### 构建环境依赖

- gcc
- Cmake v3.5
- python v2.7

### 芯片适配
<!--如芯片已经完成适配，可跳过此步骤-->

详细请参考：[芯片移植文档](./docs/port.md)

### 快速构建

- 进入已经完成适配的mcu目录
- 使用shell命令完成构建



```shell
cd ./platform/mcu/linux
rm -r build
mkdir build && cd build
cmake ..
make
```

## 其他

更多文档参考doc目录下文档，如：

| **文件名**                         | **描述** |
| :--------------------------------- | :------- |
| [version.md](./docs/version.md)    | 版本信息 |
| [introduction.md](introduction.md) | 详细介绍 |
| [principle.md](principle.md)       | 工作原理 |
| [user-guide.md](user-guide.md)     | 使用指南 |
| [api.md](api.md)                   | API 说明 |
| [samples.md](samples.md)           | 示例说明 |
| [port.md](./docs/.md)              | 移植文档 |