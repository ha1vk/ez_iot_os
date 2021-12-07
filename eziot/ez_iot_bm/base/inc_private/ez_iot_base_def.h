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
 * 
 * Contributors:
 * XuRongjun (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25     xurongjun    first version 
 *******************************************************************************/

#ifndef _EZ_IOT_BASE_DEF_H_
#define _EZ_IOT_BASE_DEF_H_

#include <ezos.h>
#include <ezlog.h>

#define BASE_DOMAIN_ID 1100
#define BASE_DOMAIN_NAME "domain_base"
#define BASE_DOMAIN_VER "v2.0.0"

#ifdef __cplusplus
extern "C"
{
#endif

#define FUNC_IN() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_BASE, "", __FUNCTION__, __LINE__, " in")
#define FUNC_OUT() elog_output(EZ_ELOG_LVL_VERBOSE, TAG_BASE, "", __FUNCTION__, __LINE__, " out")

#define CHECK_COND_DONE(cond, errcode)                                   \
    if ((cond))                                                          \
    {                                                                    \
        ezlog_e(TAG_BASE, "cond done:0x%x,errcode:0x%x", cond, errcode); \
        rv = (errcode);                                                  \
        goto done;                                                       \
    }

#define CHECK_RV_DONE(errcode)                      \
    if (0 != errcode)                               \
    {                                               \
        ezlog_e(TAG_BASE, "errcode:0x%x", errcode); \
        rv = (errcode);                             \
        goto done;                                  \
    }

#ifdef __cplusplus
}
#endif

#endif