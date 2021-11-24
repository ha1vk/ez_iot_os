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
#include "sdk_kernel_def.h"
#include "ezdev_sdk_kernel_queue.h"
#include "ezdev_sdk_kernel_extend.h"
#include "cJSON.h"

EXTERN_QUEUE_BASE_FUN
EZDEV_SDK_KERNEL_EXTEND_INTERFACE

queque_submsg g_queue_submsg;                   ///<	接收服务器消息，分发给领域模块和上层应用
queque_pubmsg_exchange g_queue_pubmsg_exchange; ///<	接收领域模块和上层应用消息，往服务器发
queque_inner_cb_notic g_queue_inner_cb_notic;   ///<	接收本地广播，分发给领域模块和上层应用

queque_submsg_v3 g_queue_submsg_v3;                   ///<	接收服务器消息V3协议, 分发给领域模块和上层应用
queque_pubmsg_exchange_v3 g_queue_pubmsg_exchange_v3; ///<	接收上层应用消息V3协议,往服务器发

QUEUE_INIT(submsg)          ///<	展开后为init_queue_submsg(EZDEV_SDK_UINT8 max_size)函数
QUEUE_INIT(pubmsg_exchange) ///<	展开后为init_queue_pubmsg_exchange(EZDEV_SDK_UINT8 max_size)函数
QUEUE_INIT(inner_cb_notic)  ///<	展开后为init_queue_inner_cb_notic(EZDEV_SDK_UINT8 max_size)函数

QUEUE_INIT(submsg_v3)          ///<	展开后为init_queue_submsg_v3(EZDEV_SDK_UINT8 max_size)函数
QUEUE_INIT(pubmsg_exchange_v3) ///<	展开后为init_queue_pubmsg_exchange_v3(EZDEV_SDK_UINT8 max_size)函数

QUEUE_FINI(submsg)          ///<	展开后为fini_queue_submsg()函数
QUEUE_FINI(pubmsg_exchange) ///<	展开后为fini_queue_pubmsg_exchange()函数
QUEUE_FINI(inner_cb_notic)  ///<	展开后为fini_queue_inner_cb_notic()函数

QUEUE_FINI(submsg_v3)          ///<	展开后为fini_queue_submsg_v3()函数
QUEUE_FINI(pubmsg_exchange_v3) ///<	展开后为fini_queue_pubmsg_exchange_v3()函数

QUEUE_POP(submsg)          ///<	展开后为pop_queue_submsg(ezdev_sdk_kernel_submsg**)函数
QUEUE_POP(pubmsg_exchange) ///<	展开后为pop_queue_pubmsg_exchange(ezdev_sdk_kernel_pubmsg_exchange**)函数
QUEUE_POP(inner_cb_notic)  ///<	展开后为pop_queue_inner_cb_notic(ezdev_sdk_kernel_inner_cb_notic**)函数
QUEUE_GET(pubmsg_exchange) ///<	展开后为get_queue_pubmsg_exchange(ezdev_sdk_kernel_pubmsg_exchange**)函数

QUEUE_POP(submsg_v3)          ///<	展开后为pop_queue_submsg_v3(ezdev_sdk_kernel_submsg_v3**)函数
QUEUE_POP(pubmsg_exchange_v3) ///<	展开后为pop_queue_pubmsg_exchange_v3(ezdev_sdk_kernel_pubmsg_exchange_v3**)函数
QUEUE_GET(pubmsg_exchange_v3) ///<	展开后为get_queue_pubmsg_exchange_v3(ezdev_sdk_kernel_pubmsg_exchange_v3**)函数

QUEUE_PUSH(submsg)          ///<	展开后为push_queue_submsg(ezdev_sdk_kernel_submsg**)函数
QUEUE_PUSH(pubmsg_exchange) ///<	展开后为push_queue_pubmsg_exchange(ezdev_sdk_kernel_pubmsg_exchange**)函数
QUEUE_PUSH(inner_cb_notic)  ///<	展开后为push_queue_inner_cb_notic(ezdev_sdk_kernel_inner_cb_notic**)函数

QUEUE_PUSH(submsg_v3)          ///<	展开后为push_queue_submsg_v3(ezdev_sdk_kernel_submsg_v3**)函数
QUEUE_PUSH(pubmsg_exchange_v3) ///<	展开后为push_queue_pubmsg_exchange_v3(ezdev_sdk_kernel_pubmsg_exchangeV3**)函数

QUEUE_PUSH_HEAD(submsg)          ///<	展开后为push_queue_head_submsg(ezdev_sdk_kernel_submsg**)函数
QUEUE_PUSH_HEAD(pubmsg_exchange) ///<	展开后为push_queue_head_pubmsg_exchange(ezdev_sdk_kernel_pubmsg_exchange**)函数
QUEUE_PUSH_HEAD(inner_cb_notic)  ///<	展开后为push_queue_head_inner_cb_notic(ezdev_sdk_kernel_inner_cb_notic**)函数

