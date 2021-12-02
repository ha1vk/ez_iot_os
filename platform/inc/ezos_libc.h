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
 * 2021-11-15     xurongjun    Remove redundant functions
 * 2021-11-25     zoujinwei    add functions
 *******************************************************************************/

#ifndef H_EZOS_LIBC_H_
#define H_EZOS_LIBC_H_

#include <stdio.h>
#include <ezos_def.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

    EZOS_API int ezos_printf(const char *format, ...);
    EZOS_API int ezos_sprintf(char *str, const char *format, ...);
    EZOS_API int ezos_snprintf(char *str, size_t size, const char *format, ...);
    EZOS_API int ezos_vsnprintf(char *str, size_t size, const char *format, va_list ap);
    EZOS_API int ezos_sscanf(const char *str, const char *format, ...);

    EZOS_API int ezos_memcmp(const void *s1, const void *s2, size_t n);
    EZOS_API void *ezos_memset(void *s, int c, size_t n);
    EZOS_API void *ezos_memcpy(void *dest, const void *src, size_t n);
    EZOS_API void *ezos_memmove(void *dest, const void *src, size_t n);

    EZOS_API char *ezos_strcpy(char *dest, const char *src);
    EZOS_API char *ezos_strncpy(char *dest, const char *src, size_t n);
    EZOS_API int ezos_strcmp(const char *s1, const char *s2);
    EZOS_API int ezos_strncmp(const char *s1, const char *s2, size_t n);
    EZOS_API char *ezos_strstr(const char *haystack, const char *needle);
    EZOS_API char *ezos_strchr(const char *s, int c);
    EZOS_API char *ezos_strrchr(const char *s, int c);
    EZOS_API size_t ezos_strlen(const char *s);

    EZOS_API int ezos_atoi(const char *nptr);
    EZOS_API void ezos_bzero(void *s, size_t n);
    EZOS_API char *ezos_strdup(const char *s);

    EZOS_API void ezos_va_start(va_list ap, const char *last, ...);
    EZOS_API void ezos_va_end(va_list ap);


#ifdef __cplusplus
}
#endif

#endif //H_EZOS_LIBC_H_
