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
* 2021-11-02     zoujinwei    first version
*******************************************************************************/

#ifndef H_EZOS_MEM_H_
#define H_EZOS_MEM_H_

#include <ezos_def.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
    * @brief 申请动态内存空间 
    *
    * @param[in] size  需要申请的内存大小 .
    * @return  成功返回内存地址 失败返回NULL
    */
    EZOS_API void *ezos_malloc(size_t size);

    /**
    * @brief 释放动态内存空间
    *
    * @param[in] ptr  动态内存地址 .
    */
    EZOS_API void ezos_free(void *ptr);

    /**
    * @brief 申请动态内存空间
    *
    * @param[in] nmemb  需要申请的内存块数量 .
    * @param[in] size  需要申请的每块内存大小 .
    * @return  成功返回内存地址 失败返回NULL
    */
    EZOS_API void *ezos_calloc(size_t nmemb, size_t size);

    /**
    * @brief 重新申请动态内存空间，并释放之前申请的内存
    *
    * @param[in] ptr   之前申请的动态内存地址 .
    * @param[in] size  需要申请的内存大小 .
    * @return  成功返回内存地址 失败返回NULL
    */
    EZOS_API void *ezos_realloc(void *ptr, size_t size);

#ifdef __cplusplus
}
#endif

#endif //H_EZOS_MEM_H_
