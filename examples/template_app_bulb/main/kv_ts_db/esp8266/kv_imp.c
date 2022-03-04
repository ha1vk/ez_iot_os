#include "kv_imp.h"
#include <flashdb.h>
#include "FreeRTOS.h"
#include "semphr.h"

static struct fdb_kvdb ez_kvdb;
static SemaphoreHandle_t s_lock = NULL;
#define EZ_KVDB_NAME "data"
#define EZ_KVDB_PART_NAME "ez_kvdb"
#define EZ_KVDB_MAX_SIZE 1024 * 32     //kvdb 至少需要2个sec
#define EZ_KVDB_SEC_SIZE 1024 * 16     //乐鑫底层flash 读写以4k 为块进行擦除，kv 需要是4k 的倍数

void kv_lock(fdb_db_t db)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
}

void kv_unlock(fdb_db_t db)
{
    xSemaphoreGive(s_lock);
}

int32_t kv_init_adv(fdb_kvdb_t db, const char *name, const char *part_name, struct fdb_default_kv *default_kv, int32_t max_size, int32_t sec_size)
{
    if (db->parent.init_ok) 
    {
        return 0;
    }

    if (s_lock == NULL) {
        s_lock = xSemaphoreCreateCounting(1, 1);
        assert(s_lock != NULL);
    }

    fdb_kvdb_control(db, FDB_KVDB_CTRL_SET_LOCK, (void *)kv_lock);
    fdb_kvdb_control(db, FDB_KVDB_CTRL_SET_UNLOCK, (void *)kv_unlock);
    fdb_kvdb_control(db, FDB_KVDB_CTRL_SET_MAX_SIZE, (void *)&max_size);
    fdb_kvdb_control(db, FDB_KVDB_CTRL_SET_SEC_SIZE, (void *)&sec_size);

    return fdb_kvdb_init(db, name, part_name, default_kv, NULL);
}

int kv_init(const void *default_kv)
{
    /**
     * @brief For single product devices, 16k is recommended, and for gateway devices,
     *          16*n is recommended, where n is the number of sub-devices.
     */
    int32_t max_size = EZ_KVDB_MAX_SIZE;
    int32_t sec_size = EZ_KVDB_SEC_SIZE;
    int rv = EZ_KV_ERR_SUCC;
    
    if (ez_kvdb.parent.init_ok)
    {
        return rv;
    }

    if (s_lock == NULL) {
        s_lock = xSemaphoreCreateCounting(1, 1);
        assert(s_lock != NULL);
    }

    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_LOCK, (void *)kv_lock);
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_UNLOCK, (void *)kv_unlock);
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_MAX_SIZE, (void *)&max_size);
    fdb_kvdb_control(&ez_kvdb, FDB_KVDB_CTRL_SET_SEC_SIZE, (void *)&sec_size);

    fdb_kvdb_init(&ez_kvdb, EZ_KVDB_NAME, EZ_KVDB_PART_NAME, (struct fdb_default_kv *)default_kv, NULL);

    return rv;
}

int kv_raw_set(const char *key, const void *value, size_t length)
{
    struct fdb_blob blob;

    return fdb_kv_set_blob(&ez_kvdb, (const char *)key, fdb_blob_make(&blob, value, length));
}

int kv_raw_get(const char *key, void *value, size_t *length)
{
    struct fdb_blob blob;

    size_t read_len = fdb_kv_get_blob(&ez_kvdb, (const char *)key, fdb_blob_make(&blob, value, *length));
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

int kv_del(const char *key)
{
    return fdb_kv_del(&ez_kvdb, (const char *)key);
}

int kv_del_by_prefix(const char *key_prefix)
{
    //TODO
    return EZ_KV_ERR_INIT_FAILED;
}

void kv_print(void)
{
    fdb_kv_print(&ez_kvdb);
}

int kv_deinit(void)
{
    return EZ_KV_ERR_INIT_FAILED;
    //do nothing
}