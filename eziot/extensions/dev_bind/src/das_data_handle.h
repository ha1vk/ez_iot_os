
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
#ifndef _H_DAS_DATA_HANDLE_H_
#define _H_DAS_DATA_HANDLE_H_

#include "ezxml.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ez_base_api.h"

typedef int (*req_rsp_handle)(ezxml_t req);

#ifdef __cplusplus
extern "C"
{
#endif

    void extend_start_cb(EZDEV_SDK_PTR pUser);

	void extend_stop_cb(EZDEV_SDK_PTR pUser);

	void extend_data_route_cb(ezdev_sdk_kernel_submsg *ptr_submsg, EZDEV_SDK_PTR pUser);

	void extend_event_cb(ezdev_sdk_kernel_event *ptr_event, EZDEV_SDK_PTR pUser);

    void base_set_cb(ez_base_cb_t cb);

    ez_base_err base_set_operation_code(const char *pcode, const int len);

    int verify_challengecode_req(ezxml_t req);

    int plt2pu_set_userid(ezxml_t req);

    int pu2plt_query_userid_req();

    int pu2plt_query_userid_rsp(void *buf, int buf_len);

    ez_base_err base_report_bind_token(const ez_bind_token_t *ptoken);

    int das_req_rsp_handle(int req_cmd, void *buf, int buf_len, int rsp_cmd, const char *cmd_version, unsigned int msg_req, req_rsp_handle _handle);

    int ez_send_msg2plat(unsigned char* msg,unsigned int len, const int cmd_id, const char *cmd_version, 
                         unsigned char msg_response, unsigned int* msg_seq,unsigned int qos);

#ifdef __cplusplus
}
#endif

#endif