#include "ez_iot_shadow_extern.h"
#include "ez_iot_shadow.h"
#include "ez_iot_shadow_def.h"
#include "ez_iot_shadow_core.h"
#include "ez_iot_core_lowlvl.h"
#include "ez_iot_core_def.h"
#include "ezlog.h"

static ez_void_t iot_core_event_route(ez_kernel_event_t *ptr_event);
static ez_void_t shadow_data_route_cb(ez_kernel_submsg_v3_t *psub_msg);

ez_err_t shadow_extern_init(ez_void_t)
{
    ez_err_t rv = EZ_SHD_ERR_SUCC;

    ez_kernel_extend_v3_t extend_info = {0};
    extend_info.ez_kernel_event_route = iot_core_event_route;
    extend_info.ez_kernel_data_route = shadow_data_route_cb;
    ezos_strncpy(extend_info.module, SHADOW_MODULE_NAME, sizeof(extend_info.module) - 1);

    rv = ez_kernel_extend_load_v3(&extend_info);
    CHECK_COND_DONE(EZ_CORE_ERR_NOT_INIT == rv, EZ_SHD_ERR_NOT_READY);
    CHECK_COND_DONE(EZ_CORE_ERR_MEMORY == rv, EZ_SHD_ERR_MEMORY);
    CHECK_COND_DONE(EZ_CORE_ERR_SUCC != rv, EZ_SHD_ERR_GENERAL);

done:

    return rv;
}

ez_void_t shadow_extern_deini(ez_void_t)
{
}

static ez_void_t iot_core_event_route(ez_kernel_event_t *ptr_event)
{
    ez_kernel_publish_ack_t *pack_ctx = NULL;

    if (!ptr_event)
    {
        return;
    }

    switch (ptr_event->event_type)
    {
    case SDK_KERNEL_EVENT_ONLINE:
        shadow_core_event_occured(SHADOW_EVENT_TYPE_ONLINE);
        break;
    case SDK_KERNEL_EVENT_SWITCHOVER:
        shadow_core_event_occured(SHADOW_EVENT_TYPE_RESET);
        break;
    case SDK_KERNEL_EVENT_BREAK:
        shadow_core_event_occured(SHADOW_EVENT_TYPE_OFFLINE);
        break;
    case SDK_KERNEL_EVENT_RECONNECT:
        shadow_core_event_occured(SHADOW_EVENT_TYPE_ONLINE);
        break;
    case SDK_KERNEL_EVENT_PUBLISH_ACK:
        pack_ctx = (ez_kernel_publish_ack_t *)ptr_event->event_context;
        if (0 == ezos_strcmp(pack_ctx->module_name, SHADOW_MODULE_NAME))
        {
            ezlog_v(TAG_SHD, "publish ack, module mismatch");
            return;
        }

        shadow_core_cloud_data_in(NULL, pack_ctx->msg_seq, NULL, (ez_void_t *)&pack_ctx->last_error);

    default:
        break;
    }
}

static ez_void_t shadow_data_route_cb(ez_kernel_submsg_v3_t *psub_msg)
{
    ez_shadow_res_t shadow_res = {0};

    if (0 != ezos_strcmp(psub_msg->method, SHADOW_METHOD_NAME))
    {
        ezlog_e(TAG_SHD, "method mismatch");
        return;
    }

    ezos_strncpy(shadow_res.dev_serial, psub_msg->sub_serial, sizeof(shadow_res.dev_serial) - 1);
    ezos_strncpy(shadow_res.res_type, psub_msg->resource_type, sizeof(shadow_res.res_type) - 1);
    shadow_res.local_index = ezos_atoi(psub_msg->resource_id);

    if (0 == ezos_strlen(psub_msg->sub_serial))
    {
        ezos_strncpy(shadow_res.dev_serial, SHADOW_DEFAULT_NAME, sizeof(shadow_res.dev_serial) - 1);
    }

    shadow_core_cloud_data_in((void *)&shadow_res, psub_msg->msg_seq, psub_msg->msg_type, psub_msg->buf);
}
