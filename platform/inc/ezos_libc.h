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

#ifndef H_EZOS_LIBC_H_
#define H_EZOS_LIBC_H_

#if (defined(_WIN32) || defined(_WIN64))
#  if defined(EZOS_API_EXPORTS)
#    define EZOS_API __declspec(dllexport)
#  else
#    define EZOS_API __declspec(dllimport)
#  endif
#  define EZOS_CALL __stdcall
#elif defined(__linux__)
#  define EZOS_API
#  define EZOS_CALL
#else
#  define EZOS_API
#  define EZOS_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

EZOS_API int EZOS_CALL ezos_memcmp(const void *s1, const void *s2, size_t n);
EZOS_API void * EZOS_CALL ezos_memcpy(void *dest, const void *src, size_t n);
EZOS_API char * EZOS_CALL ezos_strcpy(char *dest, const char *src);
EZOS_API char * EZOS_CALL ezos_strncpy(char *dest, const char *src, size_t n);
EZOS_API int EZOS_CALL ezos_strcmp(const char *s1, const char *s2);
EZOS_API int EZOS_CALL ezos_strncmp(const char *s1, const char *s2, size_t n);
EZOS_API char * EZOS_CALL ezos_strstr(const char *haystack, const char *needle);
EZOS_API size_t EZOS_CALL ezos_strlen(const char *s);

#ifdef __cplusplus
}
#endif

#endif//H_EZOS_LIBC_H_
