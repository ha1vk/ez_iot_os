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
 * Change Logs:
 * Date           Author       Notes
 * 2021-12-01     xurongjun    Remove redundant interfaces
 *******************************************************************************/

#ifndef _EZDEVSDK_UTILS_H
#define _EZDEVSDK_UTILS_H

#include "ezos_time.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void md5_hexdump(unsigned const char *src, int len, int upper, unsigned char *dst);

    /**
     * @brief 内部错误码转外部错误码
     * 
     * @param 内部错误码 
     * @return 外部错误码
     */
    unsigned int mkiE2ezE(unsigned int mkernel_err);

    /**
     * @brief 获取当前模块编译日期
     * 
     * @param pbuf 接收缓冲区
     */
    int get_module_build_date(char *pbuf);

    /**
     * @brief 
     * 
     * @param assign_timer 
     * @param time_ms 
     * @return char 
     */
    char ezcore_time_isexpired_bydiff(ezos_timespec_t *assign_timer, unsigned int time_ms);

    /**
     * @brief 
     * 
     * @param assign_timer 
     * @param time_count 
     */
    void ezcore_time_countdown(ezos_timespec_t *assign_timer, unsigned int time_count);

#ifdef __cplusplus
}
#endif

#endif //_EZDEVSDK_UTILS_H