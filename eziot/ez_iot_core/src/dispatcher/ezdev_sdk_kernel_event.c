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
 *******************************************************************************/

#include <ezos.h>
#include <ezlog.h>
#include "ezdev_sdk_kernel_event.h"
#include "ezdev_sdk_kernel_queue.h"
#include "mkernel_internal_error.h"
#include "ezdev_sdk_kernel_struct.h"

EXTERN_QUEUE_FUN(inner_cb_notic)
EXTERN_QUEUE_BASE_FUN
/** 
 *  \brief		添加一条消息至本地消息队列
 *  \method		push_event_to_queue
 *  \param[in] 	prt_inner_cb_notic	消息对象
 *  \return 	成功返0 失败返回对应错误码
 */
static mkernel_internal_error push_event_to_queue(ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic)
{
    mkernel_internal_error kernel_error = mkernel_internal_succ;
    ez_int32_t i;
    if (NULL == prt_inner_cb_notic)
    {
        return mkernel_internal_input_param_invalid;
    }

    kernel_error = push_queue_inner_cb_notic(prt_inner_cb_notic);

    if (mkernel_internal_succ != kernel_error)
    {
#if CONFIG_EZIOT_CORE_MULTI_TASK
        ezlog_e(TAG_CORE, "queue no memory, try again.");

        for (i = 0; i < 50; i++)
        {
            kernel_error = push_queue_inner_cb_notic(prt_inner_cb_notic);
            if (mkernel_internal_succ == kernel_error)
            {
                break;
            }
        }
#else
        ezlog_e(TAG_CORE, "queue no memory");
#endif
    }

    return kernel_error;
}

/** 
 *  \brief		广播一条微内核启动消息
 *	\note		接收对象 已向微内核注册的所有领域
 *  \method		broadcast_user_start
 *  \return 	成功返0 失败返回对应错误码
 */
mkernel_internal_error broadcast_user_start()
{
    mkernel_internal_error kernel_error = mkernel_internal_succ;
    ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic = NULL;
    prt_inner_cb_notic = (ezdev_sdk_kernel_inner_cb_notic *)ezos_malloc(sizeof(ezdev_sdk_kernel_inner_cb_notic));
    if (NULL == prt_inner_cb_notic)
    {
        return mkernel_internal_malloc_error;
    }

    ezos_memset(prt_inner_cb_notic, 0, sizeof(ezdev_sdk_kernel_inner_cb_notic));
    prt_inner_cb_notic->cb_type = extend_cb_start;
    prt_inner_cb_notic->cb_event.event_type = 0;
    prt_inner_cb_notic->cb_event.event_context = NULL;
    kernel_error = push_event_to_queue(prt_inner_cb_notic);
    return kernel_error;
}

/** 
 *  \brief		广播一条微内核停止消息
 *	\note		接收对象 已向微内核注册的所有领域
 *  \method		broadcast_user_stop
 *  \return 	成功返0 失败返回对应错误码
 */
mkernel_internal_error broadcast_user_stop()
{
    mkernel_internal_error kernel_error = mkernel_internal_succ;
    ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic = NULL;
    prt_inner_cb_notic = (ezdev_sdk_kernel_inner_cb_notic *)ezos_malloc(sizeof(ezdev_sdk_kernel_inner_cb_notic));
    if (NULL == prt_inner_cb_notic)
    {
        return mkernel_internal_malloc_error;
    }
    ezos_memset(prt_inner_cb_notic, 0, sizeof(ezdev_sdk_kernel_inner_cb_notic));
    prt_inner_cb_notic->cb_type = extend_cb_stop;
    prt_inner_cb_notic->cb_event.event_type = 0;
    prt_inner_cb_notic->cb_event.event_context = NULL;

    kernel_error = push_event_to_queue(prt_inner_cb_notic);
    return kernel_error;
}

mkernel_internal_error broadcast_user_event(sdk_kernel_event_type event_type, void *ctx, EZDEV_SDK_UINT32 ctx_size)
{
    mkernel_internal_error rv = mkernel_internal_succ;
    void *context = NULL;
    ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic = NULL;

    do
    {
        prt_inner_cb_notic = (ezdev_sdk_kernel_inner_cb_notic *)ezos_malloc(sizeof(ezdev_sdk_kernel_inner_cb_notic));
        if (NULL == prt_inner_cb_notic)
        {
            rv = mkernel_internal_malloc_error;
            break;
        }

        if (NULL != ctx)
        {
            if (NULL == (context = (void *)ezos_malloc(ctx_size)))
            {
                ezos_free(prt_inner_cb_notic);
                rv = mkernel_internal_malloc_error;
                break;
            }

            ezos_memcpy(context, ctx, ctx_size);
        }

        ezos_memset(prt_inner_cb_notic, 0, sizeof(ezdev_sdk_kernel_inner_cb_notic));
        prt_inner_cb_notic->cb_type = extend_cb_event;
        prt_inner_cb_notic->cb_event.event_type = event_type;
        prt_inner_cb_notic->cb_event.event_context = context;
        if (mkernel_internal_succ != (rv = push_event_to_queue(prt_inner_cb_notic)))
        {

            ezos_free(prt_inner_cb_notic);

            if (NULL != context)
                ezos_free(context);
        }
    } while (0);

    return rv;
}

mkernel_internal_error broadcast_user_event_reconnect_success()
{
    mkernel_internal_error kernel_error = mkernel_internal_succ;
    ezdev_sdk_kernel_inner_cb_notic *prt_inner_cb_notic = NULL;
    prt_inner_cb_notic = (ezdev_sdk_kernel_inner_cb_notic *)ezos_malloc(sizeof(ezdev_sdk_kernel_inner_cb_notic));
    if (NULL == prt_inner_cb_notic)
    {
        return mkernel_internal_malloc_error;
    }
    ezos_memset(prt_inner_cb_notic, 0, sizeof(ezdev_sdk_kernel_inner_cb_notic));
    prt_inner_cb_notic->cb_type = extend_cb_event;
    prt_inner_cb_notic->cb_event.event_type = SDK_KERNEL_EVENT_RECONNECT;
    prt_inner_cb_notic->cb_event.event_context = NULL;
    kernel_error = push_event_to_queue(prt_inner_cb_notic);
    return kernel_error;
}