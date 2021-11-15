#include "ezos_kv.h"
#include "ezos_libc.h"

static ez_kv_func_t g_kv_funcs = {0};

EZOS_API ez_void_t ezos_kv_callback_set(ez_kv_func_t *pfuncs)
{
    ezos_memcpy(&g_kv_funcs, pfuncs, sizeof(g_kv_funcs));
}
