/**
 * @file ut_getway.c
 * @author xurongjun (xurongjun@ezvizlife.com)
 * @brief ceshi 
 * @version 0.1
 * @date 2021-01-28
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <ezlog.h>
#include <kv_imp.h>
#include "utest.h"
#include <ezos.h>
#include "webclient.h"

static char *url = "http://test15fdfs.ys7.com/group1/M00/07/45/rBThLWGnY5SAfQojAAAXwtznkBY23.json";

static long global_init();

void ut_webclient_test();
UTEST_TC_EXPORT(ut_webclient_test, global_init, NULL, 60);



void ut_webclient_test(void)
{
    httpclient_t  *h_client = NULL;
    int status  = 0;
    unsigned int content_len = 0;
    int readlen=1024;
    

    h_client = webclient_session_create(1024);
    uassert_not_null(h_client);
    if(NULL == h_client){
        return;
    }
    
    status = webclient_get_position(h_client, url, 0);
    if(status!=200 && 206!=status)
    {
        ezos_printf("webclient_get_position faield,status:%d\n",status);
        return;
    }

    content_len = webclient_content_length_get(h_client);
    if(content_len <= 0)
    {
        ezos_printf("webclient_content_length_get error:%d\n",content_len);
        return;
    }

    char* recvbuffer = (char*)ezos_malloc(content_len+1);
    ezos_memset(recvbuffer, 0, content_len+1);
    webclient_read(h_client, recvbuffer, content_len);
    webclient_close(h_client);
}

static long global_init()
{
    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(CONFIG_EZIOT_UNIT_TEST_SDK_LOGLVL);

    return 0;
}
