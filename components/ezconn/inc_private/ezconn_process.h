#ifndef EZCONN_PROCESS_H
#define EZCONN_PROCESS_H

#include "ezos_def.h"
#include "ezconn_adapter.h"
#include "http_server.h"

typedef enum
{
    REQ_DEVINFO,
    REQ_GET_LIST,
    REQ_WIFI_CONFIG,
} http_req_type_e;

ez_int32_t ezconn_process_http_req(http_req_type_e req_type, httpd_req_t *req, ezconn_ctx_t *ctx);

#endif
