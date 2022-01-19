#include "ezos.h"
#include "ezlog.h"
#include "cli.h"

#define KV_NAME_COUNTER "kv_init_counter"
#define KV_NAME_STR "kv_str_data"
#define KV_NAME_RAW "kv_raw_data"

extern int example_kv_init(const void *default_kv);
extern int example_kv_raw_set(const char *key, const void *value, size_t length);
extern int example_kv_raw_get(const char *key, void *value, size_t *length);
extern int example_kv_del(const char *key);
extern int example_kv_del_by_prefix(const char *key_prefix);
extern void example_kv_print(void);
extern int example_kv_deinit(void);

static ez_kv_func_t g_kv_func = {
    .ezos_kv_init = example_kv_init,
    .ezos_kv_raw_set = example_kv_raw_set,
    .ezos_kv_raw_get = example_kv_raw_get,
    .ezos_kv_del = example_kv_del,
    .ezos_kv_del_by_prefix = example_kv_del_by_prefix,
    .ezos_kv_print = example_kv_print,
    .ezos_kv_deinit = example_kv_deinit,
};

static int g_kv_init_counter = 0;
static char* g_kv_str_data = "ezapp, easy your life!";
static char g_kv_raw_data[24] = {0x00, 0x11, 0x22, 0x33, 0x00, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xff, 0xee};

static ez_kv_default_node_t default_node_table[] = {
    {KV_NAME_COUNTER, (void *)&g_kv_init_counter, sizeof(g_kv_init_counter)},
    {KV_NAME_RAW, g_kv_raw_data, sizeof(g_kv_raw_data)}};

static ez_kv_default_t default_kvs = {
    .kvs = default_node_table,
    .num = sizeof(default_node_table) / sizeof(ez_kv_default_node_t),
};

static void example_kv(char *r_buf, int len, int argc, char **argv)
{
    char buf[32] = {0};
    int counter = 0;
    size_t length;

    ezlog_init();
    ezlog_start();
    ezlog_filter_lvl(EZ_ELOG_LVL_DEBUG);

    /* 设置kv回调函数并初始化 */
    ezos_kv_callback_set(&g_kv_func);
    ezos_kv_init(&default_kvs);

    /* 读取默认键值对(整形)，修改后回写 */
    length = sizeof(counter);
    ezos_kv_raw_get(KV_NAME_COUNTER, &counter, &length);
    ezlog_d(TAG_APP, "%s:%d", KV_NAME_COUNTER, counter++);
    ezos_kv_raw_set(KV_NAME_COUNTER, &counter, length);

    /* 读取默认键值对(二进制)，打印输出 */
    length = sizeof(buf);
    ezos_kv_raw_get(KV_NAME_RAW, buf, &length);
    ezlog_d(TAG_APP, "%s:", KV_NAME_RAW);
    ezlog_hexdump(TAG_APP, 16, (ez_uint8_t*)buf, length);

    /* 追加键值对，读取并删除*/
    length = ezos_strlen(g_kv_str_data);
    ezos_kv_raw_set(KV_NAME_STR, g_kv_str_data, length);

    ezos_bzero(buf, sizeof(buf));
    ezos_kv_raw_get(KV_NAME_STR, buf, &length);
    ezlog_d(TAG_APP, "%s:%s", KV_NAME_STR, buf);
    ezos_kv_del(KV_NAME_STR);

    ezlog_stop();
    ezos_kv_deinit();
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(example_kv, eziot example kv);
#else
EZOS_CLI_EXPORT("example_kv", "kv test", example_kv);
#endif