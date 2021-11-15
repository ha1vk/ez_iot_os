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

#include "ezos_libc.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

EZOS_API int ezos_printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int rv = vprintf(format, ap);
    va_end(ap);

    return rv;
}

EZOS_API int ezos_sprintf(char *str, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int rv = sprintf(str, format, ap);
    va_end(ap);

    return rv;
}

EZOS_API int ezos_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int rv = snprintf(str, size, format, ap);
    va_end(ap);

    return rv;
}

EZOS_API int ezos_memcmp(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}

EZOS_API void *EZOS_CALL ezos_memcpy(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

EZOS_API char *EZOS_CALL ezos_strcpy(char *dest, const char *src)
{
    return strcpy(dest, src);
}

EZOS_API char *EZOS_CALL ezos_strncpy(char *dest, const char *src, size_t n)
{
    return strncpy(dest, src, n);
}

EZOS_API int ezos_strcmp(const char *s1, const char *s2)
{
    return strcmp(s1, s2);
}

EZOS_API int ezos_strncmp(const char *s1, const char *s2, size_t n)
{
    return strncmp(s1, s2, n);
}

EZOS_API char *EZOS_CALL ezos_strstr(const char *haystack, const char *needle)
{
    return strstr(haystack, needle);
}

EZOS_API size_t ezos_strlen(const char *s)
{
    return strlen(s);
}
