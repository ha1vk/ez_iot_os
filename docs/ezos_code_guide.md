

# 代码导读

## 代码目录结构

```c
ez-iot-os
|   apps
|   +-- ezapp_template    // 应用开发模板
|   +-- ezapp_bulb        // 智能灯应用
|
+-- components
|   +-- ezconn            // ap配网组件
|   +-- ezlist            // 链表
|   +-- ezlog             // 日志组件
|   +-- eztimer           // 定时器
|   +-- ezutil            // 工具类接口
|   +-- ezxml             // xml解析器
|   +-- FlashDB           // 键值对数据库
|   +-- http_server       // Http Client
|   +-- mbedtls           // 密码算法套件
|   +-- mqtt              // ibm mqtt
|   +-- utest             // 单元测试框架
|   +-- webclient         // Http Client
|
+-- examples              // 示例代码
|
+-- eziot                 // 萤石IoT云端功能
|   +-- ez_iot_core       // 接入模块，如认证、连接
|   +-- ez_iot_bm         // 业务模块
|       +-- base          // 基础功能，如绑定、时区等
|       +-- ota           // 升级功能
|       +-- shadow        // 设备影子，属性一致性
|       +-- tsl           // 物模型
|       +-- hub           // 子设备管理
|
+-- platform              // OS、硬件适配
|   +-- inc               // 适配接口声明
|   +-- bsp
|       +-- esp8266       // 适配乐鑫 esp8266
|       +-- esp32         // 适配乐鑫 esp32
|       +-- rt-thread     // 适配睿赛德 RT-Thread
|       +-- linux         // 适配通用 Linux
|
+-- tools                 // 辅助工具
|   +-- cmake             // 构建框架组织脚本
|   +-- kconfig           // kconfig lib
|
+-- unit_test             // 单元测试代码
+
+-- Kconfig               // Kconfig一级组织目录
+-- SConscript            // Kconfig一级组织目录(for RT-Thread)

```



## 业务组件

| 组件 | 关联文档 | 描述 |
| ---- | ---- | ---- |
| [core](../eziot/ez_iot_core) | [README.md](../eziot/ez_iot_core/README.md) -- 待补充 -- | 云端接入。如认证、连接。 |
| [base](../eziot/ez_iot_bm/base) | [README.md](../eziot/ez_iot_bm/base/README.md) | 基础功能。如绑定/解绑、时区。 |
| [tsl](../eziot/ez_iot_bm/tsl) | [README.md](../eziot/ez_iot_bm/tsl/README.md) -- 待补充 -- | 物模型。属性、事件、操作。 |
| [ota](../eziot/ez_iot_bm/ota) | [README.md](../eziot/ez_iot_bm/ota/README.md) -- 待补充 -- | 设备升级。                    |
| | | |

## 基础组件

| 组件 | 关联文档 | 描述 |
| ---- | ---- | ---- |
| [ezconn](../components/ezconn) | [README.md](../components/ezconn/README.md) | WiFi AP配网 |
| [http_server](../components/http_server) | [README.md](../components/http_server/README.md) | Http Server |
| [ezlog](../components/ezlog) | [README.md](../components/ezlog/README.md) | 日志 |
| [FlashDB](../components/FlashDB) | [README.md](../components/FlashDB/README.md) | 数据库 |
| | | |



