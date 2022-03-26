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
* 
* 
* Change Logs:
* Date           Author       Notes
* 2021-01-20     xurongjun    first vision
*******************************************************************************/
#ifndef _EZOS_KV_H_
#define _EZOS_KV_H_

#include <stddef.h>
#include <ezos_def.h>

#define EZ_KV_DEFALUT_KEY_DEVID "devid"
#define EZ_KV_DEFALUT_KEY_MASTERKEY "masterkey"
#define EZ_KV_DEFALUT_KEY_TSLMAP "tslmap"
#define EZ_KV_DEFALUT_KEY_HUBLIST "hublist"

#ifdef __cplusplus
extern "C"
{
#endif

    /* kv error code */
    typedef enum
    {
        EZ_KV_ERR_SUCC,
        EZ_KV_ERR_ERASE,
        EZ_KV_ERR_READ,
        EZ_KV_ERR_WRITE,
        EZ_KV_ERR_NOT_INIT,
        EZ_KV_ERR_NAME,
        EZ_KV_ERR_NAME_EXIST,
        EZ_KV_ERR_SAVED_FULL,
        EZ_KV_ERR_INIT_FAILED,
    } ez_kv_err_e;

    typedef struct
    {
        char *key;
        void *value;
        size_t value_len;
    } ez_kv_default_node_t;

    typedef struct
    {
        ez_kv_default_node_t *kvs;
        size_t num;
    } ez_kv_default_t;

    /**
     * The KV database initialization.
     *
     * @param default_kv the default KV
     * @return int
     */
    EZOS_API int ezos_kv_init(const ez_kv_default_t *default_kv);

    /**
     * Set a raw KV. If it value is NULL, delete it.
     * If not find it in flash, then create it.
     *
     * @param key KV name
     * @param value KV value
     * @param length KV value size
     * @return int
     */
    EZOS_API int ezos_kv_raw_set(const char *key, const void *value, size_t length);

    /**
     * Get a raw KV value by key name.
     *
     * @param key KV name
     * @param value KV value
     * @param length KV value length.If it value is NULL, get the length and return 0(success).
     * @return int
     */
    EZOS_API int ezos_kv_raw_get(const char *key, void *value, size_t *length);

    /**
     * Delete an KV.
     *
     * @param key KV name
     * @return int
     */
    EZOS_API int ezos_kv_del(const char *key);

    /**
     * @brief 
     * 
     * @param key_prefix 
     * @return EZOS_API 
     */
    EZOS_API int ezos_kv_del_by_prefix(const char *key_prefix);

    /**
     * Print all KV.
     */
    EZOS_API void ezos_kv_print();

    /**
     * The KV database finalization.
     */
    EZOS_API int ezos_kv_deinit();

    typedef struct
    {
        /* encapsulated member functions */
        int (*ezos_kv_init)(const void *default_kv);
        int (*ezos_kv_raw_set)(const char *key, const void *value, size_t length);
        int (*ezos_kv_raw_get)(const char *key, void *value, size_t *length);
        int (*ezos_kv_del)(const char *key);
        int (*ezos_kv_del_by_prefix)(const char *key_prefix);
        void (*ezos_kv_print)(void);
        int (*ezos_kv_deinit)(void);
    } ez_kv_func_t;

    EZOS_API void ezos_kv_callback_set(ez_kv_func_t *pfuncs);

#ifdef __cplusplus
}
#endif

#endif