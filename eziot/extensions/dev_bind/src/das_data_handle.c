/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
* Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/
#include "ez_error.h"
#include "md5.h"
#include "ezxml.h"
#include "ez_sdk_log.h"
#include "das_data_handle.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ezdev_sdk_kernel.h"
#include "ez_base_def.h"
#include "ez_base_api.h"

#define EZ_BASE_RSP 1
#define EZ_BASE_REQ 0

#define  OPERATION_CODE_LEN 128   ///< max operation code len

#define    kCenPlt2PuSetUserIdReq           0X00004947
#define    kCenPlt2PuSetUserIdRsp           0X00004948
#define    kCenPlt2PuVerifyChallengeCodeReq 0X00002843
#define    kCenPlt2PuVerifyChallengeCodeRsp 0X00002844
#define    kPu2CenPltGetUserListReq         0X00003445
#define    kPu2CenPltGetUserListRsp         0X00003446


static char g_user_operation_code[OPERATION_CODE_LEN] = {0};

static const char *get_verify_code(void);

static int msg2dev_set_userid(const char *userid);

static ez_base_cb_t g_base_cb = {0};

void base_set_cb(ez_base_cb_t cb)
{
    g_base_cb = cb;
}

static int check_result(ezxml_t rsp)
{
    char* response = NULL;
    ezxml_t result;
    char strres[32]={0};
    int ret = -1;
    do
    {
        response = ezxml_name(rsp);
        if(NULL!=response)
        {
            if( 0 != strcmp(response,"Response"))
            {
                ez_log_e(TAG_BASE,"result not response:%s\n",response);
                break;
            }
            result = ezxml_child(rsp, "Result");
            if(NULL!=result)
            {
                strncpy(strres, ezxml_txt(result), sizeof(strres)-1);
                ret = strtol(strres, NULL, 10);
                ez_log_d(TAG_BASE,"check_result:%s,%02x\n", strres, ret);
            }
            else
            {
                ez_log_e(TAG_BASE,"check Result err\n");
            }
        }
        else
        {
            ez_log_e(TAG_BASE,"check response err\n");
            break;
        }
    }while(0);
    
    return ret;
}


int ez_send_msg2plat(unsigned char* msg,unsigned int len, const int cmd_id, const char *cmd_version, unsigned char msg_response, unsigned int* msg_seq)
{
    ezdev_sdk_kernel_pubmsg pubmsg;
    memset(&pubmsg, 0, sizeof(ezdev_sdk_kernel_pubmsg));

    if(NULL == msg||len == 0||NULL == msg_seq)
    {
        return -1;
    }
    
    pubmsg.msg_response = msg_response;
    pubmsg.msg_seq = *msg_seq;

    pubmsg.msg_body = msg;
    pubmsg.msg_body_len = len;

    pubmsg.msg_domain_id = ez_base_module_id;
    pubmsg.msg_command_id = cmd_id;

    strncpy(pubmsg.command_ver, cmd_version, version_max_len - 1);

    ez_log_i(TAG_BASE,"cmd_id:%#02x\n",cmd_id);
    ez_log_v(TAG_BASE,"type:%d ,seq:%d\n",msg_response, *msg_seq);

    ezdev_sdk_kernel_error sdk_error = ezdev_sdk_kernel_send(&pubmsg);
    if (sdk_error != ezdev_sdk_kernel_succ)
    {
        ez_log_e(TAG_BASE,"ezdev_sdk_kernel_send failed:%08x\n", sdk_error);
        return -1;
    }
    if(0 == msg_response)
    {
        *msg_seq = pubmsg.msg_seq;
    }
    ez_log_d(TAG_BASE,"type:%d, msg_seq:%d\n", msg_response, *msg_seq);

    return 0;
}

