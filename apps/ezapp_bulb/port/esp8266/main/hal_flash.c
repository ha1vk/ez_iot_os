/*******************************************************************************
 * Copyright © 2017-2022 Ezviz Inc.
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
 * XuRongjun (xurongjun@ezvizlife.com) - Device flash operation interface adaptation
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-14     xurongjun    first version 
 *******************************************************************************/

#include "esp_spi_flash.h"
#include <fal.h>

#define FLASH_ERASE_MIN_SIZE (4 * 1024)
// #define LOCKER_ENABLE

#ifdef LOCKER_ENABLE
#include "ezos.h"

static ez_mutex_t s_lock = NULL;

#define LOCK()                   \
    do                           \
    {                            \
        ezos_mutex_lock(s_lock); \
    } while (0)

#define UNLOCK()                   \
    do                             \
    {                              \
        ezos_mutex_unlock(s_lock); \
    } while (0)
#else
#define LOCK()
#define UNLOCK()
#endif

int hal_flash_init(void)
{
#ifdef LOCKER_ENABLE
    if (s_lock == NULL)
    {
        s_lock = ezos_mutex_create();
        assert(s_lock != NULL);
    }
#endif
    return 1;
}

int hal_flash_read(long offset, uint8_t *buf, size_t size)
{
    /* You can add your code under here. */
    int32_t ret;
    uint32_t addr = nor_flash0.addr + offset;

    LOCK();
    ret = spi_flash_read(addr, buf, size);
    UNLOCK();

    // ez_log_w("kvdb", "read:%d", ret);
    // ez_log_hexdump("kvdb", 32, buf, size);

    return ret;
}

int hal_flash_write(long offset, const uint8_t *buf, size_t size)
{
    int32_t ret;
    uint32_t addr = nor_flash0.addr + offset;

    LOCK();
    ret = spi_flash_write(addr, buf, size);
    UNLOCK();

    // ez_log_w("kvdb", "write:%d,%d", offset, ret);
    // ez_log_hexdump("kvdb", 32, buf, size);

    return ret;
}

int hal_flash_erase(long offset, size_t size)
{
    int32_t ret;
    uint32_t addr = nor_flash0.addr + offset;

    int32_t erase_size = ((size - 1) / FLASH_ERASE_MIN_SIZE) + 1;

    LOCK();
    ret = spi_flash_erase_range(addr, erase_size * FLASH_ERASE_MIN_SIZE);
    UNLOCK();
    // ez_log_w("kvdb", "erase:%d", ret);
    return ret;
}

/*
  "esp8266_onchip" : Flash 设备的名字。
  0x08000000: 对 Flash 操作的起始地址。
  1024*1024：Flash 的总大小（1MB）。
  128*1024：Flash 块/扇区大小（因为 STM32F2 各块大小不均匀，所以擦除粒度为最大块的大小：128K）。
  {init, read, write, erase} ：Flash 的操作函数。 如果没有 init 初始化过程，第一个操作函数位置可以置空。
  8 : 设置写粒度，单位 bit， 0 表示未生效（默认值为 0 ），该成员是 fal 版本大于 0.4.0 的新增成员。各个 flash 写入粒度不尽相同，可通过该成员进行设置，以下列举几种常见 Flash 写粒度：
  nor flash:  1 bit
  stm32f2/f4: 8 bit
  stm32f1:    32 bit
  stm32l4:    64 bit
 */

//1.定义 flash 设备

const struct fal_flash_dev nor_flash0 =
{
    .name = "norflash0",
    .addr = 0x0000,
    .len = 1024 * 1024 * 2,
    .blk_size = FLASH_ERASE_MIN_SIZE,
    .ops = {hal_flash_init, hal_flash_read, hal_flash_write, hal_flash_erase},
    .write_gran = 1
};