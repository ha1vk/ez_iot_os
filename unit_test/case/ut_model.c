#include <stdlib.h>
#include <string.h>

#include "ez_iot_core.h"
#include "cJSON.h"
#include "ez_iot_core_def.h"
#include "ez_iot_model.h"
#include "ez_iot_model_def.h"
#include <ezlog.h>
#include <kv_imp.h>
#include "utest.h"
#include <ezos.h>

#define CHECK_NULL(cond) \
    if((cond))           \
    {                    \
        break;           \
    }

#define SAFE_FREE(p) \
    if (p)           \
    {                \
        ezos_free(p);     \
        p = NULL;    \
    }

#define JSON_SAFE_FREE(IN) \
    if (IN)                \
    {                     \
        cJSON_Delete(IN);\
    }


static long global_init();

void ut_model_test();
static void eziot_ut_model(void)
{
    UTEST_UNIT_RUN(ut_model_test);
}
UTEST_TC_EXPORT(eziot_ut_model, "eziot.ut_model", global_init, NULL, 60);

static int m_event_id = -1;
static ez_server_info_t m_lbs_addr = {CONFIG_EZIOT_UNIT_TEST_CLOUD_HOST, CONFIG_EZIOT_UNIT_TEST_CLOUD_PORT};
static ez_dev_info_t m_dev_info = {0};


static ez_kv_func_t g_kv_func = {
    .ezos_kv_init = kv_init,
    .ezos_kv_raw_set = kv_raw_set,
    .ezos_kv_raw_get = kv_raw_get,
    .ezos_kv_del = kv_del,
    .ezos_kv_del_by_prefix = kv_del_by_prefix,
    .ezos_kv_print = kv_print,
    .ezos_kv_deinit = kv_deinit,
};

static int parse_attr_value(ez_model_msg_t* msg, char* status, int len)
{
    do
    {
        CHECK_NULL(!msg);
        CHECK_NULL(!status);

        ezlog_i(TAG_APP, "msg->type:%d\n", msg->type);
        if(model_data_type_string == msg->type)
        {
            strncpy(status,(char*)msg->value, len -1);
            ezlog_d(TAG_APP, "status value:%s \n", status);
        } 
       
    } while (0);

    return 0;
}

//属性应答消息
int ez_model_attr_das_reply(ez_basic_info_t* pbasic_info, char* msg_type, ez_model_msg_t* buf, ez_err_info_t* errinfo, unsigned int msg_seq)
{ 
    ezlog_d(TAG_APP," attribute reply:status:%d, code:%s, err_msg:%s\n", errinfo->status, errinfo->err_code, errinfo->err_msg);
    return 0;
}

//属性请求消息
static int ez_model_attr_das_req(ez_basic_info_t* pbasic_info, char* msg_type, ez_model_msg_t* buf, unsigned int msg_seq)
{ 
    int ret = -1;
    char* szdata = NULL;
    ez_err_info_t status;
    ez_model_msg_t    msg;
    ez_msg_attr_t msg_attr;
    char szvalue[8]={0};
    char szstatus[16]={0};

    memset(&status, 0, sizeof(ez_err_info_t));
    memset(&msg, 0, sizeof(ez_model_msg_t));
    memset(&msg_attr, 0, sizeof(ez_msg_attr_t));

    if(NULL == pbasic_info||NULL== msg_type || NULL == buf)
    {
        ezlog_d(TAG_APP," attribute das req input null\n");
        return -1;
    }  
    ezlog_d(TAG_APP," recv attr das req:%s \n", (char*)buf->value);
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
            ret = ez_iot_model_reply_to_das(pbasic_info, &msg, &status, &msg_attr);

            msg.type = model_data_type_string;
            msg.value = szstatus;
            msg.length = strlen(szstatus);

            ///< 上报的时候seq可以随便传入一个数值,
            memset(msg_attr.msg_type, 0, EZ_MSG_TYPE_LEN);
            strncpy(msg_attr.msg_type, "report", EZ_MSG_TYPE_LEN-1);

            ret = ez_iot_model_send_msg(pbasic_info, &msg, &msg_attr);
        }
        /* code */
    } while (0);

    return ret;
}

//操作请求消息
int ez_model_service_das_req(ez_basic_info_t* basic_info, char* msg_type, ez_model_msg_t* recv_buf, unsigned int msg_seq)
{   
    int ret = 0;
    int value = 50;
    char* szdata = NULL;
    cJSON* root  = NULL;
    cJSON* data  = NULL;
    ez_err_info_t status;
    ez_model_msg_t    msg;
    ez_msg_attr_t msg_attr;
    if(NULL == basic_info||NULL == recv_buf||NULL  == msg_type)
    {
        ezlog_i(TAG_APP,"ez_model_service_das_req input invalid \n");
        return -1;
    }
    memset(&status, 0, sizeof(ez_err_info_t));
    memset(&msg, 0, sizeof(ez_model_msg_t));
    memset(&msg_attr, 0, sizeof(ez_msg_attr_t));
    strncpy(status.err_code, "0x0", EZ_ERR_CODE_LEN-1);
    status.status = 200;
    strncpy(status.err_msg, "success", EZ_ERR_MSG_LEN - 1);
    msg_attr.msg_qos = 0;
    msg_attr.msg_seq = msg_seq;

    do
    {
        root = cJSON_CreateObject();
        CHECK_NULL(!root);
        data = cJSON_AddObjectToObject(root, "data");
        CHECK_NULL(!data);
        cJSON_AddNumberToObject(data,"value", value);
        szdata = cJSON_PrintUnformatted(root);
        CHECK_NULL(!szdata);
        msg.type= model_data_type_object;
        msg.value = szdata;
        msg.length = strlen(szdata);
        if(0 == strcmp(msg_type, "operate"))
        {
            strncpy(msg_attr.msg_type, "operate_reply", EZ_MSG_TYPE_LEN-1);
            ret = ez_iot_model_reply_to_das(basic_info, &msg, &status, &msg_attr);
        }

    } while (0);
    
   SAFE_FREE(szdata);
   JSON_SAFE_FREE(root);

   return ret;
}

