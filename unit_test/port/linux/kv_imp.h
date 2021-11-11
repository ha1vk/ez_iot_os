/**
 * @file kv_imp.h
 * @author xurongjun (xurongjun@ezvizlife.com)
 * @brief 
 * @version 0.1
 * @date 2021-01-19
 * 
 * @copyright Copyright (c) 2021
 * 
 */

int kv_init(const void* *default_kv);

int kv_raw_set(const char* *key, char *value, unsigned int length);

int kv_raw_get(const char *key, char *value, unsigned int *length);

int kv_del(const char *key);

int kv_del_by_prefix(const unsigned char *key_prefix);

void kv_print(void);

int kv_deinit(void);