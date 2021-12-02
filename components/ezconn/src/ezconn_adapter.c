#include "http_server.h"
#include "eztimer.h"

#include "ezconn.h"
#include "ezlog.h"
#include "ezos_thread.h"
#include "ezos_time.h"
#include "ezos_wifi.h"
#include "ezconn_process.h"
#include "ezconn_adapter.h"

#include "cJSON.h"

static ezconn_ctx_t g_conn_ctx = {0};
static ez_bool_t g_exit_flag = ez_true;
static ez_bool_t g_server_busy = ez_false;

int32_t ezconn_set_busy_state(ez_bool_t is_busy)
{
    g_server_busy = is_busy;
    return 0;
}

int32_t ezconn_set_exit_state(ez_bool_t is_exit)
{
    g_exit_flag = is_exit;
    return 0;
}

static void stop_httpd(void)
{
    if (NULL != g_conn_ctx.httpd_server)
    {
        ezlog_d(TAG_AP, "stopping httpserver...");
        httpd_stop(g_conn_ctx.httpd_server);
        g_conn_ctx.httpd_server = NULL;
        ezos_delay_ms(1000);
        ezlog_d(TAG_AP, "stop httpserver success.");
    }
    else
    {
        ezlog_d(TAG_AP, "http server is NULL.");
    }
}

ez_err_t http_get_devinfo_handler(httpd_req_t *req)
{
    while (g_server_busy)
    {
        ezos_delay_ms(200);

        ezlog_i(TAG_AP, "http server is busy.");
        if (g_exit_flag)
        {
            ezlog_i(TAG_AP, "get devinfo handler exit.");
            return EZCONN_SUCC;
        }
    }

    g_server_busy = ez_true;
    g_conn_ctx.wifi_cb(EZCONN_STATE_APP_CONNECTED, NULL);
    ezlog_i(TAG_AP, "app connected success. err_code:0x%x.", EZCONN_STATE_APP_CONNECTED);

    ez_int32_t ret = EZCONN_SUCC;

    ret = ezconn_process_http_req(REQ_DEVINFO, req, &g_conn_ctx);
    if (EZCONN_SUCC != ret)
    {
        ezlog_e(TAG_AP, "get device info failed.");
    }
    else
    {
        ezlog_v(TAG_AP, "get dev_info success.");
    }
    g_server_busy = ez_false;
    return ret;
}

ez_err_t http_get_point_handler(httpd_req_t *req)
{
    while (g_server_busy)
    {
        ezos_delay_ms(200);

        ezlog_i(TAG_AP, "http server is busy.");
        if (g_exit_flag)
        {
            ezlog_i(TAG_AP, "get ap point handler exit.");
            return EZCONN_SUCC;
        }
    }
    g_server_busy = ez_true;

    ez_int32_t ret = ezconn_process_http_req(REQ_GET_LIST, req, &g_conn_ctx);
    if (EZCONN_SUCC != ret)
    {
        ezlog_e(TAG_AP, "get wifi list failed.");
    }
    else
    {
        ezlog_v(TAG_AP, "get wifi list success.");
    }

    g_server_busy = ez_false;
    return ret;
}

ez_err_t http_put_wifi_handler(httpd_req_t *req)
{
    if (g_exit_flag)
    {
        return EZCONN_SUCC;
    }
    while (g_server_busy)
    {
        ezos_delay_ms(200);

        ezlog_i(TAG_AP, "http server is busy.");
        if (g_exit_flag)
        {
            ezlog_i(TAG_AP, "put wifi handler exit.");

            return EZCONN_SUCC;
        }
    }
    g_server_busy = ez_true;

    ezconn_process_http_req(REQ_WIFI_CONFIG, req, &g_conn_ctx);

    ezlog_i(TAG_AP, "wifi config end.");
    return EZCONN_SUCC;
}

httpd_uri_t get_dev_info = {
    .uri = "/AccessDevInfo",
    .method = HTTP_GET,
    .handler = http_get_devinfo_handler,
    .user_ctx = NULL};

httpd_uri_t get_access_point = {
    .uri = "/PreNetwork/SecurityAndAccessPoint",
    .method = HTTP_GET,
    .handler = http_get_point_handler,
    .user_ctx = NULL};

httpd_uri_t put_wifi_config = {
    .uri = "/PreNetwork/WifiConfig",
    .method = HTTP_PUT,
    .handler = http_put_wifi_handler,
    .user_ctx = NULL};


