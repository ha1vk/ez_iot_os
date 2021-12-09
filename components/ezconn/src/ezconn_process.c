#include "ezconn_process.h"
#include "ezconn.h"

#include "cJSON.h"
#include "ezos_wifi.h"
#include "ezlog.h"
#include "eztimer.h"

static ez_int32_t send_http_resp(httpd_req_t *req, ez_int32_t errcode, ez_char_t *body)
{
    ezlog_v(TAG_AP, "send response. errcode: %d", errcode);
    if (NULL != body)
    {
        ezlog_v(TAG_AP, "http body: %s", body);
    }
    switch (errcode)
    {
    case EZCONN_SUCC:
        httpd_resp_set_status(req, HTTPD_200);
        httpd_resp_set_type(req, HTTPD_TYPE_JSON);
        if (NULL != body) 
        {
            httpd_resp_send(req, body, ezos_strlen(body));
            ezlog_d(TAG_AP, "rsp http body: %s", body);
        }
        break;
    default:
        httpd_resp_set_status(req, HTTPD_500);
        httpd_resp_set_type(req, HTTPD_TYPE_JSON);
        break;
    }

    return EZCONN_SUCC;
}

static ez_int32_t process_get_devinfo_req(httpd_req_t *req, ezconn_ctx_t *ctx)
{
    ez_int32_t ret = EZCONN_SUCC;

    cJSON *js_root = NULL;
    char *js_str = NULL;
    do
    {
        js_root = cJSON_CreateObject();
        if (NULL == js_root)
        {
            ezlog_e(TAG_AP, "json create object error.");
            ret = EZCONN_ERRNO_NO_MEMORY;
            break;
        }

        cJSON_AddStringToObject(js_root, "ap_version", "1.0");
        cJSON_AddStringToObject(js_root, "dev_subserial", ctx->dev_info.dev_serial);
        cJSON_AddStringToObject(js_root, "dev_type", ctx->dev_info.dev_type);
        cJSON_AddStringToObject(js_root, "dev_firmwareversion", ctx->dev_info.dev_version);

        js_str = cJSON_PrintUnformatted(js_root);
        if (NULL == js_str)
        {
            ezlog_e(TAG_AP, "json print error.");
            ret = EZCONN_ERRNO_NO_MEMORY;
            break;
        }
    } while (ez_false);

    send_http_resp(req, ret, (ez_char_t *)js_str);

    if (NULL != js_root)
    {
        cJSON_Delete(js_root);
    }
    if (NULL != js_str)
    {
        ezos_free(js_str);
    }

    return ret;
}

static const char *security_mode[] = {
    "open",
    "WEP",
    "WPA-personal",
    "WPA2-personal",
    "WPA-WPA2-personal",
    "WPA2-enterprise"};

static ez_int32_t process_get_list_req(httpd_req_t *req, ezconn_ctx_t *ctx)
{
    ez_int32_t ret = EZCONN_SUCC;
    cJSON *js_root = NULL;
    char *js_str = NULL;
    do 
    {
        ezos_wifi_list_t ap_list[20] = {0};
        ez_uint8_t list_num = ezos_sta_get_scan_list(20, ap_list);
        if (0 == list_num)
        {
            ezlog_e(TAG_AP, "get list failed.");
            ret = EZCONN_ERRNO_GET_LIST_FAILED;
            break;
        }
        
        js_root = cJSON_CreateObject();
        if (NULL == js_root)
        {
            ezlog_e(TAG_AP, "json creat");
            ret = EZCONN_ERRNO_NO_MEMORY;
            break;
        }
        cJSON *js_list = cJSON_CreateArray();
        if (NULL == js_list)
        {
            ezlog_e(TAG_AP, "json creat");
            ret = EZCONN_ERRNO_NO_MEMORY;
            break;
        }
    
        cJSON_AddItemToObject(js_root, "access_point_list", js_list);
        int32_t tmp_num = list_num > 20 ? 20 : list_num;
        for (int32_t i = 0; i < tmp_num; i++)
        {
            cJSON *js_info = cJSON_CreateObject();
            if (NULL == js_info)
            {
                continue;
            }
            cJSON_AddStringToObject(js_info, "ssid", (char *)ap_list[i].ssid);
            cJSON_AddNumberToObject(js_info, "signal_strength", ap_list[i].rssi);
            cJSON_AddStringToObject(js_info, "security_mode", security_mode[ap_list[i].authmode]);
            cJSON_AddItemToArray(js_list, js_info);
        }
        js_str = cJSON_PrintUnformatted(js_root);
        if (NULL == js_str)
        {
            ezlog_e(TAG_AP, "json print");
            ret = EZCONN_ERRNO_NO_MEMORY;
            break;
        }

    } while (ez_false);

    send_http_resp(req, ret, (ez_char_t *)js_str);

    if (NULL != js_root)
    {
        cJSON_Delete(js_root);
    }
    if (NULL != js_str)
    {
        ezos_free(js_str);
    }

    return ret;
}

