```c

+-- ezapp_bulb                                        // 智能灯应用
|   +-- ezcloud                                       // 萤石云接入
|       +-- ezcloud_access.c
|       +-- ezcloud_base.c
|       +-- ezcloud_ota.c
|       +-- ezcloud_tsl.c
|   +-- distribution                                  // 无线配网
|       +-- wd_ap.c
|       +-- wd_ble.c
|   +-- bussiness                                     // 灯业务相关
|       +-- ezcloud_tsl_protocol.c                    // 物模型协议
|       +-- bulb_base.c
|       +-- bulb_plan.c
|       +-- bulb_sense.c
|       +-- bulb_countdown.c
|       +-- bulb_storage.c
|   +-- product                                       // 产测相关
|       +-- product_test.c
|       +-- product_config.c
|       +-- device_info.c
|   +-- port                                          // 编译时&运行时适配
|       +-- inc
|           +-- hal_config.h
|           +-- hal_flash.h
|           +-- hal_ota.h
|       +-- linux
|       +-- esp8266
|       +-- Kconfig
|   +-- debug                                         // 调试相关
|       +-- cli_cmd.c
|       +-- dev_log.c
|
```

