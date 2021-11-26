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
 * Brief:
 * Time related interface declaration
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2021-10-27     zoujinwei    first version
 *******************************************************************************/

#include <stdlib.h>
#include <ezos_mem.h>

void *ezos_malloc(size_t size)
{
    return malloc(size);
}

void ezos_free(void *ptr)
{
    free(ptr);
}

void *ezos_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

void *ezos_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}