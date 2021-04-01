#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bscJSON.h"
#include "ez_sdk_log.h"
#include "ez_model_def.h"
#include "ez_model.h"
#include "ezdev_sdk_kernel_struct.h"
#include "base_typedef.h"
#include "model_sample.h"

#define CHECK_NULL(cond) \
    if((cond))           \
    {                    \
        break;           \
    }

#define SAFE_FREE(p) \
    if (p)           \
    {                \
        free(p);     \
        p = NULL;    \
    }

#define JSON_SAFE_FREE(IN) \
    if (IN)                \
    {                     \
        bscJSON_Delete(IN);\
    }


static int parse_attr_value(ez_model_msg* msg, char* status, int len)
{
    do
    {
        CHECK_NULL(!msg);
        CHECK_NULL(!status);

        ez_log_i(TAG_APP, "msg->type:%d\n", msg->type);
        if(model_data_type_string == msg->type)
        {
            strncpy(status,(char*)msg->value, len -1);
            ez_log_d(TAG_APP, "status value:%s \n", status);
        } 
        //bscJSON* pJsMsg = bscJSON_Parse((char*)msg->value);
       // CHECK_NULL((NULL==pJsMsg));
        //ez_log_d(TAG_APP, "json type:%d\n", pJsMsg->type);
        //if(bscJSON_String == pJsMsg->type)
       // {
       //     strncpy(status, pJsMsg->valuestring, len -1);
        //}
        //JSON_SAFE_FREE(pJsMsg);
       
    } while (0);

    return 0;
}

//属性应答消息
int ez_model_attr_das_reply(ez_basic_info* pbasic_info, char* msg_type, ez_model_msg* buf, ez_err_info* errinfo, unsigned int msg_seq)
{ 
    ez_log_d(TAG_APP," attribute reply:status:%d, code:%s, err_msg:%s\n", errinfo->status, errinfo->err_code, errinfo->err_msg);
    return 0;
}

//属性请求消息
static int ez_model_attr_das_req(ez_basic_info* pbasic_info, char* msg_type, ez_model_msg* buf, unsigned int msg_seq)
{ 
    int ret = -1;
    char* szdata = NULL;
    ez_err_info status;
    ez_model_msg    msg;
    ez_msg_attr msg_attr;
    char szvalue[8]={0};
    char szstatus[16]={0};

    memset(&status, 0, sizeof(ez_err_info));
    memset(&msg, 0, sizeof(ez_model_msg));
    memset(&msg_attr, 0, sizeof(ez_msg_attr));

    if(NULL == pbasic_info||NULL== msg_type || NULL == buf)
    {
        ez_log_d(TAG_APP," attribute das req input null\n");
        return -1;
    }  
    ez_log_d(TAG_APP," recv attr das req:%s \n", (char*)buf->value);
    do
    {
        msg_attr.msg_qos= 0;
        msg_attr.msg_seq= msg_seq;//回复响应的时候需要将这个seq值带回给服务
        strncpy(status.err_code, "0x0", sizeof(status.err_code)-1);
        status.status = 200;
        strncpy(status.err_msg, "success", EZ_ERR_MSG_LEN - 1);

        if(0 == strcmp(msg_type, "set"))
        {
            parse_attr_value(buf, szstatus ,sizeof(szstatus));

            strncpy(msg_attr.msg_type, "set_reply", EZ_MSG_TYPE_LEN-1);
            ///< 先应答服务的属性设置请求,此时需要将seq值原样带回去,
            ret = ez_model_reply_to_das(pbasic_info, &msg, &status, &msg_attr);

            msg.type = model_data_type_string;
            msg.value = szstatus;
            msg.length = strlen(szstatus);

            ///< 上报的时候seq可以随便传入一个数值,
            memset(msg_attr.msg_type, 0, EZ_MSG_TYPE_LEN);
            strncpy(msg_attr.msg_type, "report", EZ_MSG_TYPE_LEN-1);

            ret = ez_model_send_msg(pbasic_info, &msg, &msg_attr);
        }
        /* code */
    } while (0);

    return ret;
}