int das_req_rsp_handle(int req_cmd, void *buf, int buf_len, int rsp_cmd, const char *cmd_version, unsigned int msg_req, req_rsp_handle _handle)
{
    ezxml_t req;
    ezxml_t rsp;
    ezxml_t result;
    char * strrsp = NULL;
    char * strreq = NULL;
    int ret = 0;
    unsigned int rsp_len = 0;
    char err[32] = {0};
    rsp = ezxml_new("Response");
    if(NULL == rsp)
    {
        ez_log_e(TAG_BASE,"new xml failed\n");
        return -1;
    }
    do
    {
        req = ezxml_parse_str((char*) buf, buf_len);
        if (NULL == req)
        { 
            ez_log_e(TAG_BASE,"input xml parse failed \n");
            ret = CIVIL_RESULT_GENERAL_PARSE_FAILED;
            break;
        }
        else
        {
            strreq = ezxml_name(req);
            if(NULL==strreq||0!=strcmp(strreq,"Request"))
            {
                ez_log_e(TAG_BASE,"das req format err\n");
                ret = CIVIL_RESULT_GENERAL_PARSE_FAILED;
                break;
            }
        }
        ret = _handle(req);
    } while (0);

    sprintf(err, "%d", ret);
    result = ezxml_add_child(rsp, "Result", 1);
    ezxml_set_txt(result, err);
    strrsp = ezxml_toxml(rsp);
    if(NULL == strrsp)
    {
        ez_log_e(TAG_BASE,"xml to str failed\n");
    }
    else
    {
        ez_log_v(TAG_BASE,"rsp:%s\n", strrsp);
        rsp_len = strlen(strrsp);
    }
    ret = ez_send_msg2plat((unsigned char*)strrsp, rsp_len, rsp_cmd, cmd_version, EZ_BASE_RSP, &msg_req);
    ez_log_v(TAG_BASE,"ez_send_msg2plat,len:%d,ret:%d\n", rsp_len,ret);

    if(NULL!= req)
    {
        ezxml_free(req);
    }
    if(NULL!= rsp)
    {
        ezxml_free(rsp);
    }
    if(NULL!=strrsp)
    {
        free(strrsp);
        strrsp = NULL;
    }
    return ret;
}

int verify_challengecode_req(ezxml_t req)
{
    int ret = CIVIL_RESULT_GENERAL_NO_ERROR;
    unsigned char md[16]={0};
    unsigned char mcode[33]={0};
    unsigned int  mdlen = sizeof(md);
    ezxml_t Code;
    ezxml_t UserID;
    char strcode[33] = {0};
    char struser_id[64]={0};
    
    do
    {
        Code = ezxml_child(req, "Code");
        if (NULL == Code)
        {
            ez_log_e(TAG_BASE,"VerifyChallengeCode, not find code\n");
            ret= CIVIL_RESULT_GENERAL_COMMAND_NOT_SUITABLE;
            break;
        }
        
        bscomptls_md5((unsigned char *)get_verify_code(), strlen(get_verify_code()), md);
        bscomptls_hexdump(md, mdlen, 0, mcode);

        strncpy(strcode, ezxml_txt(Code), sizeof(strcode)-1);
        if(0!= strcmp(strcode, (char *)mcode))
        {
            ez_log_e(TAG_BASE,"VerifyChallengeCode, code compare err\n");
            ret=CIVIL_RESULT_PU_CHALLENGE_CODE_VERIFY_FAILED;
            break;
        }
        UserID = ezxml_child(req, "UserID");
        if (NULL == UserID)
        {
            ez_log_e(TAG_BASE,"VerifyChallengeCode, not find UserID\n");
            ret=CIVIL_RESULT_GENERAL_COMMAND_NOT_SUITABLE;
            break;
        }
        strncpy(struser_id, ezxml_txt(UserID), sizeof(struser_id)-1);
        ret = msg2dev_set_userid(struser_id);
    } while (0);

    return ret;
}

int pu2plt_query_userid_rsp(void *buf, int buf_len)
{
    int ret = -1;
    char userid[64]={0};
    //char name[64]={0};
    ezxml_t rsp;
    ezxml_t User;
    do
    {
        rsp = ezxml_parse_str((char*)buf, buf_len);
        if(NULL == rsp)
        {
            ez_log_e(TAG_BASE,"userid_rsp: parse err\n");
            break;
        }
        ret = check_result(rsp);
        if (0 != ret)
        {
            if(0x101c02 == ret)
            {
                ez_log_i(TAG_BASE,"device not bind:%0x\n",ret);
                ret = msg2dev_set_userid(userid);
            }
            break;
        }
        User = ezxml_child(rsp, "User");
        if(NULL!=User)
        {
            if(NULL!=ezxml_attr(User,"Id"))
            {
                strncpy(userid, ezxml_attr(User,"Id"), sizeof(userid) -1);
            }
            /*if(NULL!=ezxml_attr(User,"Name"))
            {
                strncpy(name, ezxml_attr(User,"Name"), sizeof(name) -1);
            }*/
        }
        else
        {
            ez_log_e(TAG_BASE,"parse User err\n");
        }
        ret = msg2dev_set_userid(userid);
        ez_log_v(TAG_BASE,"set_userid ret:%d\n", ret);
    } while (0);

    if(NULL!=rsp)
    {
        ezxml_free(rsp);
    }

    return ret;
}

