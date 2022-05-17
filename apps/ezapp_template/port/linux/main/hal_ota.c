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
 * XuRongjun (xurongjun@ezvizlife.com) - Device ota abstract interface declaration
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-16     xurongjun    first version 
 *******************************************************************************/

#include "ezlog.h"
#include <stdio.h>

static FILE *g_update_handle = NULL;

// OTA_CODE_NONE: 0x00
// OTA_CODE_MEM: 0X00500039
// OTA_CODE_DAMAGE: 0X00500038
// OTA_CODE_GENNERAL: 0X0050003B
ez_err_t hal_ota_begin(ez_size_t image_size)
{
    ez_err_t rv = 0x00;
    FILE *pfile  = fopen("ezapp_ota" , "w+");

    if (NULL == pfile)
    {
        rv = 0X0050003B;
    }

    return rv;
}

// OTA_CODE_NONE: 0x00
// OTA_CODE_BURN: 0X0050003A
// OTA_CODE_GENNERAL: 0X0050003B
ez_err_t hal_ota_write(const ez_void_t *data, ez_size_t size)
{
    ez_err_t rv = 0x00;

    if(size !=  fwrite(data, size, 1, g_update_handle))
    {
        rv = 0X0050003A;
    }

    return rv;
}

// OTA_CODE_NONE: 0x00
// OTA_CODE_DIGEST: 0X00500036
// OTA_CODE_SIGN: 0X00500037
// OTA_CODE_DAMAGE: 0X00500038
// OTA_CODE_GENNERAL: 0X0050003B
ez_err_t hal_ota_end(ez_void_t)
{
    ez_err_t rv = 0x00;

    if(0 != fclose(g_update_handle))
    {
        rv = 0X0050003B;
    }

    return rv;
}

// OTA_CODE_NONE: 0x00
// OTA_CODE_GENNERAL: 0X0050003B
ez_err_t hal_ota_action(ez_void_t)
{
    ez_err_t rv = 0x00;

    system("mv -f ezapp.bin ezapp.bin.old");
    system("mv -f ezapp_ota ezapp.bin");

    return rv;
}