static ez_char_t *get_err_str(ezos_wifi_state_e state)
{
    switch (state)
    {
    case EZOS_WIFI_STATE_NOT_CONNECT:
        return "wifi not connect.";
    case EZOS_WIFI_STATE_CONNECT_SUCCESS:
        return "wifi connect success.";
    case EZOS_WIFI_STATE_PASSWORD_ERROR:
        return "wifi password error.";
    case EZOS_WIFI_STATE_NO_AP_FOUND:
        return "wifi no ap found.";
    default:
        return "unknown error.";
    }
}

static ez_int32_t gen_rsp_with_state(ezos_wifi_state_e state, ez_char_t **rsp_str)
{
    ez_int32_t ret = 0;
    cJSON *js_root = NULL;
    do 
    {
        js_root = cJSON_CreateObject();
        if (NULL == js_root)
        {
            ezlog_e(TAG_AP, "json creat");
            ret = EZCONN_ERRNO_NO_MEMORY;
            break;
        }
        cJSON_AddNumberToObject(js_root, "status_code", state);
        cJSON_AddStringToObject(js_root, "status_string", get_err_str(state));

        *rsp_str = cJSON_PrintUnformatted(js_root);
        if (NULL == *rsp_str)
        {
            ezlog_e(TAG_AP, "json print");
            ret = EZCONN_ERRNO_NO_MEMORY;
            break;
        }
    } while (ez_false);
    if (NULL != js_root)
    {
        cJSON_Delete(js_root);
    }
    return ret;
}

static ez_int32_t process_wifi_config_req(const char *req_content, ezconn_ctx_t *ctx)
{
    ez_int32_t ret = EZCONN_SUCC;
    
    cJSON *js_root = NULL;
    do 
    {
        js_root = cJSON_Parse(req_content);
        if (NULL == js_root)
        {
            ezlog_e(TAG_AP, "json parse");
            ret = EZCONN_ERRNO_JSON_PARSE;
            break;
        }

        cJSON *js_cc = cJSON_GetObjectItem(js_root, "areaCode");
        if (NULL != js_cc)
        {
            ezos_strncpy(ctx->wifi_info.cc, js_cc->valuestring, sizeof(ctx->wifi_info.cc) - 1);
            ctx->wifi_info.cc[3] = '\0';
        }

        cJSON *js_token = cJSON_GetObjectItem(js_root, "token");
        if (NULL == js_token)
        {
            ezlog_e(TAG_AP, "token absent");
            ret = EZCONN_ERRNO_PARAM_ERROR;
            break;
        }
        ezos_strncpy(ctx->wifi_info.token, js_token->valuestring, sizeof(ctx->wifi_info.token) - 1);

        cJSON *js_domain = cJSON_GetObjectItem(js_root, "lbs_domain");
        if (NULL == js_domain)
        {
            ezlog_e(TAG_AP, "domain absent");
            ret = EZCONN_ERRNO_PARAM_ERROR;
            break;
        }
        ezos_strncpy(ctx->wifi_info.domain, js_domain->valuestring, sizeof(ctx->wifi_info.domain) - 1);

        cJSON *js_devid = cJSON_GetObjectItem(js_root, "device_id");
        if (NULL != js_devid)
        {
            ezos_strncpy(ctx->wifi_info.device_id, js_devid->valuestring, sizeof(ctx->wifi_info.device_id) - 1);
        }
        
        cJSON *js_wifi_info = cJSON_GetObjectItem(js_root, "wifi_info");
        if (NULL == js_wifi_info)
        {
            ezlog_e(TAG_AP, "wifi_info absent");
            ret = EZCONN_ERRNO_PARAM_ERROR;
            break;
        }

        cJSON *js_ssid = cJSON_GetObjectItem(js_wifi_info, "ssid");
        if (NULL == js_ssid)
        {
            ezlog_e(TAG_AP, "ssid absent");
            ret = EZCONN_ERRNO_PARAM_ERROR;
            break;
        }
        ezos_strncpy(ctx->wifi_info.ssid, js_ssid->valuestring, sizeof(ctx->wifi_info.ssid) - 1);

        cJSON *js_pwd = cJSON_GetObjectItem(js_wifi_info, "password");
        if (NULL == js_pwd)
        {
            ezlog_e(TAG_AP, "password absent");
            ret = EZCONN_ERRNO_PARAM_ERROR;
            break;
        }
        ezos_strncpy(ctx->wifi_info.password, js_pwd->valuestring, sizeof(ctx->wifi_info.password) - 1);

    } while (ez_false);

    if (NULL != js_root)
    {
        cJSON_Delete(js_root);
    }

    return ret;
}

