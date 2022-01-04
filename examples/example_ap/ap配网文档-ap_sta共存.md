[TOC]

---

---

### 第一部分 设备接入配网

#### 1. 接口

```c++
    typedef enum
    {
        EZCONN_WIFI_MODE_AP, 
        EZCONN_WIFI_MODE_STA,
        EZCONN_WIFI_MODE_APSTA,
    } ezconn_wifi_mode_e;
    /**
     * @brief   设置wifi模式，ap模式/station模式/ap+station模式 
     * 
     * @param   wifi_mode ： see ezconn_wifi_mode_e
     * @return  0 for success, other for failed
     */
    ez_err_t ezconn_wifi_config(ezconn_wifi_mode_e wifi_mode);


    // 设备信息，用于ap初始化
    typedef struct
    {
        ez_int8_t ap_ssid[32 + 1];      // ap ssid
        ez_int8_t ap_pwd[64 + 1];       // ap password, 不需要密码则传空
        ez_int8_t auth_mode;            // ap auth mode, 0 for no ap_password 
        ez_uint8_t channel;             // ap channel

        ez_int8_t ap_timeout;           // 配网超时时间，单位min
        ez_bool_t apsta_coexist;        // 是否支持ap station模式共存
        ez_int8_t res[2];
    } ezconn_ap_info_t;

    typedef struct 
    {
        ez_int8_t dev_serial[72];        // 设备序列号
        ez_int8_t dev_type[64];          // 设备类型
        ez_int8_t dev_version[64];       // 设备版本号
    } ezconn_dev_info_t;
	/**
     *  @fn         AP模块初始化，包括初始化ap以及启动httpserver
     *  @param[in]  dev_info：设备信息
     *  @param[in]  cb：回调函数，用于获取wifi设置状态
     *  @param[in]  timeout：wifi配置超时时间，范围1-60，单位min，默认15min
     *  @param[in]  support_apsta：是否支持ap、sta共存模式
     *  @return     成功：EZCONN_SUCC，失败：other
     *  @info       需要在ezconn_wifi_config之后调用
     */
    ez_err_t ezconn_ap_start(ezconn_ap_info_t *ap_info, ezconn_dev_info_t *dev_info, wifi_info_cb cb);

    /**
     *  @fn         AP模块反初始化，包括停掉httpserver和ap模块
     *  @return     成功：EZCONN_SUCC，失败：other
     */
    ez_err_t ezconn_ap_stop();
```

```c++
typedef enum
    {
        EZCONN_STATE_SUCC = 0,             ///< ap config succ
        EZCONN_STATE_APP_CONNECTED ,       ///< app connected
        EZCONN_STATE_CONNECTING_ROUTE ,    ///< connecting
        EZCONN_STATE_CONNECT_FAILED,       ///< connect failed
        EZCONN_STATE_WIFI_CONFIG_TIMEOUT,  ///< connect timeout
    } ezconn_state_e;

    // 配网过程获取到的信息
    typedef struct
    {
        ez_int8_t ssid[32+1];        // 设备连接的wifi ssid
        ez_int8_t password[64+1];    // 设备连接的wifi password
        ez_int8_t res[2];

        ez_int8_t cc[4];             // 国家码cc
        ez_int8_t token[128];        // app端校验的token
    
        ez_int8_t domain[128];       // 设备注册平台地址;
        ez_int8_t device_id[32];     // 设备uuid，可选配置
    } ezconn_wifi_info_t;

	/**
     *  @brief  给上层wifi信息的回调函数
     *  @param  err_code: 配网错误码, see ezconn_errno_e
     *  @param  wifi_info: 给应用层的数据
     *  @info   回调中先判断err_code，若err_code为EZCONN_SUCC，则可以从参数wifi_info中拿到ssid和密码
     *          否则，均为配网失败
     *  @warn   回调中不能处理过多的业务，否则可能会导致栈溢出
     */
    typedef void (*wifi_info_cb)(ezconn_state_e err_code, ezconn_wifi_info_t *wifi_info);
```



#### 2.接入流程

- 首先启动设备ap，示例如下（具体见example10_ap/esp32/main/ap_demo.c)：

```c++
void example_ap_init_open_apsta()
{
    ezconn_dev_info_t dev_info = {0};
    ezconn_ap_info_t ap_info = {0};


    strncpy((char *)ap_info.ap_ssid, "EZVIZ_AP_11112", sizeof(ap_info.ap_ssid) - 1);
    ap_info.auth_mode = 0;
    ap_info.channel = 1;
    ap_info.ap_timeout = 5;
    ap_info.apsta_coexist = ez_true;

    strncpy((char *)dev_info.dev_serial, "88888888", sizeof(dev_info.dev_serial) - 1);
    strncpy((char *)dev_info.dev_type, "EZ_001", sizeof(dev_info.dev_type) - 1);
    strncpy((char *)dev_info.dev_version, "V1.0.0 build 210302", sizeof(dev_info.dev_version) - 1);
    ezconn_wifi_init();
    ezconn_wifi_config(EZCONN_WIFI_MODE_APSTA);
    ezconn_ap_start(&ap_info, &dev_info, wifi_cb);
    return;
}
```