int plt2pu_set_userid(ezxml_t req)
{
    int ret = CIVIL_RESULT_GENERAL_NO_ERROR;
    ezxml_t  x_userid;
    char user_id[64]={0};
    x_userid = ezxml_child(req, "UserId");
    if (NULL!= x_userid)
    {
        strncpy(user_id, ezxml_txt(x_userid), sizeof(user_id)-1);
        ret = msg2dev_set_userid((const char*)user_id);
        ez_log_v(TAG_BASE,"das set userid %s\n", user_id);
    }
    else
    {
        ez_log_e(TAG_BASE,"plt2pu_set_userid err ,no UserId\n");
        ret = CIVIL_RESULT_GENERAL_PARSE_FAILED;
    }
    return ret;
}

int pu2plt_query_userid_req()
{
    int ret = -1;
    ezxml_t req;
    ezxml_t auth;
    ezxml_t serial;
    char* strreq= NULL;
    int len = 0;
    unsigned int msg_seq = 0;
    do
    {
        req = ezxml_new("Request");
        if(NULL== req)
        {
           break;
        }
        auth = ezxml_add_child(req, "Authorization", 1);
        ezxml_set_txt(auth,"");

        serial = ezxml_add_child(req, "DevSerial", 1);
        ezxml_set_txt(serial, ezdev_sdk_kernel_getdevinfo_bykey("dev_subserial"));

        strreq = ezxml_toxml(req);
        if(NULL == strreq)
        {
            break;
        }
        len = strlen(strreq);
        ez_log_v(TAG_BASE,"query_userid_req:%s\n", strreq);
        ret = ez_send_msg2plat((unsigned char*)strreq, len,kPu2CenPltGetUserListReq, ez_base_cmd_version, EZ_BASE_REQ, &msg_seq);

    }while(0);
    
    if(req)
    {
        ezxml_free(req);
    }
    if(NULL!=strreq)
    {
        free(strreq);
        strreq = NULL;
    }

    return ret;
}



static const char *get_verify_code(void)
{
    const char *dev_user_operation_code = g_user_operation_code;
    const char *dev_verify_code = ezdev_sdk_kernel_getdevinfo_bykey("dev_verification_code");
    if (strlen(dev_user_operation_code))
    {
        ez_log_v(TAG_BASE,"operation:%s\n",dev_user_operation_code);
        return dev_user_operation_code;
    }

    if (dev_verify_code == NULL)
    {
        ez_log_e(TAG_BASE,"dev_verify_code is null\n");
        return NULL;
    }

    if (strlen(dev_verify_code))
    {
        ez_log_v(TAG_BASE,"verify:%s\n",dev_verify_code);
        return dev_verify_code;
    }

    return NULL;
}

static int msg2dev_set_userid(const char *userid)
{
    int ret = 0;
    ez_msg2dev_t msg;
    ez_set_userid_t data;
    memset(&msg , 0, sizeof(ez_msg2dev_t));
    memset(&data , 0, sizeof(ez_set_userid_t));

    msg.type = EZ_SET_USERID;
    msg.len = sizeof(ez_set_userid_t);
    msg.data = &data;

    strncpy(data.user_id, userid, sizeof(data.user_id) - 1);
    ret = g_base_cb.recv_msg(&msg);

    return ret;
}
void extend_start_cb(EZDEV_SDK_PTR pUser)
{
    ez_log_d(TAG_BASE,"base extend_start_cb\n");
}

void extend_stop_cb(EZDEV_SDK_PTR pUser)
{
    ez_log_d(TAG_BASE,"base extend_stop_cb\n");
}

