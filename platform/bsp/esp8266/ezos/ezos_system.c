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
 * xurongjun (xurongjun@ezvizlife.com)
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-25     xurongjun    first version
*******************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include "ezos_system.h"
#include "esp_system.h"

long int ezos_rand(void)
{
    return rand();
}

int ezos_get_uuid(char *uuid, short len)
{
    return esp_base_mac_addr_get((ez_uint8_t *)uuid);
}

void ezos_reboot(void)
{
    esp_restart();
}