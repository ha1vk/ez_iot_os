## 简介

base里包含了基础功能，包括设备绑定/解绑，绑定关系查询，校时服务器获取等

## 资源占用

基本不占用资源

## 目录导读

```
+-- base
|+-- inc
|    +-- ez_iot_base.h
|
|+-- inc_private
|    +-- ez_iot_base_ctx.h
|    +-- ez_iot_base_def.h
|    +-- ez_iot_base_extern.h
|    +-- ez_iot_base_protocol.h
|
|+-- src
|    +-- ez_iot_base_ctx.c
|    +-- ez_iot_base_extern.c
|    +-- ez_iot_base_protocol.c
|    +-- ez_iot_base.c
|
|+-- CMakeLists.txt
|+-- Kconfig
|+-- SConscript
```

## 交互设计

![test](.\figures\binding.png)

## 配置项

- EZIOT_BASE_ENABLE：是否使能该模块

## 依赖

- cJSON
- ezxml
- ezlog
- ez_iot_core

## API指南

**函数**

ez_err_t ez_iot_base_init(const ez_base_notice pfunc)

**返回**

```
    typedef enum
    {
        EZ_BASE_ERR_SUCC = 0x00,                                   ///< Success
        EZ_BASE_ERR_NOT_INIT = BASE_MODULE_ERRNO_BASE + 0x01,      ///< The module is not initialized
        EZ_BASE_ERR_NOT_READY = BASE_MODULE_ERRNO_BASE + 0x02,     ///< The sdk core module is not started
        EZ_BASE_ERR_PARAM_INVALID = BASE_MODULE_ERRNO_BASE + 0x03, ///< The input parameters is illegal, it may be that some                                                                             parameters can not be null or out of range
        EZ_BASE_ERR_GENERAL = BASE_MODULE_ERRNO_BASE + 0x04,       ///< Unknown error
        EZ_BASE_ERR_MEMORY = BASE_MODULE_ERRNO_BASE + 0x05,        ///< Out of memory
    } ez_base_err_e;                                               ///< internal conversion to ez_err_t
```

**参数**

- pfunc：基础功能通知函数（ez_base_notice是函数指针类型，修饰pfunc，形参pfunc的实参函数地址传入后，函数指针初始化于该地址，所以一旦调用函数指针，相当于调用了函数指针指向地址的函数）

**示例**

```
static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    switch (event_type)
    {
    case EZ_EVENT_BINDING:
    {
    }
    ......
}
ez_iot_base_init(ez_base_notice_func);
```



**函数**

ez_void_t ez_iot_base_deinit(void)

**返回**

- 无返回

**参数**

- 无参数

**示例**

```
ez_iot_base_deinit();
ez_iot_core_deinit();
```



**函数**

ez_err_t ez_iot_base_bind_query()

**返回**

```
    typedef enum
    {
        EZ_BASE_ERR_SUCC = 0x00,                                   ///< Success
        EZ_BASE_ERR_NOT_INIT = BASE_MODULE_ERRNO_BASE + 0x01,      ///< The module is not initialized
        EZ_BASE_ERR_NOT_READY = BASE_MODULE_ERRNO_BASE + 0x02,     ///< The sdk core module is not started
        EZ_BASE_ERR_PARAM_INVALID = BASE_MODULE_ERRNO_BASE + 0x03, ///< The input parameters is illegal, it may be that some                                                                             parameters can not be null or out of range
        EZ_BASE_ERR_GENERAL = BASE_MODULE_ERRNO_BASE + 0x04,       ///< Unknown error
        EZ_BASE_ERR_MEMORY = BASE_MODULE_ERRNO_BASE + 0x05,        ///< Out of memory
    } ez_base_err_e;                                               ///< internal conversion to ez_err_t
```

**参数**

- 无参数

**示例**

```
ez_iot_base_init(ez_base_notice_func);
ez_iot_base_bind_query(); 
```



**函数**

ez_err_t ez_iot_base_bind_near(ez_char_t *bind_token)

**返回**

```
    typedef enum
    {
        EZ_BASE_ERR_SUCC = 0x00,                                   ///< Success
        EZ_BASE_ERR_NOT_INIT = BASE_MODULE_ERRNO_BASE + 0x01,      ///< The module is not initialized
        EZ_BASE_ERR_NOT_READY = BASE_MODULE_ERRNO_BASE + 0x02,     ///< The sdk core module is not started
        EZ_BASE_ERR_PARAM_INVALID = BASE_MODULE_ERRNO_BASE + 0x03, ///< The input parameters is illegal, it may be that some                                                                             parameters can not be null or out of range
        EZ_BASE_ERR_GENERAL = BASE_MODULE_ERRNO_BASE + 0x04,       ///< Unknown error
        EZ_BASE_ERR_MEMORY = BASE_MODULE_ERRNO_BASE + 0x05,        ///< Out of memory
    } ez_base_err_e;                                               ///< internal conversion to ez_err_t
```

**参数**

- bind_token：http下发的一串字符，需要从http的报文中解析出来，作为参数传入此函数进行绑定

**示例**

```
ez_char_t *dev_token = "68b8efe8246f461691971c95eb8ba725";       
ez_iot_base_bind_near(bind_token)
```



**函数**

ez_err_t ez_iot_base_bind_response(ez_int32_t challenge_code)

**返回**

```
    typedef enum
    {
        EZ_BASE_ERR_SUCC = 0x00,                                   ///< Success
        EZ_BASE_ERR_NOT_INIT = BASE_MODULE_ERRNO_BASE + 0x01,      ///< The module is not initialized
        EZ_BASE_ERR_NOT_READY = BASE_MODULE_ERRNO_BASE + 0x02,     ///< The sdk core module is not started
        EZ_BASE_ERR_PARAM_INVALID = BASE_MODULE_ERRNO_BASE + 0x03, ///< The input parameters is illegal, it may be that some                                                                             parameters can not be null or out of range
        EZ_BASE_ERR_GENERAL = BASE_MODULE_ERRNO_BASE + 0x04,       ///< Unknown error
        EZ_BASE_ERR_MEMORY = BASE_MODULE_ERRNO_BASE + 0x05,        ///< Out of memory
    } ez_base_err_e;                                               ///< internal conversion to ez_err_t
```

**参数**

- challenge_code：云端下发给SDK，SDK通过回调传上来，只要通过函数发送就表示挑战成功了，SDK就可以绑定成功了

**示例**

```
static ez_int32_t ez_base_notice_func(ez_base_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    switch (event_type)
    {
    case EZ_EVENT_BINDING_CHALLENGE:
    {
        bind_chanllenge = (ez_bind_challenge_t *)data;
        m_challenge_code = bind_chanllenge->challenge_code;
    }
    ......
}
ez_iot_base_init(ez_base_notice_func);
ez_iot_base_bind_response(challenge_code);
```

