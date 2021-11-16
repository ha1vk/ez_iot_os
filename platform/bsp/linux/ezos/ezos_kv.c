#include <ezos_gconfig.h>
#include <ezos_kv.h>
#include <ezos_libc.h>

static ez_kv_func_t g_kv_funcs = {0};
#define CHECK_NULL_RETURN(p, rv) \
    if (NULL == p)               \
    {                            \
        return (rv);             \
    }

EZOS_API ez_void_t ezos_kv_callback_set(ez_kv_func_t *pfuncs)
{
    ezos_memcpy(&g_kv_funcs, pfuncs, sizeof(g_kv_funcs));
}

EZOS_API int ezos_kv_init(const ez_kv_default_t *default_kv)
{
    CHECK_NULL_RETURN(g_kv_funcs.ezos_kv_init, EZ_KV_ERR_INIT_FAILED);

    return g_kv_funcs.ezos_kv_init((void *)default_kv);
}

EZOS_API int ezos_kv_raw_set(const char *key, const void *value, size_t length)
{
    CHECK_NULL_RETURN(g_kv_funcs.ezos_kv_raw_set, EZ_KV_ERR_NOT_INIT);

    return g_kv_funcs.ezos_kv_raw_set(key, value, length);
}

EZOS_API int ezos_kv_raw_get(const char *key, void *value, size_t *length)
{
    CHECK_NULL_RETURN(g_kv_funcs.ezos_kv_raw_get, EZ_KV_ERR_NOT_INIT);
    CHECK_NULL_RETURN(length, EZ_KV_ERR_READ);

    return g_kv_funcs.ezos_kv_raw_get(key, value, length);
}

EZOS_API int ezos_kv_del(const char *key)
{
    CHECK_NULL_RETURN(g_kv_funcs.ezos_kv_del, EZ_KV_ERR_NOT_INIT);

    return g_kv_funcs.ezos_kv_del(key);
}

EZOS_API int ezos_kv_del_by_prefix(const char *key_prefix)
{
    CHECK_NULL_RETURN(g_kv_funcs.ezos_kv_del_by_prefix, EZ_KV_ERR_NOT_INIT);

    return g_kv_funcs.ezos_kv_del_by_prefix(key_prefix);
}

EZOS_API void ezos_kv_print()
{
    if (!g_kv_funcs.ezos_kv_print)
    {
        return;
    }

    g_kv_funcs.ezos_kv_print();
}

EZOS_API int ezos_kv_deinit()
{
    CHECK_NULL_RETURN(g_kv_funcs.ezos_kv_print, EZ_KV_ERR_NOT_INIT);

    return g_kv_funcs.ezos_kv_deinit();
}