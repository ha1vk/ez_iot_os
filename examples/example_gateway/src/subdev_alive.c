#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <stddef.h>

#include "ezos_def.h"
#include "ez_iot_core.h"
#include "ez_iot_hub.h"
#include "ezlog.h"
#include "ezos_kv.h"
#include "ez_iot_tsl.h"
#include "cJSON.h"
#include "hub_mgr.h"
#include "ezcloud_access.h"
#include "flashdb.h"
#include "kv_imp.h"


//#include "s2j/s2j.h"
//#include "s2j/s2jdef.h"



#define CHECK_COND_RETURN(cond, errcode) \
    if ((cond))                          \
    {                                    \
        return (errcode);                \
    }

#define CHECK_COND_DONE(cond, errcode) \
    if ((cond))                        \
    {                                  \
        rv = (errcode);                \
        goto done;                     \
    }
#define SAFE_FREE(p) \
    if (p)           \
    {                \
        free(p);     \
        p = NULL;    \
    }

#define SUBLIST_JSON_KEY_SN "subdev_sn"
#define SUBLIST_JSON_KEY_INTERVAL "alive_intervaltime"      //心跳间隔时间
#define SUBLIST_JSON_KEY_OVERTIME "subdev_overtime_s"       //连接超时时间

static int find_dev_by_sn(cJSON *json_obj, char *sn);



ez_int_t get_alivelist_root(cJSON *js_root) //从KV中获取alivelist的json root
{
    ez_err_t rv = -1;
    size_t length = 0;
    ez_char_t *pbuf = NULL;
    ez_char_t temp_data[10] = {0};
        
    ezos_kv_raw_get((const ez_int8_t *)EZ_SUBDEV_KEY_ALIVELIST, NULL, &length); //获取kv中alivelist的value的字符长度，
    
    if(length < 0 || length >= 0xffffffff) //获取alivelist失败，则创建alivelist
    {
        fdb_kv_set(&ez_kvdb,EZ_SUBDEV_KEY_ALIVELIST, temp_data);
        
        CHECK_COND_RETURN(ezos_kv_raw_get((const int8_t *)EZ_SUBDEV_KEY_ALIVELIST, NULL, &length), -1);
    }
        
    if (0 == length)  //无子设备，不作处理
    {
        ezlog_i(TAG_APP, "subdev is NULL");
        rv = -1;
    }
    else if(0 < length)
    {
        ezlog_i(TAG_APP, "KV read error");
        
        rv = -1;
    }
    else
    {
        pbuf = (char *)malloc(length + 1);
        CHECK_COND_RETURN(!pbuf, -1);

        /* 从kv中取出subdev_alivelist对应的value字符串    */
        CHECK_COND_RETURN(ezos_kv_raw_get((const ez_int8_t *)EZ_SUBDEV_KEY_ALIVELIST, (ez_int8_t *)pbuf, &length), -1);

        /* 将subdev_alivelist对应的value字符串  转化为JSON格式 */
        CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), -1);

        rv = EZ_HUB_ERR_SUCC;
     }

done:
    SAFE_FREE(pbuf);

    return rv;
}



ez_int_t save_alivelist_root(cJSON *js_root)//保存json root到KV的alivelist中
{
    ez_char_t *pbuf_save = NULL;

    /*转化为字符串*/
    CHECK_COND_RETURN(!(pbuf_save = cJSON_PrintUnformatted(js_root)), -1);
    
    /* 存入KV */
    CHECK_COND_RETURN(ezos_kv_raw_set((const ez_int8_t *)EZ_SUBDEV_KEY_ALIVELIST, (ez_int8_t *)pbuf_save, strlen(pbuf_save)), -1);

}

/**
 * @brief 上电时，对每个子设备的json数据初始化，初始化超时时间参数为0
 * 
 * @param info 
 * @return void 
 */
static ez_int_t every_subdev_init(cJSON *json_obj)
{
    for (ez_int_t i = 0; i < cJSON_GetArraySize(json_obj); i++)//遍历每一个子设备
    {
        cJSON *js_item = cJSON_GetArrayItem(json_obj, i);

        cJSON_ReplaceItemInObject(js_item,"subdev_overtime_s",cJSON_CreateNumber(0));  //上电初始化时，将超时时间置零

    }
    return 0;
}

/**
 * @brief 上电时，重置子设备的心跳超时时间  初始化超时时间为0
 * 
 * @param info 
 * @return void 
 */
EZ_INT subdev_alive_init(void)
{
    EZ_INT rv = -1;

    cJSON *js_root = NULL;
    
    if(get_alivelist_root(js_root) == 0)//从KV中获取子设备心跳的json结构体
    {
        /* 对每个子设备的心跳管理进行初始化 */
        CHECK_COND_DONE(every_subdev_init(js_root) == -1,-1);

        /* 存入KV */
        save_alivelist_root(js_root);
    }

done:
    if(!js_root)  {cJSON_Delete(js_root);}
    return rv;
}



