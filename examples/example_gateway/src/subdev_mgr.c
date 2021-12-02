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
#include "kv_imp.h"
#include "ez_iot_tsl.h"
#include "cJSON.h"
#include "flashdb.h"
#include "hub_mgr.h"
#include "ezcloud_access.h"



#define CHECK_DONE(cond) \
    if ((cond))                        \
    {                                  \
        goto done;                     \
    }

ez_char_t *g_addsubdev_UUID = NULL;

static add_subdev_info_t add_subdev_info;

void property_report_addResult(void);//上报添加的子设备信息

void record_subdev_info(const ez_subdev_info_t *subdev_info)
{
    if(NULL == subdev_info)
    {return;}
    
    strcpy(add_subdev_info.addsubdev[add_subdev_info.addsubdev_num].subdev_sn , subdev_info->subdev_sn);

    strcpy(add_subdev_info.addsubdev[add_subdev_info.addsubdev_num].pass , "adding");
    
    add_subdev_info.addsubdev_num++;
}

void add_subdev_example(void)//添加子设备示例
{
    ez_subdev_info_t subdev_info;

    if(g_addsubdev_UUID == NULL)  //如果未使能添加子设备窗口，则无法添加子设备
    return ;

    /* 输入 子设备信息 */
    strcpy(subdev_info.subdev_sn,SUBDEV_TEST_SN);
    strcpy(subdev_info.subdev_type,SUBDEV_TEST_PID);
    strcpy(subdev_info.subdev_ver,SUBDEV_TEST_VERSION);
    subdev_info.sta = true;

    int buf_result = ez_iot_hub_add(&subdev_info);  //添加子设备
    
    if(buf_result  == EZ_HUB_ERR_SUBDEV_EXISTED) //如果子设备已存在,更新版本号，注册ota信息
    {
        ez_iot_hub_ver_update(subdev_info.subdev_sn , subdev_info.subdev_ver);//更新该子设备的版本信息
        //ez_ota_start(); //如果开启了OTA功能  ，子设备更新，需要注册ota信息
    }
    else if(buf_result == 0) //添加子设备成功
    {
        ez_subdev_alive_add(&subdev_info);     //添加子设备的心跳管理,加入到kv
        record_subdev_info(&subdev_info);//记录本次添加的子设备信息,用于APP界面显示本次添加的设备
        //ez_ota_start(); //如果开启了OTA功能  ，添加子设备成功，需要注册ota信息
    }
    else
    {
        ez_iot_hub_del(subdev_info.subdev_sn);
        ezlog_e(TAG_AT, "ez_iot_hub_add fail");
    }

}

void del_subdev_example(void)//删除子设备示例
{
    ez_subdev_info_t subdev_info;

    strcpy(subdev_info.subdev_sn , SUBDEV_TEST_SN);
    subdev_info.sta = false;

    ez_iot_hub_del(subdev_info.subdev_sn);
    ez_subdev_alive_del(subdev_info.subdev_sn);        //删除子设备       同时删除心跳管理的kv
}






#define DEFAULT_ADD_SUBDEV_OVERTIME   300  //定义添加子设备的默认超时时间
static time_t adddev_overtime_point;//超时时间点
static char g_openadd_state[10] = {0};//开放添加子设备的状态  “success”添加完成   “adding”正在添加   “error”未添加
static add_subdev_info_t add_subdev_info;

void start_adddev_timer(int overtime)
{

    time_t now = 0;

    time(&now);
    adddev_overtime_point = now + overtime;
    memset(g_openadd_state,0,sizeof(g_openadd_state));
    strcpy(g_openadd_state,"adding"); // 状态设置成添子加设备中
}