- 将**设备信息、回调函数、以及ap超时时间**通过接口<font color="blue">***ezconn_ap_start***</font>设置。示例如下：

```c++
const char *TAG_DEMO = "T_DEMO";

static void wifi_cb(ezconn_state_e err_code, ezconn_wifi_info_t *wifi_info)
{
    switch (err_code)
    {
    case EZCONN_STATE_APP_CONNECTED:
        ezlog_w(TAG_DEMO, "app connected.");
        break;
    case EZCONN_STATE_SUCC:
        ezlog_w(TAG_DEMO, "wifi config success.");
        ezlog_i(TAG_DEMO, "ssid: %s", wifi_info->ssid);
        ezlog_i(TAG_DEMO, "password: %s", wifi_info->password);
        ezlog_i(TAG_DEMO, "token: %s", wifi_info->token);
        ezlog_i(TAG_DEMO, "domain: %s", wifi_info->domain);
        ezconn_ap_stop();
        break;
    case EZCONN_STATE_CONNECTING_ROUTE:
        ezlog_w(TAG_DEMO, "connecting route.");
        break;
    case EZCONN_STATE_CONNECT_FAILED:
        ezlog_w(TAG_DEMO, "connect failed.");
        break;
    case EZCONN_STATE_WIFI_CONFIG_TIMEOUT:
        ezlog_w(TAG_DEMO, "wifi config timeout.");
        ezconn_ap_stop();
        break; 
    default:
        break;
    }
}
```

- 由回调<font color="blue">***wifi_cb***</font>返回的wifi相关信息，**token**用于注册平台，**domain**为注册平台地址，应用层需保存，用于接入sdk上平台。


- 设备ap热点启动，信息如下：
  > ap配网服务地址：192.168.4.1
  >
  > ap配网服务端口：80
  >
  > ap配网服务协议：http协议
  
- ap热点启动之后，由app与设备通过http协议进行交互，包括**获取设备信息、获取wifi列表、以及设置需要连接的wifi ssid和password**，具体见第二部分app与设备交互。

---

---

### 第二部分 app与设备配网交互

#### 1. ap配网交互流程

```sequence
Title:ap配网交互图
#dev->dev:ap start
app->dev:connect ap
app->dev:req: HTTP Get /AccessDevInfo
dev-->app:rsp: device info
app->dev:req: HTTP Get /PreNetwork/SecurityAndAccessPoint
dev-->app:rsp: wifi lists
app->dev:req: HTTP PUT /PreNetwork/WifiConfig
dev->dev:connect wifi\nget result
dev-->app:rsp: wifi connect error code
```

#### 2. 交互协议

- 获取设备相关信息

```http
Get /AccessDevInfo
描述：APP端连接之后获取设备相关信息 
入口数据：无 

返回：
    {
         "ap_version":"1.0",//版本，当前ap版本1.0
         "dev_subserial":"123456789",//设备序列号，最大64
         "dev_type":"EZ1",//设备型号，最大64
         "dev_firmwareversion":"V1.0.0 build 190823"//设备固件版本号，最大64
    }
```

- 获取wifi列表

```http
Get /PreNetwork/SecurityAndAccessPoint
描述：AP接入wifi配置操作时，获取相关信息 （最大设置为20个）
入口数据：无 

返回：
	{
      "access_point_list":[{
       "ssid":"",
       /*必填,SSID,string*/
       "signal_strength":-20,
       /*必填,信号强度,"-100-0",int，数值越大信号越强*/
       "security_mode":"",
       /*可选,安全模式: "open,WEP,WPA-personal,WPA2-personal,WPA-WPA2-personal,WPA2-enterprise",string*/
      }]
    ｝
```

- 设置wifi **ssid和password**

```http
PUT /PreNetwork/WifiConfig
描述： AP接入Wifi配置操作（wifi连接可能会耗一定的时间，这里设置wifi连接超时时间为10s） 
入口数据：
	{
      "token":"",//必填，用于认证
      "lbs_domain":"",//必填，设备注册平台地址
      "device_id":"", //可选，设备uuid
      "wifi_info":
      {
       "ssid":"",
       /*必填,wifi的ssid,string*/
       "password":"",
       /*必填,wifi的密码信息,string*/
      },
    }
    
返回：
	{
         "status_code"："" //状态码， int类型
         "status_string"：""//状态描述， string类型
    }
```

#### 3. 错误码定义

由app设置wifi信息之后，设备返回本次wifi连接的错误码<font color="red">status_code</font>，定义如下：

```c++
typedef enum
{
    EZOS_WIFI_STATE_NOT_CONNECT     = 102,  // 设备未连接路由器
    EZOS_WIFI_STATE_CONNECT_SUCCESS = 104,  // 设备连接路由器成功
    EZOS_WIFI_STATE_PASSWORD_ERROR  = 106,  // 密码错误
    EZOS_WIFI_STATE_NO_AP_FOUND     = 201,  // app设置路由器ssid未找到
    EZOS_WIFI_STATE_UNKNOW          = 202,  // 未知错误
} ezos_wifi_state_e;
```

app端可根据配网返回的错误码，自定义app的提示界面。

