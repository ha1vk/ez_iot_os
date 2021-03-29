#include "ez_base_api.h"
#include "ez_base_def.h"
#include "ezdev_sdk_kernel.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ez_sdk_log.h"
#include "das_data_handle.h"

static int g_inited = 0;

EZ_BASE_API ez_base_err ez_base_init(const ez_base_init_t *pinit)
{
    int ret = 0;
    ez_base_err err = EZ_SUCCESS;
    ezdev_sdk_kernel_extend extern_info;
    do
    {
        if (0 != g_inited)
        {
            err = EZ_INITED;
            break;
        }
        if (!pinit||NULL == pinit->cb.recv_msg)
        {
            err = EZ_INVALID_PARAM;
            break;
        }
        memset(&extern_info, 0, sizeof(ezdev_sdk_kernel_extend));

        extern_info.domain_id = ez_base_module_id;
        extern_info.ezdev_sdk_kernel_extend_start = extend_start_cb;
        extern_info.ezdev_sdk_kernel_extend_stop = extend_stop_cb;
        extern_info.ezdev_sdk_kernel_extend_data_route = extend_data_route_cb;
        extern_info.ezdev_sdk_kernel_extend_event = extend_event_cb;
        strncpy(extern_info.extend_module_name, ez_base_module_name, ezdev_sdk_extend_name_len -1);
        strncpy(extern_info.extend_module_version, ez_base_module_version, ezdev_sdk_extend_name_len-1);
        ret = ezdev_sdk_kernel_extend_load(&extern_info);
        if(0 != ret)
        {
            err = EZ_REG_ERR;
            break;
        }

        base_set_cb(pinit->cb);

        g_inited = 1;

    } while (0);

    return err;
}

EZ_BASE_API ez_base_err ez_base_deinit()
{

    if (!g_inited)
    {
        return EZ_NOT_INITED;
    }

    g_inited = 0;

    base_set_operation_code("", 0);

    ez_log_v(TAG_BASE,"ez_base_deinit end\n");

    return EZ_SUCCESS;
}

EZ_BASE_API ez_base_err ez_base_send_msg(const unsigned char* buf, const unsigned int len, const int cmd_id, ez_attr_t* msg_attr)
{
    int ret = 0;
    ez_base_err err  = EZ_SUCCESS;
    do
    {
        if (!g_inited)
        {
            err = EZ_NOT_INITED;
            break;
        }
        if(NULL == buf|| 0 == len||NULL == msg_attr)
        {
            err = EZ_INVALID_PARAM;
            break;
        }
        ret = ez_send_msg2plat((unsigned char*)buf, len, cmd_id, ez_base_cmd_version, msg_attr->msg_type, &msg_attr->msg_seq);
        if(0!=ret)
        {
            err = EZ_FAILED; 
            break;
        }
    } while (0);

    return err;
}

EZ_BASE_API ez_base_err ez_base_set_operation_code(const char *pcode, const int len)
{
    int ret = 0;
    ez_base_err err  = EZ_SUCCESS;

    ret  = base_set_operation_code(pcode, len);
    if(0!=ret)
    {
       err = EZ_INVALID_PARAM; 
    }
    return err;
}

EZ_BASE_API ez_base_err ez_base_query_userid()
{
    int ret = 0;
    ez_base_err err  = EZ_SUCCESS;

    ret  = pu2plt_query_userid_req();
    if(0!=ret)
    {
       err = EZ_FAILED; 
    }
    
    return err;
}