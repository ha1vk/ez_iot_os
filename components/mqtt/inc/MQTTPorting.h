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
 * Brief:
 * mqtt adaptation interface declaration
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-09     XuRongjun    first version
 *******************************************************************************/

#include <ezos.h>

#ifndef _MQTTPORTING_H_
#define _MQTTPORTING_H_

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct MQTTNetwork
    {
        int socket_fd;
        int (*mqttread)(struct MQTTNetwork *, unsigned char *, int, int);
        int (*mqttwrite)(struct MQTTNetwork *, unsigned char *, int, int);
    } Network;

    typedef struct Timer
    {
        ezos_timespec_t end_time;
    } Timer;

    /**
     * @brief 
     * 
     * @param assign_timer 
     */
    void TimerInit(Timer *assign_timer);

    /**
     * @brief 
     * 
     * @param assign_timer 
     * @return char 
     */
    char TimerIsExpired(Timer *assign_timer);

    /**
     * @brief 
     * 
     * @param assign_timer 
     * @param time_count 
     */
    void TimerCountdownMS(Timer *assign_timer, unsigned int time_count);

    /**
     * @brief 
     * 
     * @param assign_timer 
     * @param time_count 
     */
    void TimerCountdown(Timer *assign_timer, unsigned int time_count);

    /**
     * @brief 
     * 
     * @param assign_timer 
     * @return int 
     */
    int TimerLeftMS(Timer *assign_timer);

#ifdef __cplusplus
}
#endif

#endif