static ez_int_t every_subdev_overtime_deal(cJSON *json_obj)
{
    ez_int_t rv = -1;

    hub_subdev_alive_t  buf_subdev_alive = {0};
    ez_subdev_info_t subdev_temp = {0};
    cJSON *js_item = NULL;
    cJSON *js_value =  NULL;
    
    for (ez_int_t i = 0; i <cJSON_GetArraySize(json_obj); i++)//遍历所有子设备
    {
        js_item = cJSON_GetArrayItem(json_obj, i);
        js_value = cJSON_GetObjectItem(js_item , "subdev_overtime_s"); 
        buf_subdev_alive.subdev_overtime_s = js_value->valueint;       //获取子设备 的 超时时间 计时  数值
        
        js_value = cJSON_GetObjectItem(js_item , "alive_intervaltime"); 
        buf_subdev_alive.alive_intervaltime = js_value->valueint;      //获取子设备  心跳间隔时间
        
        js_value = cJSON_GetObjectItem(js_item , "subdev_sn");
        strcpy(buf_subdev_alive.subdev_sn , js_value->valuestring);    //获取子设备 sn号

        //如子设备不存在   、或已掉线，则不作处理
        if(EZ_HUB_ERR_SUCC == ez_iot_hub_subdev_query(buf_subdev_alive.subdev_sn, &subdev_temp))
        {
            if(subdev_temp.sta == true)
            {
                buf_subdev_alive.subdev_overtime_s += SUBDEV_ALIVE_DEAL_INTERVALTIME;  //每次进入此循环，自增5秒超时时间， 
                                                                                       //超出3倍间隔时间，则设备离线
                //超时判断   超时时间 > 3 * subdev_overtime_s ,则认为设备离线 
                if(buf_subdev_alive.subdev_overtime_s  > (3* buf_subdev_alive.alive_intervaltime))
                {
                    ez_iot_hub_status_update(buf_subdev_alive.subdev_sn, false);      //子设备 离线处理
                }
                cJSON_ReplaceItemInObject(js_item,"subdev_overtime_s",cJSON_CreateNumber(buf_subdev_alive.subdev_overtime_s));
            }
        }
        
        rv = 0;
    }

    return rv;
}


ez_int_t subdev_alive_func(void)
{
    ez_int_t rv = -1;

    cJSON *js_root = NULL;
    
    if(get_alivelist_root(js_root) == 0)//从KV中获取子设备心跳的json结构体
    {
        /*对每个设备进行超时处理*/
        CHECK_COND_DONE(every_subdev_overtime_deal(js_root), -1);

        /* 存入KV */
        save_alivelist_root(js_root);
    }

done:
    if(!js_root)  {cJSON_Delete(js_root);}
    return rv;
}


static ez_int_t find_subdev_and_update_overtime(cJSON *json_obj,const ez_int8_t *subdev_sn,const ez_int8_t overtime)
{
    ez_int_t rv = -1;
    cJSON *js_item = NULL;

    for (ez_int_t i = 0; i < cJSON_GetArraySize(json_obj); i++) //遍历子设备
    {
        js_item = cJSON_GetArrayItem(json_obj, i);

        cJSON *svalue = cJSON_GetObjectItem(js_item , "subdev_sn"); //对比子设备SN号
        ez_int_t buf = strcmp(subdev_sn , svalue->valuestring);

        if(buf == 0)
        {
            cJSON_ReplaceItemInObject(js_item,"subdev_overtime_s",cJSON_CreateNumber(overtime));//更新超时时间

           //如设备已离线，则重新上线
           ez_subdev_info_t subdev_temp = {0};
           ez_iot_hub_subdev_query(subdev_sn, &subdev_temp);
        
           if(subdev_temp.sta == false)
            {
                ez_iot_hub_status_update(subdev_sn, true);//子设备 离线处理. 离线状态改为在线
            }
            return 0;
        }
        
    }
    return rv;
}

ez_int_t subdev_alive_update_overtime(const ez_char_t *subdev_sn,const ez_int8_t subdev_overtime_s)
{
    cJSON *js_root = NULL;

    ez_int_t rv = get_alivelist_root(js_root);//从KV中获取子设备心跳的json结构体
    if(rv == 0)
    {
        /* 对指定SN的设备进行超时处理 */
        CHECK_COND_DONE(find_subdev_and_update_overtime(js_root,subdev_sn,subdev_overtime_s), -1);   

        /* 存入KV */
        save_alivelist_root(js_root);
    }

done:
    if(!js_root)  {cJSON_Delete(js_root);}

    return rv;

}


ez_int_t subdev_alive_update_example(void)
{
    return subdev_alive_update_overtime(SUBDEV_TEST_SN ,0); //将SN号对应设备的超时时间  清零
}