//操作应答消息
int ez_service_manage_das_reply(ez_basic_info_t* basic_info, ez_err_info_t* status, void* buf)
{   
    if(NULL == status|| NULL == buf)
    {
        ezlog_i(TAG_APP," service das reply input null\n");
        return -1;
    }
    ezlog_i(TAG_APP," service reply:status:%d, code:%s, err_msg:%s\n",status->status, status->err_code,status->err_msg);
    return 0;
}

static int ez_model_recv_das_reply(ez_basic_info_t* basic_info, char* msg_type, ez_err_info_t* status, ez_model_msg_t* buf, unsigned int msg_seq)
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

static int ez_model_recv_das_req(ez_basic_info_t* basic_info, char* msg_type, ez_model_msg_t* buf, unsigned int msg_seq)
{
    int ret = 0;
    if(NULL == buf)
    {
        return -1;
    }
    ezlog_i(TAG_APP,"msg_type:%s,resource_id:%s, resource_type:%s, domain_id:%s, identifier:%s \n",msg_type, \
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
    ez_domain_reg_t  domain_reg;
    memset(&domain_reg, 0, sizeof(ez_domain_reg_t));
    do 
    {
        strncpy(domain_reg.domain, "PowerMgr",sizeof(domain_reg.domain) -1);
        domain_reg.das_req_router = ez_model_recv_das_req;
        domain_reg.das_reply_router = ez_model_recv_das_reply;
        
        ret = ez_iot_model_domain_reg(&domain_reg);
        if(0!=ret)
        {
            ezlog_e(TAG_APP,"ez_iot_model_domain_reg err\n");
        }
    }while(0);

    return ret;
}

int model_sample_stop()
{
    int ret = 0;
    ret = ez_iot_model_domain_dereg("PowerMgr");
    if(0!=ret )
    {
       ezlog_e(TAG_APP,"ez_iot_model_domain_dereg err\n");
    }
   
    return ret;
}

static ez_int32_t ez_event_notice_func(ez_event_e event_type, ez_void_t *data, ez_int32_t len)
{
    switch (event_type)
    {
    case EZ_EVENT_ONLINE:
        m_event_id = EZ_EVENT_ONLINE;
        break;
    case EZ_EVENT_OFFLINE:
        /* save devid */
        break;
    case EZ_EVENT_DEVID_UPDATE:
        break;

    default:
        break;
    }

    return 0;
}

void ut_model_test(void)
{
    ez_byte_t devid[33] = "RkYm0KKEgz4VEXG5te7sPxI2AIaQD9o=";


    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_DEVID_SET, (ez_void_t *)devid));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_ctrl(EZ_CMD_KVIMPL_SET, (ez_void_t *)&g_kv_func));
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_init(&m_lbs_addr, &m_dev_info, ez_event_notice_func));

    uassert_int_equal(EZ_CODE_SUCESS, model_sample_start());
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_start());

    ezos_delay_ms(10000);
    uassert_int_equal(EZ_CODE_SUCESS, model_sample_stop());
    uassert_int_equal(EZ_CORE_ERR_SUCC, ez_iot_core_stop());
    ez_iot_core_deinit();
    //ezlog_stop();
}

static long global_init()
{
    //ezlog_init();
    //ezlog_start();
    //ezlog_filter_lvl(CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL);

    ezos_strncpy(m_dev_info.dev_typedisplay, CONFIG_EZIOT_UNIT_TEST_DEV_DISPLAY_NAME, sizeof(m_dev_info.dev_typedisplay) - 1);
    ezos_strncpy(m_dev_info.dev_firmwareversion, CONFIG_EZIOT_UNIT_TEST_DEV_FIRMWARE_VERSION, sizeof(m_dev_info.dev_firmwareversion) - 1);

#if defined(CONFIG_EZIOT_UNIT_TEST_DEV_AUTH_MODE_SAP)
    m_dev_info.auth_mode = 0;
    ezos_strncpy(m_dev_info.dev_type, CONFIG_EZIOT_UNIT_TEST_DEV_TYPE, sizeof(m_dev_info.dev_type) - 1);
    ezos_strncpy(m_dev_info.dev_subserial, CONFIG_EZIOT_UNIT_TEST_DEV_SERIAL_NUMBER, sizeof(m_dev_info.dev_subserial) - 1);
    ezos_strncpy(m_dev_info.dev_verification_code, CONFIG_EZIOT_UNIT_TEST_DEV_VERIFICATION_CODE, sizeof(m_dev_info.dev_verification_code) - 1);
#elif defined(CONFIG_EZIOT_UNIT_TEST_DEV_AUTH_MODE_LICENCE)
    m_dev_info.auth_mode = 1;
    ezos_strncpy(m_dev_info.dev_type, CONFIG_EZIOT_UNIT_TEST_DEV_PRODUCT_KEY, sizeof(m_dev_info.dev_type) - 1);
    ezos_snprintf(m_dev_info.dev_subserial, sizeof(m_dev_info.dev_subserial) - 1, "%s:%s", CONFIG_EZIOT_UNIT_TEST_DEV_PRODUCT_KEY, CONFIG_EZIOT_UNIT_TEST_DEV_NAME);
    ezos_strncpy(m_dev_info.dev_verification_code, CONFIG_EZIOT_UNIT_TEST_DEV_LICENSE, sizeof(m_dev_info.dev_verification_code) - 1);
#endif

    return 0;
}
