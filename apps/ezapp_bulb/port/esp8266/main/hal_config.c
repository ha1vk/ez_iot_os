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
 * Contributors:
 * XuRongjun (xurongjun@ezvizlife.com) - Device configuration interface implement
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-14     xurongjun    first version 
 *******************************************************************************/

#include "hal_config.h"
#include "ezos.h"
#include "ezlog.h"
#include <flashdb.h>

ez_bool_t is_init_ok = ez_false; //initialized successfully

#define CHECK_INIT_OK(rv)                          \
    if (!is_init_ok)                               \
    {                                              \
        ezlog_e(TAG_APP, "hal config not inited"); \
        return (rv);                               \
    }

#define CHECK_NULL(val, rv)                           \
    if (NULL == (val))                                \
    {                                                 \
        ezlog_e(TAG_APP, "hal config param invalid"); \
        return (rv);                                  \
    }

static struct fdb_kvdb sdk_kvdb;
static struct fdb_kvdb app_kvdb;
static ez_mutex_t m_kv_mutex = NULL;

#define EZ_KVDB_NAME "data"
#define EZ_APP_KVDB_PART_NAME "app_kvdb"
#define EZ_SDK_KVDB_PART_NAME "sdk_kvdb"

// for ezhal_kv interface adaptation
static int kv_init(const void *default_kv);
static int kv_raw_set(const char *key, const void *value, size_t length);
static int kv_raw_get(const char *key, void *value, size_t *length);
static int kv_del(const char *key);
static int kv_del_by_prefix(const char *key_prefix);
static void kv_print(void);
static int kv_deinit(void);

// kv implement
static int kv_init_ex(fdb_kvdb_t db, const char *part_name, const void *default_kv);
static int kv_raw_set_ex(fdb_kvdb_t db, const char *key, const void *value, size_t length);
static int kv_raw_get_ex(fdb_kvdb_t db, const char *key, void *value, size_t *length);
static int kv_del_ex(fdb_kvdb_t db, const char *key);
static int kv_del_by_prefix_ex(fdb_kvdb_t db, const char *key_prefix);
static void kv_print_ex(fdb_kvdb_t db);
static int kv_deinit_ex(fdb_kvdb_t db);

ez_bool_t hal_config_init(ez_void_t)
{
    ez_kv_func_t kv_func = {
        .ezos_kv_init = kv_init,
        .ezos_kv_raw_set = kv_raw_set,
        .ezos_kv_raw_get = kv_raw_get,
        .ezos_kv_del = kv_del,
        .ezos_kv_del_by_prefix = kv_del_by_prefix,
        .ezos_kv_print = kv_print,
        .ezos_kv_deinit = kv_deinit,
    };

    if (is_init_ok)
    {
        return ez_true;
    }

    m_kv_mutex = ezos_mutex_create();
    if (0 != kv_init_ex(&app_kvdb, EZ_APP_KVDB_PART_NAME, NULL))
    {
        return ez_false;
    }

    // 初始化flash
    extern int hal_flash_init(void);
    hal_flash_init();

    is_init_ok = ez_true;
    ezos_kv_callback_set(&kv_func);

    return ez_true;
}

ez_bool_t hal_config_product_load(ez_char_t *val, ez_int32_t len)
{
    CHECK_INIT_OK(ez_false);
    CHECK_NULL(val, ez_false);

    // 读取设备产品配置文件
    extern int hal_flash_read(long offset, uint8_t *buf, size_t size);
    const ez_long_t offset = 0x11000;
    if (0 != hal_flash_read(offset, (ez_void_t *)val, (size_t)len))
    {
        return ez_false;
    }

    return ez_true;
}

ez_bool_t hal_config_lic_load(ez_char_t *val, ez_int32_t len)
{
    CHECK_INIT_OK(ez_false);
    CHECK_NULL(val, ez_false);

    // 读取设备license
    extern int hal_flash_read(long offset, uint8_t *buf, size_t size);
    const ez_int32_t offset = 0x10000;
    if (0 != hal_flash_read(offset, (ez_void_t *)val, (size_t)len))
    {
        return ez_false;
    }

    return ez_true;
}

ez_bool_t hal_config_get_int(const ez_char_t *key, ez_int32_t *val, ez_int32_t _defval)
{
    CHECK_INIT_OK(ez_false);
    CHECK_NULL(key, ez_false);
    CHECK_NULL(val, ez_false);

    size_t length = sizeof(ez_int32_t);
    kv_raw_get_ex(&app_kvdb, key, (void *)val, &length);
    if (0 == length)
    {
        *val = _defval;
    }

    return ez_true;
}

ez_bool_t hal_config_set_int(const ez_char_t *key, ez_int32_t val)
{
    CHECK_INIT_OK(ez_false);
    CHECK_NULL(key, ez_false);

    if (FDB_NO_ERR != kv_raw_set_ex(&app_kvdb, key, (const void *)&val, sizeof(val)))
    {
        return ez_false;
    }

    return ez_true;
}

