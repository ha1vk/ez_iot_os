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
 * yangjianglin (yangjianglin@ezvizlife.com)
*******************************************************************************/
#ifndef _EZOS_DEF_H_
#define _EZOS_DEF_H_

#if (defined(_WIN32) || defined(_WIN64))
    #if defined(EZOS_API_EXPORTS)
    #define EZOS_API __declspec(dllexport)
    #else
    #define EZOS_API __declspec(dllimport)
    #endif
    #define EZOS_CALL __stdcall
#else
    #define EZOS_API
    #define EZOS_CALL
#endif

typedef signed char         EZ_INT8;
typedef unsigned char       EZ_UINT8;
typedef unsigned char       EZ_BYTE;
typedef signed short        EZ_INT16;
typedef unsigned short      EZ_UINT16;
typedef signed int          EZ_INT32;
typedef unsigned int        EZ_UINT32;
typedef long long           EZ_INT64;
typedef unsigned long long  EZ_UINT64;

typedef void            EZ_VOID;
typedef void*           EZ_HANDLE;
typedef char            EZ_CHAR;
typedef unsigned char   EZ_UCHAR;
typedef short           EZ_SHORT;
typedef unsigned short  EZ_USHORT;
typedef int             EZ_INT;
typedef unsigned int    EZ_UINT;
typedef long            EZ_LONG;
typedef unsigned long   EZ_ULONG;

typedef int         EZ_BOOL;
typedef int         EZ_ERR;
#define EZ_TRUE     1
#define EZ_FALSE    0

#ifndef EZ_MIN
#define EZ_MIN(a,b)        ((a) > (b) ? (b) : (a))
#endif

#ifndef EZ_MAX
#define EZ_MAX(a,b)        ((a) > (b) ? (a) : (b))
#endif

///< for low case type
typedef EZ_INT8     ez_int8_t;
typedef EZ_UINT8    ez_uint8_t;
typedef EZ_BYTE     ez_byte_t;
typedef EZ_INT16    ez_int16_t;
typedef EZ_UINT16   ez_uint16_t;
typedef EZ_INT32    ez_int32_t;
typedef EZ_UINT32   ez_uint32_t;
typedef EZ_INT64    ez_int64_t;
typedef EZ_UINT64   ez_uint64_t;
typedef EZ_VOID     ez_void_t;
typedef EZ_HANDLE   ez_handle_t;
typedef EZ_CHAR     ez_char_t;
typedef EZ_UCHAR    ez_uchar_t;
typedef EZ_SHORT    ez_short_t;
typedef EZ_USHORT   ez_ushort_t;
typedef EZ_INT      ez_int_t;
typedef EZ_UINT     ez_uint_t;
typedef EZ_LONG     ez_long_t;
typedef EZ_ULONG    ez_ulong_t;

typedef EZ_BOOL     ez_bool_t;
typedef EZ_ERR      ez_err_t;
#define ez_true     1
#define ez_false    0

#endif