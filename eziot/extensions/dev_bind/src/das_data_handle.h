
#ifndef _H_DAS_DATA_HANDLE_H_
#define _H_DAS_DATA_HANDLE_H_

#include "ezxml.h"
#include "ezdev_sdk_kernel_struct.h"
#include "ez_base_api.h"

typedef int (*req_rsp_handle)(ezxml_t req, ezxml_t rsp);

#ifdef __cplusplus
extern "C"
{
#endif

    void extend_start_cb(EZDEV_SDK_PTR pUser);

	void extend_stop_cb(EZDEV_SDK_PTR pUser);

	void extend_data_route_cb(ezdev_sdk_kernel_submsg *ptr_submsg, EZDEV_SDK_PTR pUser);

	void extend_event_cb(ezdev_sdk_kernel_event *ptr_event, EZDEV_SDK_PTR pUser);

    void base_set_cb(ez_base_cb_t cb);

    int base_set_operation_code(const char *pcode, const int len);

    int verify_challengecode_req(ezxml_t req, ezxml_t rsp);

    int plt2pu_set_userid(ezxml_t req, ezxml_t rsp);

    int pu2plt_query_userid_req();

    int pu2plt_query_userid_rsp(void *buf, int buf_len);

    int das_req_rsp_handle(int req_cmd, void *buf, int buf_len, int rsp_cmd, const char *cmd_version, unsigned int msg_req, req_rsp_handle _handle);

    int ez_send_msg2plat(unsigned char* msg,unsigned int len, const int cmd_id, const char *cmd_version, unsigned char msg_response, unsigned int* msg_seq);

#ifdef __cplusplus
}
#endif

#endif