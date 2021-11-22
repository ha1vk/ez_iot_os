#ifndef _TSL_DEF_H_
#define _TSL_DEF_H_

#include <ezos.h>
#include <ezlog.h>

#define TSL_MODULE_NAME "model"
#define TSL_EVENT_METHOD_NAME "event"
#define TSL_SERVICE_METHOD_NAME "service"
#define TSL_ATTRIBUTE_METHOD_NAME "attribute"
#define TSL_MSG_TYPE_REPORT "report"
#define TSL_MSG_TYPE_QUERY "query"
#define TSL_MSG_TYPE_SET_REPLY "set_reply"
#define TSL_MSG_TYPE_OPERATE_REPLY "operate_reply"

#ifdef __FILENAME__
#define FUNC_IN() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_TSL, __FILENAME__, __FUNCTION__, __LINE__, " in")
#define FUNC_OUT() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_TSL, __FILENAME__, __FUNCTION__, __LINE__, " out")
#else
#define FUNC_IN() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_TSL, "", __FUNCTION__, __LINE__, " in")
#define FUNC_OUT() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_TSL, "", __FUNCTION__, __LINE__, " out")
#endif

#define CHECK_COND_DONE(cond, errcode)                                  \
    if ((cond))                                                         \
    {                                                                   \
        ezlog_e(TAG_TSL, "cond done:0x%x,errcode:0x%x", cond, errcode); \
        rv = (errcode);                                                 \
        goto done;                                                      \
    }

#define CHECK_RV_DONE(errcode)                     \
    if (0 != errcode)                              \
    {                                              \
        ezlog_e(TAG_TSL, "errcode:0x%x", errcode); \
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