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
#include <stdlib.h>
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
    int n;
    va_list ap;
    va_start(ap, format);
    n = vsprintf(str, format, ap);
    va_end(ap);

    return n;
}

EZOS_API int ezos_snprintf(char *str, size_t size, const char *format, ...)
{
    int n;
    va_list ap;
    va_start(ap, format);
    n = vsnprintf(str, size, format, ap);
    va_end(ap);

    return n;
}

EZOS_API int ezos_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    return vsnprintf(str, size, format, ap);
}

EZOS_API int ezos_sscanf(const char *str, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int rv = vsscanf(str, format, ap);
    va_end(ap);

    return rv;
}

EZOS_API int ezos_memcmp(const void *s1, const void *s2, size_t n)
{
    return memcmp(s1, s2, n);
}

EZOS_API void *ezos_memset(void *s, int c, size_t n)
{
    return memset(s, c, n);
}

EZOS_API void *ezos_memcpy(void *dest, const void *src, size_t n)
{
    return memcpy(dest, src, n);
}

EZOS_API void *ezos_memmove(void *dest, const void *src, size_t n)
{
    return memmove(dest, src, n);
}

EZOS_API char *ezos_strcpy(char *dest, const char *src)
{
    return strcpy(dest, src);
}

EZOS_API char *ezos_strncpy(char *dest, const char *src, size_t n)
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

EZOS_API char *ezos_strstr(const char *haystack, const char *needle)
{
    return strstr(haystack, needle);
}

EZOS_API char *ezos_strchr(const char *s, int c)
{
    return strchr(s, c);
}

EZOS_API char *ezos_strrchr(const char *s, int c)
{
    return strrchr(s, c);
}

EZOS_API size_t ezos_strlen(const char *s)
{
    return strlen(s);
}

EZOS_API int ezos_atoi(const char *nptr)
{
    return atoi(nptr);
}

EZOS_API void ezos_bzero(void *s, size_t n)
{
    bzero(s, n);
}

EZOS_API char *ezos_strdup(const char *s)
{
    return strdup(s);
}

EZOS_API long int ezos_strtol(const char *nptr, char **endptr, int base)
{
    return strtol(nptr, endptr, base);
}