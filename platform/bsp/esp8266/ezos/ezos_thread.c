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
 * Time related interface declaration
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-27     zoujinwei    first version
 * 2021-11-24     xurongjun    Redeclare the threaded user interface and implement it
 *******************************************************************************/
#include "ezos_thread.h"
#include "ezos_time.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <string.h>

#define THREAD_STATE_JOINABLE 0
#define THREAD_STATE_DETACHED 1

typedef struct
{
    xSemaphoreHandle lock;
} sdk_mutex_platform;

typedef struct
{
    xTaskHandle thread_hd;
    char name[16];
    int state;
    ez_thread_func_t thread_fun;
    void *user_param;
} thread_data_t;

static void sdk_thread_fun(void *aArg)
{
    thread_data_t *hd = (thread_data_t *)aArg;
    if (hd == NULL)
    {
        return;
    }

    hd->thread_fun(hd->user_param);

    if (THREAD_STATE_DETACHED == hd->state)
    {
        vTaskDelete(NULL);
    }
}

EZOS_API int ezos_thread_create(ez_thread_t *const handle, const char *name, ez_thread_func_t thread_fun,
                                const void *param, unsigned int stack_size, unsigned int priority)
{
    int rc = 0;
    thread_data_t *th_handle = (thread_data_t *)malloc(sizeof(thread_data_t));
    if (th_handle == NULL)
    {
        return -1;
    }

    memset(th_handle, 0, sizeof(ez_thread_t));
    th_handle->thread_fun= thread_fun;
    th_handle->user_param = (void *)param;
    strncpy(th_handle->name, (const char *)name, sizeof(th_handle->name) - 1);
    if (NULL != handle)
    {
        th_handle->state = THREAD_STATE_JOINABLE;
    }
    else
    {
        th_handle->state = THREAD_STATE_DETACHED;
    }
    rc = xTaskCreate(sdk_thread_fun,      /* The function that implements the task. */
                     th_handle->name, /* Just a text name for the task to aid debugging. */
                     stack_size,          /* The stack size is defined in FreeRTOSIPConfig.h. */
                     (void *)th_handle,      /* The task parameter, not used in this case. */
                     priority,            /* The priority assigned to the task is defined in FreeRTOSConfig.h. */
                     &th_handle->thread_hd); /* The task handle is not used. */

    if (rc != pdPASS)
    {
        free(th_handle);
        return -1;
    }
     if (NULL != handle)
    {
       *handle = (ez_thread_t)th_handle;
    }
    return 0;
}

EZOS_API int ezos_thread_destroy(ez_thread_t handle)
{
    if (handle == NULL)
    {
        return -1;
    }

    thread_data_t *thandle = (thread_data_t *)handle;
    xTaskHandle handle_copy = thandle->thread_hd;
    free(thandle);

    if (handle_copy != 0)
    {
        vTaskDelete(handle_copy);
    }

    return 0;
}

EZOS_API ez_int32_t ezos_thread_self()
{
    return 0;
}

EZOS_API ez_mutex_t ezos_mutex_create(void)
{
    sdk_mutex_platform *ptr_mutex_platform = NULL;
    ptr_mutex_platform = (sdk_mutex_platform *)malloc(sizeof(sdk_mutex_platform));
    if (ptr_mutex_platform == NULL)
    {
        return NULL;
    }

    ptr_mutex_platform->lock = xSemaphoreCreateMutex();
    if (ptr_mutex_platform->lock == NULL)
    {
        free(ptr_mutex_platform);
        ptr_mutex_platform = NULL;
    }

    return (ez_mutex_t)ptr_mutex_platform;
}

EZOS_API int ezos_mutex_destroy(ez_mutex_t mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)mutex;
    if (ptr_mutex_platform == NULL)
    {
        return -1;
    }

    vSemaphoreDelete(ptr_mutex_platform->lock);
    free(ptr_mutex_platform);

    ptr_mutex_platform = NULL;
    return 0;
}

EZOS_API int ezos_mutex_lock(ez_mutex_t mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)mutex;
    if (ptr_mutex_platform == NULL)
    {
        return -1;
    }

    if (xSemaphoreTake(ptr_mutex_platform->lock, portMAX_DELAY) == pdTRUE)
    {
        return 0;
    }

    return -1;
}

EZOS_API int ezos_mutex_unlock(ez_mutex_t mutex)
{
    sdk_mutex_platform *ptr_mutex_platform = (sdk_mutex_platform *)mutex;
    if (ptr_mutex_platform == NULL)
    {
        return -1;
    }

    if (xSemaphoreGive(ptr_mutex_platform->lock) == pdTRUE)
    {
        return 0;
    }

    return -1;
}
