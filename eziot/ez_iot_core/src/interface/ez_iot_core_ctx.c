#include "ez_iot_core.h"
#include <ezlog.h>
#include "ez_iot_core_ctx.h"
#include "ezdev_sdk_kernel_struct.h"

static ez_char_t *g_devid = NULL;
static ez_event_notice g_event_notice_func = NULL;
ezdev_sdk_kernel g_ezdev_sdk_kernel;

ez_void_t ez_iot_event_adapt(ez_kernel_event_t *ptr_event)
{
    if (!ptr_event || !g_event_notice_func)
    {
        return;
    }

    ezlog_v(TAG_CORE, "event t:%d", ptr_event->event_type);

    switch (ptr_event->event_type)
    {
    case SDK_KERNEL_EVENT_ONLINE:
        g_event_notice_func(EZ_EVENT_ONLINE, NULL, 0);
        break;
    case SDK_KERNEL_EVENT_BREAK:
        g_event_notice_func(EZ_EVENT_OFFLINE, NULL, 0);
        break;
    case SDK_KERNEL_EVENT_SWITCHOVER:
    {
        ez_kernel_switchover_context_t *pctx = (ez_kernel_switchover_context_t *)ptr_event->event_context;
        g_event_notice_func(EZ_EVENT_ONLINE, NULL, 0);
        if (NULL != pctx)
        {
            g_event_notice_func(EZ_EVENT_SERVER_UPDATE, pctx->lbs_domain, ezos_strlen((const char *)pctx->lbs_domain));
        }
        break;
    }
    case SDK_KERNEL_EVENT_RECONNECT:
        g_event_notice_func(EZ_EVENT_RECONNECT, NULL, 0);
        break;

    default:
        break;
    }
}

ez_int32_t ez_iot_value_save(sdk_keyvalue_type valuetype, ez_uchar_t *keyvalue, ez_int32_t keyvalue_size)
{
    ez_int32_t rv = -1;

    if (valuetype == sdk_keyvalue_devid)
    {
        rv = ezos_kv_raw_set(EZ_KV_DEFALUT_KEY_DEVID, keyvalue, keyvalue_size);
        // rv = g_event_notice_func(EZ_EVENT_DEVID_UPDATE, keyvalue, keyvalue_size);
    }
    else if (valuetype == sdk_keyvalue_masterkey)
    {
        rv = ezos_kv_raw_set(EZ_KV_DEFALUT_KEY_MASTERKEY, keyvalue, keyvalue_size);
    }

    return rv;
}

void ez_iot_event_notice_set(const ez_event_notice pfunc)
{
    g_event_notice_func = pfunc;
}

const ez_event_notice ez_iot_event_notice_get(ez_void_t)
{
    return g_event_notice_func;
}

void ez_iot_devid_set(const ez_char_t *devid)
{
    static ez_char_t m_devid[32 + 1] = {0};

    ezos_strncpy(m_devid, devid, sizeof(m_devid) - 1);
    g_devid = m_devid;
}

const ez_char_t *ez_iot_devid_get(ez_void_t)
{
    return g_devid;
}