QUEUE_PUSH_HEAD(submsg_v3)          ///<	展开后为push_queue_head_submsg_v3(ezdev_sdk_kernel_submsg_v3**)函数
QUEUE_PUSH_HEAD(pubmsg_exchange_v3) ///<	展开后为push_queue_head_pubmsg_exchange_v3(ezdev_sdk_kernel_pubmsg_exchange_v3**)函数

mkernel_internal_error init_queue(EZDEV_SDK_UINT16 sub_max_size, EZDEV_SDK_UINT16 pub_max_size, EZDEV_SDK_UINT16 inner_max_size)
{
    init_queue_submsg(sub_max_size);
    init_queue_pubmsg_exchange(pub_max_size);
    init_queue_inner_cb_notic(inner_max_size);

    init_queue_submsg_v3(sub_max_size);
    init_queue_pubmsg_exchange_v3(pub_max_size);
    return mkernel_internal_succ;
}

void fini_queue()
{
    ezdev_sdk_kernel_inner_cb_notic *ptr_inner_cb_notic = NULL;
    clear_queue_submsg();
    clear_queue_submsg_v3();
    clear_queue_pubmsg_exchange();
    clear_queue_pubmsg_exchange_v3();
    do
    {
        ptr_inner_cb_notic = NULL;
        if (mkernel_internal_queue_empty == pop_queue_inner_cb_notic(&ptr_inner_cb_notic) || ptr_inner_cb_notic == NULL)
        {
            break;
        }

        destroy_inner_cb_notic(ptr_inner_cb_notic);
    } while (1);

    fini_queue_submsg();
    fini_queue_pubmsg_exchange();
    fini_queue_inner_cb_notic();

    fini_queue_submsg_v3();
    fini_queue_pubmsg_exchange_v3();
}

void destroy_inner_cb_notic(ezdev_sdk_kernel_inner_cb_notic *ptr_inner_cb_notic)
{
    if (NULL != ptr_inner_cb_notic)
    {
        if (NULL != ptr_inner_cb_notic->cb_event.event_context)
        {
            ezos_free(ptr_inner_cb_notic->cb_event.event_context);
            ptr_inner_cb_notic->cb_event.event_context = NULL;
        }

        ezos_free(ptr_inner_cb_notic);
    }
}

/*****************
ez_sdk_error pop_queue_submsg(ezdev_sdk_kernel_submsg** submsg)
{
	if (g_ezdev_sdk_kernel_queue.submsg_queque_head == NULL)
	{
		g_ezdev_sdk_kernel_queue.submsg_queque_tail = NULL;
		g_ezdev_sdk_kernel_queue.submsg_size = 0;
		return ezdev_sdk_kernel_queue_empty;
	}

	*submsg = g_ezdev_sdk_kernel_queue.submsg_queque_head->msg
	g_ezdev_sdk_kernel_queue.submsg_queque_head = g_ezdev_sdk_kernel_queue.submsg_queque_head->next;

	if (g_ezdev_sdk_kernel_queue.submsg_queque_head == NULL)
	{
		g_ezdev_sdk_kernel_queue.submsg_queque_tail = NULL;
		g_ezdev_sdk_kernel_queue.submsg_size = 0;
	}
	else
	{
		g_ezdev_sdk_kernel_queue.submsg_size--;
	}

	return EZ_KERNEL_SUCC;
}

ez_sdk_error push_queue_submsg(ezdev_sdk_kernel_submsg* submsg)
{
	ezdev_sdk_kernel_submsg_queque* submsg_queque_element = NULL;
	if (g_ezdev_sdk_kernel_queue.submsg_size >= g_ezdev_sdk_kernel_queue.submsg_maxsize)
	{
		return ezdev_sdk_kernel_queue_full;
	}
	submsg_queque_element = (ezdev_sdk_kernel_submsg_queque*)ezos_malloc(sizeof(ezdev_sdk_kernel_submsg_queque);
	if (submsg_queque_element == NULL)
	{
		return ezdev_sdk_kernel_malloc_error;
	}
	submsg_queque_element->msg = submsg;
	submsg_queque_element->next = NULL;
	if (g_ezdev_sdk_kernel_queue.pubmsg_queque_tail == NULL || g_ezdev_sdk_kernel_queue.pubmsg_queque_head)
	{
		g_ezdev_sdk_kernel_queue.pubmsg_queque_head = submsg_queque_element;
		g_ezdev_sdk_kernel_queue.submsg_queque_tail = submsg_queque_element;
	}
	else
	{
		g_ezdev_sdk_kernel_queue.submsg_queque_tail->next = submsg_queque_element;
		g_ezdev_sdk_kernel_queue.submsg_queque_tail = submsg_queque_element;
	}
	g_ezdev_sdk_kernel_queue.submsg_size++;
	return EZ_KERNEL_SUCC;
}
***********/