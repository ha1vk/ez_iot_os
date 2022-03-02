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
#include <flashdb.h>
#include <stdint.h>
#include <ez_iot.h>

kv_err_e ts_init();
/*模拟fwrite 的apend写方式*/
kv_err_e ts_fwrite_append(int8_t *pStr, uint32_t length);
/*读出所有的记录，fileBuf 最大长度*/
kv_err_e ts_fread_all();

kv_err_e ts_to_websession(void *arg);