//操作请求消息
int ez_model_service_das_req(ez_basic_info* basic_info, char* msg_type, ez_model_msg* recv_buf, unsigned int msg_seq)
{   
    int ret = 0;
    int value = 50;
    char* szdata = NULL;
    bscJSON* root  = NULL;
    bscJSON* data  = NULL;

    ez_err_info status;
    ez_model_msg    msg;
    ez_msg_attr msg_attr;

    if(NULL == basic_info||NULL == recv_buf||NULL  == msg_type)
    {
        ez_log_i(TAG_APP,"ez_model_service_das_req input invalid \n");
        return -1;
    }
  
    memset(&status, 0, sizeof(ez_err_info));
    memset(&msg, 0, sizeof(ez_model_msg));
    memset(&msg_attr, 0, sizeof(ez_msg_attr));

    strncpy(status.err_code, "0x0", EZ_ERR_CODE_LEN-1);
    status.status = 200;
    strncpy(status.err_msg, "success", EZ_ERR_MSG_LEN - 1);

    msg.type= model_data_type_object;
    msg_attr.msg_qos = 0;
    msg_attr.msg_seq = msg_seq;

    do
    {
        root = bscJSON_CreateObject();
        CHECK_NULL(!root);

        data = bscJSON_CreateObject();
        CHECK_NULL(!data);

        bscJSON_AddNumberToObject(data,"value", value);
        bscJSON_AddObjectToObject(root, "data", data);
        szdata = bscJSON_PrintUnformatted(root);
        CHECK_NULL(!szdata);

        msg.value = szdata;
        msg.length = strlen(szdata);

        if(0 == strcmp(msg_type, "operate"))
        {
            strncpy(msg_attr.msg_type, "operate_reply", EZ_MSG_TYPE_LEN-1);
            ret = ez_model_reply_to_das(basic_info, &msg, &status, &msg_attr);
        }
    } while (0);
    
   SAFE_FREE(szdata);
   JSON_SAFE_FREE(root);

   return ret;
}

//操作应答消息
int ez_service_manage_das_reply(ez_basic_info* basic_info, ez_err_info* status, void* buf)
{   
    if(NULL == status|| NULL == buf)
    {
        ez_log_i(TAG_APP," service das reply input null\n");
        return -1;
    }
    ez_log_i(TAG_APP," service reply:status:%d, code:%s, err_msg:%s\n",status->status, status->err_code,status->err_msg);
    return 0;
}

static int ez_model_recv_das_reply(ez_basic_info* basic_info, char* msg_type, ez_err_info* status, ez_model_msg* buf, unsigned int msg_seq)
{
    //收到das回复的应答消息。不需要再回复
    int ret = -1;
    switch(basic_info->type)
    {
        case ez_event:
            //ToDo 开发者根据自身业务处理服务下发事件消息
            break;
        case ez_attribute:
           //ToDo 开发者根据自身业务处理das下发属性响应消息
            ret = ez_model_attr_das_reply(basic_info, msg_type, buf, status, msg_seq);
            break;
        case ez_service:
            break;
        default:
            break;
    }
    return ret;
}

static int ez_model_recv_das_req(ez_basic_info* basic_info, char* msg_type, ez_model_msg* buf, unsigned int msg_seq)
{
    int ret = 0;
    if(NULL == buf)
    {
        return -1;
    }
    ez_log_i(TAG_APP,"msg_type:%s,resource_id:%s, resource_type:%s, domain_id:%s, identifier:%s \n",msg_type, \
     basic_info->resource_id, basic_info->resource_type, basic_info->domain, basic_info->identifier);
    switch(basic_info->type)
    {
        case ez_event:
            //ToDo 开发者根据自身业务处理服务下发事件消息
            break;
        case ez_attribute:
            ret = ez_model_attr_das_req(basic_info, msg_type, buf, msg_seq);
            break;
        case ez_service:
            ret = ez_model_service_das_req(basic_info, msg_type, buf, msg_seq);
            break;
        default:
            break;
    }

    return ret;
}

int model_sample_start()
{
    int ret = 0;
    ez_domain_reg  domain_reg;
    memset(&domain_reg, 0, sizeof(ez_domain_reg));
    do 
    {
        strncpy(domain_reg.domain, "WaterPurifier",sizeof(domain_reg.domain) -1);
        domain_reg.das_req_router = ez_model_recv_das_req;
        domain_reg.das_reply_router = ez_model_recv_das_reply;
        
        ret = ez_model_domain_reg(&domain_reg);
        if(0!=ret)
        {
            ez_log_e(TAG_APP,"ez_model_domain_reg err\n");
        }
    }while(0);

    return ret ;
}

int model_sample_stop()
{
    int ret = 0;
    ret = ez_model_domain_dereg("WaterPurifier");
    if(0!=ret )
    {
       ez_log_e(TAG_APP,"ez_model_domain_dereg err\n");
    }
   
    return 0;
}