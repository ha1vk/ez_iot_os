/**
 * @file shadow_def.h
 * @author xurongjun (xurongjun@hikvision.com.cn)
 * @brief shadow 预编译
 * @version 0.1
 * @date 2020-09-20
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef _EZ_IOT_SHADOW_DEF_H_
#define _EZ_IOT_SHADOW_DEF_H_

#include <ezos.h>
#include <ezlog.h>


// /* 连续在线未发生变更强制上报时间 */
// #define FORCE_SYNC_TIME 1000 * 60 * 60 * 24

// /* 发送请求最小重试间隔 */
// #define MINIIMUM_RETRY_INTERVAL 60 * 1000

// /* 发送同步请求最大请求次数 */
// #define MAXIMUM_RETRY_TIMES 3

// /* 最大信令缓冲区 */
// #define PUSH_BUF_MAX_LEN 1024 * 2



#define SHADOW_PUSH_BUF_MAX (CONFIG_EZIOT_CORE_MESSAGE_SIZE_MAX - 128)
#define SHADOW_DEFAULT_NAME "global"
#define SHADOW_MODULE_NAME "basic"
#define SHADOW_METHOD_NAME "shadow"
#define SHADOW_MSG_TYPE_REPORT "report"
#define SHADOW_MSG_TYPE_QUERY "query"
#define SHADOW_MSG_TYPE_QUERY_REPLY "query_reply"
#define SHADOW_MSG_TYPE_SET "set"
#define SHADOW_MSG_TYPE_SET_REPLY "set_reply"

#ifdef __FILENAME__
#define FUNC_IN() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_SHD, __FILENAME__, __FUNCTION__, __LINE__, " in")
#define FUNC_OUT() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_SHD, __FILENAME__, __FUNCTION__, __LINE__, " out")
#else
#define FUNC_IN() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_SHD, "", __FUNCTION__, __LINE__, " in")
#define FUNC_OUT() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_SHD, "", __FUNCTION__, __LINE__, " out")
#endif

#define CHECK_COND_DONE(cond, errcode)                                  \
    if ((cond))                                                         \
    {                                                                   \
        ezlog_e(TAG_SHD, "cond done:0x%x,errcode:0x%x", cond, errcode); \
        rv = (errcode);                                                 \
        goto done;                                                      \
    }

#define CHECK_RV_DONE(errcode)                     \
    if (0 != errcode)                              \
    {                                              \
        ezlog_e(TAG_SHD, "errcode:0x%x", errcode); \
        rv = (errcode);                            \
        goto done;                                 \
    }

#define SAFE_FREE(p)  \
    if (p)            \
    {                 \
        ezos_free(p); \
        p = NULL;     \
    }

#endif