void extend_data_route_cb(ezdev_sdk_kernel_submsg *ptr_submsg, EZDEV_SDK_PTR pUser)
{
    int cmd_id = 0;
    int result_code = 0;
    EZDEV_SDK_UINT32 msg_seq = 0;

    if (ptr_submsg == NULL||NULL == ptr_submsg->buf)
    {
        ez_log_e(TAG_BASE,"base data_route input NULL");
        return;
    }

    cmd_id = ptr_submsg->msg_command_id;
    msg_seq = ptr_submsg->msg_seq;

    ez_log_i(TAG_BASE,"cmd:%#02x, seq:%d ,buf_len:%d\n",cmd_id, msg_seq, ptr_submsg->buf_len);
    ez_log_v(TAG_BASE,"recv buf:%s\n",ptr_submsg->buf);

    switch (cmd_id)
    {
    case kCenPlt2PuVerifyChallengeCodeReq:
        result_code = das_req_rsp_handle(cmd_id, ptr_submsg->buf, ptr_submsg->buf_len,kCenPlt2PuVerifyChallengeCodeRsp, 
                                        ez_base_cmd_version, msg_seq, verify_challengecode_req);
        break;
    case kPu2CenPltGetUserListRsp:
        result_code = pu2plt_query_userid_rsp(ptr_submsg->buf, ptr_submsg->buf_len);
        break;
    case kCenPlt2PuSetUserIdReq:
        result_code = das_req_rsp_handle(cmd_id, ptr_submsg->buf, ptr_submsg->buf_len, kCenPlt2PuSetUserIdRsp, 
                                         ez_base_cmd_version, msg_seq, plt2pu_set_userid);
        break;
    default:
        {
            ez_msg2dev_t msg;
            ez_trans_cmd_t data;
            memset(&msg ,  0, sizeof(ez_msg2dev_t));
            memset(&data , 0, sizeof(ez_trans_cmd_t));

            data.content     = (char *)ptr_submsg->buf;
            data.content_len = ptr_submsg->buf_len;
            data.cmd_id      = cmd_id;
            data.msg_seq     = ptr_submsg->msg_seq;

            msg.type         = EZ_TRANS_CMD;
            msg.len          = sizeof(data);
            msg.data         = (void*)&data;

            result_code = g_base_cb.recv_msg(&msg);
        }
        break;
    }

    ez_log_i(TAG_BASE,"result_code:%d\n",result_code);
}

void extend_event_cb(ezdev_sdk_kernel_event *ptr_event, EZDEV_SDK_PTR pUser)
{
    if(NULL == ptr_event)
    {
        ez_log_e(TAG_BASE,"event router,input NULL\n");
        return ;
    }
    ez_log_i(TAG_BASE,"event router,type:%d\n",ptr_event->event_type);
    switch (ptr_event->event_type)
    {
    case sdk_kernel_event_online:
        {
            char skey[ezdev_sdk_sessionkey_len+1]={0};
            sdk_sessionkey_context *sessionkey_context = (sdk_sessionkey_context *)ptr_event->event_context;
            if(sessionkey_context)
            {
                strncpy(skey, (char*)sessionkey_context->session_key, ezdev_sdk_sessionkey_len);
                ez_log_d(TAG_BASE,"session: %s\n", skey);
            }
        }
        break;
    case sdk_kernel_event_break:
        {
            ez_log_d(TAG_BASE,"device offline\n");
        }
        break;
    case sdk_kernel_event_switchover:
        {
            ez_log_d(TAG_BASE,"switchover online\n");
        }
        break;
    case sdk_kernel_event_fast_reg_online:
        {
            ez_log_d(TAG_BASE,"fast reg online\n");
        }
        break;;
    case sdk_kernel_event_runtime_err:
        {
            sdk_runtime_err_context *err_ctx = (sdk_runtime_err_context *)ptr_event->event_context;
            if(NULL!=err_ctx)
            {
                ez_log_v(TAG_BASE,"run_time_cb info: code:%#02x,\n", err_ctx->err_code);
                sdk_send_msg_ack_context* ack_info =(sdk_send_msg_ack_context*)err_ctx->err_ctx;
                if(ack_info)
                {
                    ez_log_v(TAG_BASE,"run_time_cb ack info: domain:%d,cmd_id:%#02x, seq:%d\n", ack_info->msg_domain_id,ack_info->msg_command_id,ack_info->msg_seq);
                }
            }
        }
        break;
    default:
        break;
    }

}


int base_set_operation_code(const char *pcode, const int len)
{
    int ret = 0;
    do
    {
        if (NULL == pcode || len > (OPERATION_CODE_LEN - 1))
        {
            ret = -1;
            break;
        }
        ez_log_v(TAG_BASE,"operation:%s,len:%d\n", pcode, len);
        memset(g_user_operation_code, 0, sizeof(g_user_operation_code));
        strncpy(g_user_operation_code, pcode, sizeof(g_user_operation_code) - 1);
    } while (0);
    
    return ret;
}
