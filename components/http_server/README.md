
## 一、简介

主要实现简单的http server功能。可以自定义端口、允许最大连接数、最大支持的uri个数等。

注：http server的ip地址为协议栈初始化时，dhcp配置的ip和网关地址。

萤石app默认为**192.168.4.1**。


## 二、资源占用

|RAM资源占用项目|占用大小(bytes)|
|----------|-------------|
|主线程栈|默认4096|
|句柄创建|1200|
|连接client数量|默认176*7（7为默认最大连接数）|
|注册URI个数|与uri长度有关|

http server组件主要资源占用情况如上，其中主线程栈空间大小和最大连接数可根据需要做调整，可最大限度的优化内存空间使用。

## 三、配置项

|配置项|说明|默认值|
|----|----|----|
|**EZIOT_COMPONENT_EZHTTPD_ENABLE**|是否开启http server组件|默认开启|
|**EZIOT_COMPONENT_EZHTTPD_STACK_SIZE**|http server主线程栈大小|默认4096字节|
|**EZIOT_COMPONENT_EZHTTPD_SERVER_PORT**| http server端口号|默认80|

## 四、依赖

- ezos：底层接口实现
- ezlog：用于日志打印输出

## 五、API 指南


常用接口如下：具体说明见**http_server.h**

    // http server默认配置
    #define HTTPD_DEFAULT_CONFIG()                \
    {                                         \
        .task_priority = 5,                   \
        .stack_size = CONFIG_EZIOT_COMPONENT_EZHTTPD_STACK_SIZE,                   \
        .server_port = CONFIG_EZIOT_COMPONENT_EZHTTPD_SERVER_PORT,                    \
        .ctrl_port = 32768,                   \
        .max_open_sockets = 7,                \
        .max_uri_handlers = 8,                \
        .max_resp_headers = 8,                \
        .backlog_conn = 5,                    \
        .lru_purge_enable = ez_false,            \
        .recv_wait_timeout = 5,               \
        .send_wait_timeout = 5,               \
        .global_user_ctx = NULL,              \
        .global_user_ctx_free_fn = NULL,      \
        .global_transport_ctx = NULL,         \
        .global_transport_ctx_free_fn = NULL, \
        .open_fn = NULL,                      \
        .close_fn = NULL,                     \
    }
**HTTPD_DEFAULT_CONFIG()**；

    /**
    * @brief Starts the web server
    *
    * Create an instance of HTTP server and allocate memory/resources for it
    * depending upon the specified configuration.
    *
    * Example usage:
    * @code{c}
    *
    * //Function for starting the webserver
    * httpd_handle_t start_webserver(void)
    * {
    *      // Generate default configuration
    *      httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    *
    *      // Empty handle to http_server
    *      httpd_handle_t server = NULL;
    *
    *      // Start the httpd server
    *      if (httpd_start(&server, &config) == ez_errno_succ) {
    *          // Register URI handlers
    *          httpd_register_uri_handler(server, &uri_get);
    *          httpd_register_uri_handler(server, &uri_post);
    *      }
    *      // If server failed to start, handle will be NULL
    *      return server;
    * }
    *
    * @endcode
    *
    * @param[in]  config : Configuration for new instance of the server
    * @param[out] handle : Handle to newly created instance of the server. NULL on error
    * @return
    *  - ez_errno_succ    : Instance created successfully
    *  - ez_errno_param_invalid      : Null argument(s)
    *  - ez_errno_httpd_alloc_mem  : Failed to allocate memory for instance
    *  - ez_errno_httpd_task       : Failed to launch server task
    */
**ez_err_t httpd_start(httpd_handle_t \*handle, const httpd_config_t \*config);**

    /**
    * @brief Stops the web server
    *
    * Deallocates memory/resources used by an HTTP server instance and
    * deletes it. Once deleted the handle can no longer be used for accessing
    * the instance.
    *
    * Example usage:
    * @code{c}
    *
    * // Function for stopping the webserver
    * void stop_webserver(httpd_handle_t server)
    * {
    *      // Ensure handle is non NULL
    *      if (server != NULL) {
    *          // Stop the httpd server
    *          httpd_stop(server);
    *      }
    * }
    *
    * @endcode
    *
    * @param[in] handle Handle to server returned by httpd_start
    * @return
    *  - ez_errno_succ : Server stopped successfully
    *  - ez_errno_param_invalid : Handle argument is Null
    */
**ez_err_t httpd_stop(httpd_handle_t handle);**

    /**
    * @brief   Registers a URI handler
    *
    * @note    URI handlers can be registered in real time as long as the
    *          server handle is valid.
    *
    * Example usage:
    * @code{c}
    *
    * ez_err_t my_uri_handler(httpd_req_t* req)
    * {
    *     // Recv , Process and Send
    *     ....
    *     ....
    *     ....
    *
    *     // Fail condition
    *     if (....) {
    *         // Return fail to close session //
    *         return ez_errno_fail;
    *     }
    *
    *     // On success
    *     return ez_errno_succ;
    * }
    *
    * // URI handler structure
    * httpd_uri_t my_uri {
    *     .uri      = "/my_uri/path/xyz",
    *     .method   = HTTPD_GET,
    *     .handler  = my_uri_handler,
    *     .user_ctx = NULL
    * };
    *
    * // Register handler
    * if (httpd_register_uri_handler(server_handle, &my_uri) != ez_errno_succ) {
    *    // If failed to register handler
    *    ....
    * }
    *
    * @endcode
    *
    * @param[in] handle      handle to HTTPD server instance
    * @param[in] uri_handler pointer to handler that needs to be registered
    *
    * @return
    *  - ez_errno_succ : On successfully registering the handler
    *  - ez_errno_param_invalid : Null arguments
    *  - ez_errno_httpd_handlers_full  : If no slots left for new handler
    *  - ez_errno_httpd_handler_exists : If handler with same URI and
    *                                   method is already registered
    */
**ez_err_t httpd_register_uri_handler(httpd_handle_t handle, const httpd_uri_t \*uri_handler);**

## 六、FAQ

### 问题一

**Q:** http server的IP地址和端口是否可以更改？

**A:** ip地址是dhcp初始化时设置的，即协议栈初始化时设定，http server组件无法修改此IP地址；
        http server端口号默认**80**，可以在http server初始化时或配置文件中做修改。

### 问题二

**Q:** 如何减少http server资源占用？

**A:** 可根据实际情况，裁减主线程栈空间和允许最大连接数