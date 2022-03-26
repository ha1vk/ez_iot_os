// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _OSAL_H_
#define _OSAL_H_

#include <unistd.h>
#include <stdint.h>
#include "ezos_thread.h"
#include "ezos_time.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define OS_SUCCESS EZHTTPD_ERRNO_SUCC
#define OS_FAIL EZHTTPD_ERRNO_FAIL

    typedef void *othread_t;

    static inline int httpd_os_thread_create(othread_t *thread,
                                             const char *name, uint16_t stacksize, int prio,
                                             void (*thread_routine)(void *arg), void *arg)
    {
        if (0 == ezos_thread_create(NULL, name, thread_routine, arg, stacksize, prio))
        {
            return OS_SUCCESS;
        }

        return OS_FAIL;
    }

    /* Only self delete is supported */
    static inline void httpd_os_thread_delete(othread_t thread)
    {
        ezos_thread_destroy(thread);
    }

    static inline void httpd_os_thread_sleep(int msecs)
    {
        ezos_delay_ms(msecs);
    }

    // static inline othread_t httpd_os_thread_handle()
    // {
    //     return xTaskGetCurrentTaskHandle();
    // }

#ifdef __cplusplus
}
#endif

#endif /* ! _OSAL_H_ */
