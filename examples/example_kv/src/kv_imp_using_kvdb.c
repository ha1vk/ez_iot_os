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
 * XuRongjun (xurongjun@ezvizlife.com) - Implementation of ezos kv interface, adapted to kvdb library by file mode.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-16     xurongjun    first version 
 *******************************************************************************/

#include "ezos.h"
#include "flashdb.h"

static ez_mutex_t m_kv_mutex = NULL;
static struct fdb_kvdb ez_kvdb;
#define EZ_KVDB_NAME "app_kvdb"
#define EZ_KVDB_PART_NAME "cache"
#define EZ_KVDB_MAX_SIZE 1024 * 64
#define EZ_KVDB_SEC_SIZE 1024 * 32

static void example_kv_lock(fdb_db_t db)
{
    ezos_mutex_lock(m_kv_mutex);
}

static void example_kv_unlock(fdb_db_t db)
{
    ezos_mutex_unlock(m_kv_mutex);
}

int example_kv_init(const void *default_kv)
{
    /**
     * @brief For single product devices, 16k is recommended, and for gateway devices,
     *          16*n is recommended, where n is the number of sub-devices.
     */
    bool file_mode = true;
    int max_size = EZ_KVDB_MAX_SIZE;
    int sec_size = EZ_KVDB_SEC_SIZE;
    int rv = EZ_KV_ERR_SUCC;

    if (ez_kvdb.parent.init_ok)
    {
        return rv;
    }

    m_kv_mutex = ezos_mutex_create();
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_LOCK, (void *)example_kv_lock);
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_UNLOCK, (void *)example_kv_unlock);
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_FILE_MODE, (void *)&file_mode);
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_MAX_SIZE, (void *)&max_size);
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_SEC_SIZE, (void *)&sec_size);

    rv = fdb_kvdb_init(&ez_kvdb, EZ_KVDB_NAME, EZ_KVDB_PART_NAME, (struct fdb_default_kv *)default_kv, NULL);

    return rv;
}

int example_kv_raw_set(const char *key, const void *value, size_t length)
{
    struct fdb_blob blob;

    return fdb_kv_set_blob(&ez_kvdb, key, fdb_blob_make(&blob, value, length));
}

int example_kv_raw_get(const char *key, void *value, size_t *length)
{
    struct fdb_blob blob;

    size_t read_len = fdb_kv_get_blob(&ez_kvdb, key, fdb_blob_make(&blob, value, *length));
    if (read_len < 0)
    {
        return EZ_KV_ERR_READ;
    }

    if (NULL == value)
    {
        *length = read_len;
        return EZ_KV_ERR_SUCC;
    }

    *length = read_len;
    return EZ_KV_ERR_SUCC;
}

int example_kv_del(const char *key)
{
    return fdb_kv_del(&ez_kvdb, (const char *)key);
}

int example_kv_del_by_prefix(const char *key_prefix)
{
    //TODO
    return EZ_KV_ERR_SUCC;
}

void example_kv_print(void)
{
    fdb_kv_print(&ez_kvdb);
}

int example_kv_deinit(void)
{
    fdb_kvdb_deinit(&ez_kvdb);
    ezos_mutex_destroy(m_kv_mutex);

    return EZ_KV_ERR_SUCC;
}