#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ez_iot_tsl.h"
#include "ezlog.h"
#include "ezos_def.h"


//#include "ezcloud_tsl.h"
//#include "ez_iot_ota.h"

#include "ezcloud_access.h"
#include "hub_mgr.h"

//#include "ez_hal/hal_thread.h"


static ez_int32_t tsl_things_action2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info,
                                     const ez_tsl_value_t *value_in, ez_tsl_value_t *value_out)
{
    EZ_INT ret=0;
    value_out->size = 1;
    value_out->type = EZ_TSL_DATA_TYPE_INT;
    value_out->value_int = 0;
        
    if (NULL == key_info || NULL == value_in )
    {
       ezlog_e(TAG_AP, "things report2dev param error.");
       return -1;
    }
        
    if( 0 == strcmp("global", sn))//网关操作   网关的sn是global
    {
        ezlog_i(TAG_AP,"%s %d, res_type = %s ,local_index = %s\n", __FUNCTION__, __LINE__,rsc_info->res_type,rsc_info->local_index);
        ezlog_i(TAG_AP,"%s %d, domain = %s ,key = %s\n", __FUNCTION__, __LINE__,key_info->domain,key_info->key);
        ezlog_i(TAG_AP,"%s %d, type = %d ,size = %d\n", __FUNCTION__, __LINE__,value_in->type,value_in->size);
        ezlog_i(TAG_AP,"%s %d, PID = %s\n", __FUNCTION__, __LINE__,value_in->value);
    
    /*     截取数据示例
            sn= global
            res_type = global ,local_index = 0
            domain = SubDevice ,key = openAdd
            type = 5 ,size = 61
            PID = {"addType":"1","uuid":"805103502289","subCat":"human-sensor"}
    */
        if( 0 == strcmp("openAdd", key_info->key)) //添加子设备
        {
            open_add_subdev(value_in->value);//打开添加子设备窗口
        }
        else if( 0 == strcmp("closeAdd", key_info->key))//关闭添加子设备
        {
            close_add_subdev();
        }
    }
    else //子设备操作
    {
    }
    
    return ret;

}

static ez_int32_t tsl_things_property2cloud(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, ez_tsl_value_t *value_out)
{
    return -1;
}

static ez_int32_t tsl_things_property2dev(const ez_char_t *sn, const ez_tsl_rsc_t *rsc_info, const ez_tsl_key_t *key_info, const ez_tsl_value_t *value)
{
    return 1;
}


EZ_INT ez_tsl_init(void)
{
    ez_tsl_things_callbacks_t tsl_things_cbs = {tsl_things_action2dev, tsl_things_property2cloud, tsl_things_property2dev};
    return ez_iot_tsl_init(&tsl_things_cbs);
}