static ez_int32_t process_wifi_config(httpd_req_t *req, ezconn_ctx_t *ctx)
{
    ez_int32_t ret = EZCONN_SUCC; 

    char *req_content = NULL;
    ez_char_t *rsp_str = NULL;
    do 
    {
        req_content = (char *)ezos_malloc(req->content_len);
        if (NULL == req_content)
        {
            ezlog_e(TAG_AP, "ezos_malloc failed");
            ret = EZCONN_ERRNO_NO_MEMORY;
            break;
        }

        int remain_len = req->content_len;
        while (remain_len > 0)
        {
            if ((ret = httpd_req_recv(req, req_content, remain_len)) <= 0)
            {
                if (ret == HTTPD_SOCK_ERR_TIMEOUT)
                {
                    continue;
                }
                break;
            }
            remain_len -= ret;
        }
        ezlog_d(TAG_AP, "req.httpbody: %s", req_content);
        ret = process_wifi_config_req(req_content, ctx);
        if (EZCONN_SUCC != ret)
        {
            ezlog_e(TAG_AP, "wifi req process failed.");
            break;
        }

        if (!ctx->apsta_coexist)
        {
            gen_rsp_with_state(EZOS_WIFI_STATE_CONNECT_SUCCESS, &rsp_str);
            send_http_resp(req, EZCONN_SUCC, rsp_str);
            if (NULL != rsp_str)
            {
                ezos_free(rsp_str);
            }
        }
       
        ezos_sta_connect(ctx->wifi_info.ssid, ctx->wifi_info.password);

        ctx->wifi_cb(EZCONN_STATE_CONNECTING_ROUTE, NULL);
        ezlog_i(TAG_AP, "conneting route...");
        
        ezos_wifi_state_e wifi_state = EZOS_WIFI_STATE_UNKNOW;
        for (int i = 0; i < 100; i++)
        {
            if (ctx->apsta_coexist)
            {
                ezos_delay_ms(100);
            }
            else
            {
                ezos_delay_ms(300);
            }
            wifi_state = ezos_get_state();
            if (EZOS_WIFI_STATE_CONNECT_SUCCESS == wifi_state)
            {
                break;
            }
        }

        ez_bool_t is_succ = ez_true;
        if (wifi_state != EZOS_WIFI_STATE_CONNECT_SUCCESS)
        {
            ezos_sta_stop(); // 若超时之后还未连接成功，则停止当前连接
            is_succ = ez_false;
        }
        
        // 若wifi已经连接，则需要app重新连接ap，再发送http response
        if (ctx->apsta_coexist)
        {
            gen_rsp_with_state(wifi_state, &rsp_str);
            send_http_resp(req, EZCONN_SUCC, rsp_str);

            if (wifi_state == EZOS_WIFI_STATE_CONNECT_SUCCESS)
            {
                for (int i = 0; i < 100; i++)
                {
                    wifi_state = ezos_get_state();
                    if (wifi_state == EZOS_WIFI_STATE_STA_CONNECTED)
                    {
                        send_http_resp(req, EZCONN_SUCC, rsp_str);
                        ezos_delay_ms(3000);
                        break;
                    }
                    ezos_delay_ms(100);
                }
            }
            else // 若wifi未连接，则不需要等ap重新连接
            {
                ezos_delay_ms(3000);
            }
        }

        if (is_succ)
        {
            ezlog_w(TAG_AP, "wifi config success.");
            ctx->wifi_cb(EZCONN_STATE_SUCC, &ctx->wifi_info);
            ezconn_set_busy_state(ez_true);
            ezconn_set_exit_state(ez_true);

            if (NULL != ctx->time_out_timer)
            {
                eztimer_change_period(ctx->time_out_timer, 1000);
            }
        }
        else
        {
            ezlog_w(TAG_AP, "wifi config err code: %d", wifi_state);
            ctx->wifi_cb(EZCONN_STATE_CONNECT_FAILED, NULL);
            ezconn_set_busy_state(ez_false);
            if (!ctx->apsta_coexist)
            {
                ezconn_set_exit_state(ez_true);
                if (NULL != ctx->time_out_timer)
                {
                    eztimer_change_period(ctx->time_out_timer, 1000);
                }

            }
            else
            {
                if (NULL != ctx->time_out_timer)
                {
                    eztimer_reset(ctx->time_out_timer);
                }
            }
        }

    } while (ez_false);
    
    if (NULL != req_content)
    {
        ezos_free(req_content);
    }
    if (NULL != rsp_str)
    {
        ezos_free(rsp_str);
    }
    return ret;
}

ez_int32_t ezconn_process_http_req(http_req_type_e req_type, httpd_req_t *req, ezconn_ctx_t *ctx)
{
    ez_int32_t ret = EZCONN_SUCC;
    ezlog_v(TAG_AP, "process http req, req_type: %d", req_type);
	switch (req_type)
	{
	case REQ_DEVINFO:
        ret = process_get_devinfo_req(req, ctx);
		break;
	case REQ_GET_LIST:
        ret = process_get_list_req(req, ctx);
		break;
	case REQ_WIFI_CONFIG:
        ret = process_wifi_config(req, ctx);
		break;
	default:
		break;
	}
    return ret;
}