static void* start_httpd(void)
{
    httpd_handle_t tmp_server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8 * 1024;
    config.max_open_sockets = 2;
    config.backlog_conn = 2;
    config.max_uri_handlers = 3;
    config.lru_purge_enable = ez_true;

    do
    {
        if (EZCONN_SUCC != httpd_start(&tmp_server, &config))
        {
            ezlog_e(TAG_AP, "httpd_start error");
            tmp_server = NULL;
            break;
        }

        if (EZCONN_SUCC != httpd_register_uri_handler(tmp_server, &get_dev_info))
        {
            ezlog_e(TAG_AP, "register GET dev info handler error.");
            tmp_server = NULL;
            stop_httpd();
            break;
        }

        if (EZCONN_SUCC != httpd_register_uri_handler(tmp_server, &get_access_point))
        {
            ezlog_e(TAG_AP, "register GET wifi point handler error.");
            tmp_server = NULL;
            stop_httpd();
            break;
        }
        if (EZCONN_SUCC != httpd_register_uri_handler(tmp_server, &put_wifi_config))
        {
            ezlog_e(TAG_AP, "register PUT wifi config handler error.");
            tmp_server = NULL;
            stop_httpd();
            break;
        }
        
    } while (0);

    return tmp_server;
}

static void time_out_cb(void)
{
    if (!g_exit_flag)
    {
        ezlog_w(TAG_AP, "Wifi config time out, stop now.");
        g_exit_flag = ez_true;
        g_conn_ctx.wifi_cb(EZCONN_STATE_WIFI_CONFIG_TIMEOUT, NULL);

        ezlog_i(TAG_AP, "wifi config timeout. err_code:0x%x.", EZCONN_STATE_WIFI_CONFIG_TIMEOUT);
    }

    return;
}

int32_t ezconn_adatper_init(ezconn_ap_info_t *ap_info, ezconn_dev_info_t *dev_info, wifi_info_cb cb)
{
    if (!g_exit_flag)
    {
        ezlog_w(TAG_AP, "ap already inited");
        return EZCONN_ERRNO_AP_ALREADY_INIT;
    }

    if (NULL == ap_info || NULL == dev_info || NULL == cb)
    {
        ezlog_e(TAG_AP, "%s param error.", __FUNCTION__);
        return EZCONN_ERRNO_INVALID_PARAM;
    }

    if (0 == ezos_strlen(dev_info->dev_serial))
    {
        ezlog_e(TAG_AP, "dev_serial error.");
        return EZCONN_ERRNO_INVALID_PARAM;
    }

    if (ap_info->ap_timeout < MIN_TIME_OUT || ap_info->ap_timeout > MAX_TIME_OUT)
    {
        ap_info->ap_timeout = DEFAULT_TIME_OUT;
    }
    ezlog_w(TAG_AP, "ap time out after %d s.", ap_info->ap_timeout * 60);

    int32_t ret = EZCONN_SUCC;
    g_exit_flag = ez_false;
    g_server_busy = ez_false;
    do 
    {
        g_conn_ctx.time_out_timer = eztimer_create("ap_timeout", (ap_info->ap_timeout * 60 * 1000), ez_false, time_out_cb);
        if (NULL == g_conn_ctx.time_out_timer)
        {
            ezlog_e(TAG_AP, "ap timeout timer create failed.");
            ret = EZCONN_ERRNO_INTERNAL;
            break;
        }

        ezos_memcpy(&g_conn_ctx.dev_info, dev_info, sizeof(ezconn_dev_info_t));

        if (0 == ezos_strlen(ap_info->ap_ssid))
        {
            ezlog_e(TAG_AP, "ap ssid NULL.");
            ret = EZCONN_ERRNO_INVALID_PARAM;
            break;
        }

        if (0 == ezos_strlen(ap_info->ap_pwd))
        {
            ezlog_w(TAG_AP, "ap_ssid: %s, ap_password: NULL", ap_info->ap_ssid);
        }
        else
        {
            ezlog_w(TAG_AP, "ap_ssid: %s", ap_info->ap_ssid);
            ezlog_v(TAG_AP, "ap pwd: %s", ap_info->ap_pwd);
        }

        ezlog_w(TAG_AP, "ap auth_mode: %d", ap_info->auth_mode);

        ezos_memcpy(&g_conn_ctx.wifi_info, ap_info, sizeof(ezconn_ap_info_t));

        ret = ezos_ap_start(ap_info->ap_ssid, ap_info->ap_pwd, ap_info->auth_mode, ap_info->channel);
        if (0 != ret)
        {
            ezlog_e(TAG_AP, "ap start failed.");
            ret = EZCONN_ERRNO_AP_START_FAILED;
            break;
        }

        g_conn_ctx.httpd_server = start_httpd();
        if (NULL == g_conn_ctx.httpd_server)
        {
            ezlog_e(TAG_AP, "start httpd failed.");
            ret = EZCONN_ERRNO_HTTPD_START_FAILED;
            break;
        }
    } while (ez_false);
    
    if (EZCONN_SUCC != ret)
    {
        g_exit_flag = ez_true;
        eztimer_delete(g_conn_ctx.time_out_timer);
        ezos_memset(&g_conn_ctx, 0, sizeof(ezconn_ctx_t));
    }
    else
    {
        ezlog_w(TAG_AP, "ap start success");
    }

    return ret;
}

int32_t ezconn_adapter_deinit(void)
{
    stop_httpd();
    eztimer_delete(g_conn_ctx.time_out_timer);
    ezos_memset(&g_conn_ctx, 0, sizeof(ezconn_ctx_t));
    g_exit_flag = ez_true;
    return 0;
}