ez_bool_t hal_config_get_double(const ez_char_t *key, ez_double_t *val, ez_double_t _defval)
{
    CHECK_INIT_OK(ez_false);
    CHECK_NULL(key, ez_false);
    CHECK_NULL(val, ez_false);

    size_t length = sizeof(ez_double_t);
    kv_raw_get_ex(&app_kvdb, key, (void *)val, &length);
    if (0 == length)
    {
        *val = _defval;
    }

    return ez_true;
}

ez_bool_t hal_config_set_double(const ez_char_t *key, ez_double_t val)
{
    CHECK_INIT_OK(ez_false);
    CHECK_NULL(key, ez_false);

    if (FDB_NO_ERR != kv_raw_set_ex(&app_kvdb, key, (const void *)&val, sizeof(val)))
    {
        return ez_false;
    }

    return ez_true;
}

ez_bool_t hal_config_get_string(const ez_char_t *key, ez_char_t *val, ez_int32_t *len, const ez_char_t *_defval)
{
    CHECK_INIT_OK(ez_false);
    CHECK_NULL(key, ez_false);
    CHECK_NULL(len, ez_false);

    size_t length = (size_t)*len;
    kv_raw_get_ex(&app_kvdb, key, val, &length);

    if (NULL == val)
    {
        *len = (ez_int32_t)length;
    }

    if (0 == len && NULL != val)
    {
        ezos_strncpy(val, _defval, *len);
        *len = ezos_strlen(_defval);
    }

    return ez_true;
}

ez_bool_t hal_config_set_string(const ez_char_t *key, const ez_char_t *val)
{
    CHECK_INIT_OK(ez_false);
    CHECK_NULL(key, ez_false);
    CHECK_NULL(val, ez_false);

    if (FDB_NO_ERR != kv_raw_set_ex(&app_kvdb, key, (const void *)&val, ezos_strlen(val)))
    {
        return ez_false;
    }

    return ez_true;
}

ez_bool_t hal_config_reset_factory(ez_void_t)
{
    CHECK_INIT_OK(ez_false);

    return ez_true;
}

ez_void_t hal_config_print()
{
    if (!is_init_ok)
    {
        return;
    }

    kv_print_ex(&app_kvdb);
}

static void kv_lock(void)
{
    ezos_mutex_lock(m_kv_mutex);
}

static void kv_unlock(void)
{
    ezos_mutex_unlock(m_kv_mutex);
}

static int kv_init(const void *default_kv)
{
    return kv_init_ex(&sdk_kvdb, EZ_SDK_KVDB_PART_NAME, default_kv);
}

static int kv_raw_set(const char *key, const void *value, size_t length)
{
    return kv_raw_set_ex(&sdk_kvdb, key, value, length);
}

static int kv_raw_get(const char *key, void *value, size_t *length)
{
    return kv_raw_get_ex(&sdk_kvdb, key, value, length);
}

static int kv_del(const char *key)
{
    return kv_del_ex(&sdk_kvdb, key);
}

static int kv_del_by_prefix(const char *key_prefix)
{
    return kv_del_by_prefix_ex(&sdk_kvdb, key_prefix);
}

static void kv_print(void)
{
    return kv_print_ex(&sdk_kvdb);
}

static int kv_deinit(void)
{
    return kv_deinit_ex(&sdk_kvdb);
}

static int kv_init_ex(fdb_kvdb_t db, const char *part_name, const void *default_kv)
{
    if (db->parent.init_ok)
    {
        return 0;
    }

    fdb_kvdb_control(db, FDB_KVDB_CTRL_SET_LOCK, (void *)kv_lock);
    fdb_kvdb_control(db, FDB_KVDB_CTRL_SET_UNLOCK, (void *)kv_unlock);

    return fdb_kvdb_init(db, EZ_KVDB_NAME, part_name, (struct fdb_default_kv *)default_kv, NULL);
}

static int kv_raw_set_ex(fdb_kvdb_t db, const char *key, const void *value, size_t length)
{
    struct fdb_blob blob;

    return fdb_kv_set_blob(db, (const char *)key, fdb_blob_make(&blob, value, length));
}

static int kv_raw_get_ex(fdb_kvdb_t db, const char *key, void *value, size_t *length)
{
    struct fdb_blob blob;

    size_t read_len = fdb_kv_get_blob(db, key, fdb_blob_make(&blob, value, *length));
    if (NULL == value)
    {
        *length = read_len;
        return EZ_KV_ERR_SUCC;
    }

    *length = read_len;
    return EZ_KV_ERR_SUCC;
}

static int kv_del_ex(fdb_kvdb_t db, const char *key)
{
    return fdb_kv_del(db, key);
}

static int kv_del_by_prefix_ex(fdb_kvdb_t db, const char *key_prefix)
{
    //TODO
    return EZ_KV_ERR_WRITE;
}

static void kv_print_ex(fdb_kvdb_t db)
{
    fdb_kv_print(db);
}

static int kv_deinit_ex(fdb_kvdb_t db)
{
    fdb_kvdb_deinit(db);

    return EZ_KV_ERR_SUCC;
}