EZ_INT ez_subdev_alive_del(const ez_char_t *subdev_sn)
{
    EZ_INT rv = 0;
    size_t length = 0;
    ez_char_t *pbuf = NULL;
    ez_char_t *pbuf_save = NULL;
    cJSON *js_root = NULL;
    ez_int_t index = -1;

    CHECK_COND_RETURN(ezos_kv_raw_get((const int8_t *)EZ_SUBDEV_KEY_ALIVELIST, NULL, &length), -1);
    CHECK_COND_RETURN(0 == length, -1);

    CHECK_COND_DONE(!(pbuf = (char *)malloc(length + 1)), -1);
    
    CHECK_COND_DONE(ezos_kv_raw_get((const int8_t *)EZ_SUBDEV_KEY_ALIVELIST, (int8_t *)pbuf, &length), -1);
    CHECK_COND_DONE(!(js_root = cJSON_Parse(pbuf)), -1);
    CHECK_COND_DONE(-1 == (index = find_dev_by_sn(js_root, (char *)subdev_sn)), -1);
    
    cJSON_DeleteItemFromArray(js_root, index);
    CHECK_COND_DONE(!(pbuf_save = cJSON_PrintUnformatted(js_root)), -1);
    CHECK_COND_DONE(ezos_kv_raw_set((const int8_t *)EZ_SUBDEV_KEY_ALIVELIST, (int8_t *)pbuf_save, strlen(pbuf_save)), -1);
    
done:
    SAFE_FREE(pbuf);
    if(!js_root)  {cJSON_Delete(js_root);}
    return rv;
    
}


static cJSON *subdev_alive_to_json(void *struct_obj)
{
    hub_subdev_alive_t *subdev_obj = (hub_subdev_alive_t *)struct_obj;
    
    cJSON *subdev_json = cJSON_CreateObject();
    
    CHECK_COND_RETURN(!subdev_obj, NULL);

    s2j_json_set_basic_element(subdev_json, subdev_obj, string, subdev_sn);

    s2j_json_set_basic_element(subdev_json, subdev_obj, int, alive_intervaltime);

    s2j_json_set_basic_element(subdev_json, subdev_obj, int, subdev_overtime_s);

    return subdev_json;
}

static int find_dev_by_sn(cJSON *json_obj, char *sn)
{
    ez_int_t index = -1;
    cJSON *js_item = NULL;
    cJSON *js_sn = NULL;

    for (ez_int_t i = 0; i < cJSON_GetArraySize(json_obj); i++)
    {
        js_item = cJSON_GetArrayItem(json_obj, i);

        if (NULL == js_item)
        {
            continue;
        }

        js_sn = cJSON_GetObjectItem(js_item, SUBLIST_JSON_KEY_SN);

        if (NULL == js_sn)
        {
            continue;
        }

        if (NULL == sn || 0 == strcmp(js_sn->valuestring, (char *)sn))
        {
            index = i;
            break;
        }
    }

    return index;
}

/**
 * @brief 添加子设备时，设置心跳管理默认参数,加入到kv
 * 
 * @param info 子设备信息
 * @return void 
 */
ez_int_t ez_subdev_alive_add(const ez_subdev_info_t *subdev_info)
{
    ez_int_t rv = -1;

    cJSON *js_root = NULL;
    cJSON *js_subdev = NULL;
	
    hub_subdev_alive_t subdev_alive_info ={0};

	CHECK_COND_RETURN(!subdev_info, EZ_HUB_ERR_PARAM_INVALID);

    /* 设置新添加子设备的心跳信息 */
    strcpy(subdev_alive_info.subdev_sn , subdev_info->subdev_sn);
    subdev_alive_info.subdev_overtime_s = 0;             //添加子设备时，初始超时时间为0
    subdev_alive_info.alive_intervaltime = 30;     //添加子设备时，默认心跳间隔时间为30秒
    
	get_alivelist_root(js_root);//从KV中获取子设备心跳的json结构体

	//查找子设备是否已被添加
    CHECK_COND_DONE(-1 != find_dev_by_sn(js_root, (char *)subdev_info->subdev_sn), EZ_HUB_ERR_SUBDEV_EXISTED);

	//查询子设备数量是否超上限
	CHECK_COND_DONE(cJSON_GetArraySize(js_root) >= COMPONENT_HUB_SUBLIST_MAX, EZ_HUB_ERR_OUT_OF_RANGE);

	//将子设备的心跳信息 转化为json格式
	CHECK_COND_DONE(!(js_subdev = subdev_alive_to_json((void *)&subdev_alive_info)), EZ_HUB_ERR_MEMORY);

	//将子设备添加到json array
	cJSON_AddItemToArray(js_root, js_subdev);
				
	/* 存入KV */
	save_alivelist_root(js_root);
	
	done:

    if(!js_root)  {cJSON_Delete(js_root);}
    if(!js_subdev)  {cJSON_Delete(js_subdev);}
    return rv;
}