void open_add_subdev(char *value)//打开添加子设备窗口
{
//  入参示例      {"addType":"1","uuid":"805103502289","subCat":"human-sensor"}

    cJSON *js_root = NULL;
    cJSON *js_buff = NULL;
    cJSON *js_validityPeriod = NULL;
    int validityPeriod = 0;
    char *type_buf =NULL;
    
    js_root = cJSON_Parse(value);
    CHECK_DONE(!js_root);

    js_buff = cJSON_GetObjectItem(js_root , "uuid");
    CHECK_DONE(js_buff==NULL);
    g_addsubdev_UUID = (ez_char_t *)malloc(strlen(js_buff->valuestring)+1);
    memset(g_addsubdev_UUID,0,strlen(g_addsubdev_UUID));
    strcpy(g_addsubdev_UUID , js_buff->valuestring);

    js_buff = cJSON_GetObjectItem(js_root , "addType");
    CHECK_DONE(!js_buff);
    type_buf = (char *)malloc(strlen(js_buff->valuestring)+1);
    CHECK_DONE(!type_buf);
    strcpy(type_buf , js_buff->valuestring);

    js_validityPeriod = cJSON_GetObjectItem(js_root , "validityPeriod");
    if((!js_validityPeriod )|| js_validityPeriod->type != cJSON_Number)
    {validityPeriod = DEFAULT_ADD_SUBDEV_OVERTIME;}//如果未设置有效期限，则默认300秒
    else
    {validityPeriod = js_validityPeriod->valueint;}

    
    if( 0 == strcmp("1", type_buf))//单个设备添加
    {
//        cJSON *subCat = cJSON_GetObjectItem(js_root , "subCat");
//      at_cmd_allow_subdev_online(subCat->valuestring , COMPONENT_HUB_SUBLIST_MAX, validityPeriod);//AT告知可添加子设备
        start_adddev_timer(validityPeriod);//启动5分钟倒计时
        memset(&add_subdev_info, 0, sizeof(add_subdev_info));
        
    }
    else if( 0 == strcmp("2", type_buf))   //多个设备添加
    {
//        at_cmd_allow_subdev_online("NULL",COMPONENT_HUB_SUBLIST_MAX,validityPeriod); //AT告知可添加子设备
        start_adddev_timer(validityPeriod);//启动5分钟倒计时
        memset(&add_subdev_info, 0, sizeof(add_subdev_info));
        
    }
    else
    {
        
    }
    //上传一次addResult 属性        //本次子设备添加情况上传
    property_report_addResult();
    add_subdev_info.addsubdev_num = 0;//本次添加的子设备数量清零
    
done:
    if(!js_root)  {cJSON_Delete(js_root);}
    if(!type_buf) {free(type_buf);}

}

void close_add_subdev(void)
{
    if(strcmp(g_openadd_state,"success") != 0)
    {
        //at_cmd_forbid_subdev_online();//超时处理,   下发AT指令，关闭入网窗口   
        
        memset(g_openadd_state,0,sizeof(g_openadd_state));
        strcpy(g_openadd_state,"success"); // 状态设置成添子加设备中
        memset(&add_subdev_info , 0 , sizeof(add_subdev_info));//清除本次添加过程中的子设备信息
        
        if(g_addsubdev_UUID)
        {
            free(g_addsubdev_UUID);
            g_addsubdev_UUID = NULL;
        }
    }
}



void adddev_overtime_close_window(void)//添加子设备窗口超时，关闭添加窗口
{
    
    time_t now = 0;

    time(&now);

    if(adddev_overtime_point < now &&  (strcmp(g_openadd_state,"success") != 0))//判断是否超时，和当前窗口状态
    {
        close_add_subdev();
    }
    
}



void property_report_addResult(void)//上报添加的子设备信息
{
    char *buff_object = NULL;
    char *buff_value = NULL;
    ez_tsl_value_t value_out = {0};

     if(g_addsubdev_UUID ==NULL){return;}

     ez_tsl_key_t key_info = {.domain = (int8_t *)"SubDevice", .key = (int8_t *)"addResult"};
     ez_tsl_rsc_t rsc_info = {.res_type = (int8_t *)"global", .local_index = (int8_t *)"0"};
    
    value_out.type = EZ_TSL_DATA_TYPE_OBJECT;
    
    int num = add_subdev_info.addsubdev_num * (sizeof(subdev_type_t)+50)+50;
    buff_object = (char *)malloc(num);
    memset(buff_object,0,num);


    int i;
    for(i=0;i<add_subdev_info.addsubdev_num;i++)
    {
        if(i == 0)
        {
            sprintf(buff_object, "{\"sn\":\"%s\",\"state\":\"%s\"}",add_subdev_info.addsubdev[i].subdev_sn,add_subdev_info.addsubdev[i].pass);
        }
        else
        {
            char buff[256];
            memset(buff,0,sizeof(buff));
            sprintf(buff, ",{\"sn\":\"%s\",\"state\":\"%s\"}",add_subdev_info.addsubdev[i].subdev_sn,add_subdev_info.addsubdev[i].pass);
            strcat(buff_object,buff);
        }
    }
    
    
    buff_value = (char *)malloc(add_subdev_info.addsubdev_num * (sizeof(subdev_type_t)+100)+100);
    
    sprintf(buff_value,"{\"uuid\":\"%s\",\"state\":\"%s\",\"subDevice\":[%s]}",g_addsubdev_UUID,g_openadd_state,buff_object);
    
    value_out.value = malloc(strlen(buff_value)+1);
    memset(value_out.value , 0 ,(strlen(buff_value)+1) );
    strcpy(value_out.value , buff_value);
    
    value_out.size = strlen(value_out.value);
    
    ez_iot_tsl_property_report(ez_cloud_get_sn(), &rsc_info, &key_info, &value_out);
    
    if(!buff_value) {free(buff_value);}
    if(!buff_object) {free(buff_object);}

}



