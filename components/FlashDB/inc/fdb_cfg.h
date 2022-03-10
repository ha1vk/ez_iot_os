/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-03     xurongjun    Configuration items adapt to ezos kconfig 
 */

/**
 * @file
 * @brief configuration file
 */

#ifndef _FDB_CFG_H_
#define _FDB_CFG_H_

#include <ezos.h>

/* using KVDB feature */
#ifdef CONFIG_EZIOT_COMPONENT_FLASHDB_USING_KVDB
#define FDB_USING_KVDB
#endif

#ifdef FDB_USING_KVDB
/* Auto update KV to latest default when current KVDB version number is changed. @see fdb_kvdb.ver_num */
#ifdef CONFIG_EZIOT_COMPONENT_FLASHDB_KV_AUTO_UPDATE
#define FDB_KV_AUTO_UPDATE
#endif
#endif

/* using TSDB (Time series database) feature */
#ifdef CONFIG_EZIOT_COMPONENT_FLASHDB_USING_TSDB
#define FDB_USING_TSDB
#endif

#ifdef CONFIG_EZIOT_COMPONENT_FLASHDB_USING_FAL_MODE
/* Using FAL storage mode */
#define FDB_USING_FAL_MODE
#elif CONFIG_EZIOT_COMPONENT_FLASHDB_USING_FILE_LIBC_MODE
/* Using file storage mode by LIBC file API, like fopen/fread/fwrte/fclose */
#define FDB_USING_FILE_LIBC_MODE
#elif CONFIG_EZIOT_COMPONENT_FLASHDB_USING_FILE_POSIX_MODE
/* Using file storage mode by POSIX file API, like open/read/write/close */
#define FDB_USING_FILE_POSIX_MODE
#endif

#ifdef FDB_USING_FAL_MODE
/* the flash write granularity, unit: bit
 * only support 1(nor flash)/ 8(stm32f2/f4)/ 32(stm32f1) */
#ifdef CONFIG_EZIOT_COMPONENT_FLASHDB_WRITE_GRAN_1BIT
#define FDB_WRITE_GRAN                1
#elif CONFIG_EZIOT_COMPONENT_FLASHDB_WRITE_GRAN_8BIT
#define FDB_WRITE_GRAN                8
#elif CONFIG_EZIOT_COMPONENT_FLASHDB_WRITE_GRAN_32BIT
#define FDB_WRITE_GRAN                32
#else
#error "the flash write granularity only support 1(nor flash)/ 8(stm32f2/f4)/ 32(stm32f1)"
#endif /* ifdef FDB_USING_FAL_MODE */

#endif

#ifdef CONFIG_EZIOT_COMPONENT_FLASHDB_BIG_ENDIAN
/* MCU Endian Configuration, default is Little Endian Order. */
#define FDB_BIG_ENDIAN
#endif


/* log print macro. default EF_PRINT macro is printf() */
#define FDB_PRINT(...)              ezos_printf(__VA_ARGS__)

#ifdef CONFIG_EZIOT_COMPONENT_FLASHDB_DEBUG_ENABLE
/* print debug information */
#define FDB_DEBUG_ENABLE
#endif

#endif /* _FDB_CFG_H_ */
