#ifndef EZCONN_ADAPTER_H
#define EZCONN_ADAPTER_H

#include "ezos_def.h"
#include "http_server.h"
#include "ezconn.h"

typedef struct
{
    ez_int32_t          time_out_timer;
    httpd_handle_t      httpd_server;
    wifi_info_cb        wifi_cb;
    ezconn_dev_info_t   dev_info;
    ezconn_wifi_info_t  wifi_info;
    ez_bool_t           apsta_coexist;
} ezconn_ctx_t;

ez_int32_t ezconn_adatper_init(ezconn_ap_info_t *ap_info, ezconn_dev_info_t *dev_info, wifi_info_cb cb);

ez_int32_t ezconn_adapter_deinit(void);

ez_int32_t ezconn_set_busy_state(ez_bool_t is_busy);

ez_int32_t ezconn_set_exit_state(ez_bool_t is_exit);

#endif // !EZCONN_ADAPTER_H
