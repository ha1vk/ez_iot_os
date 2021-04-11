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
 *******************************************************************************/
#if defined(__linux__)
#include <unistd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)

#ifdef BSPATCH_EXPORTS
#define BSPATCH_API __declspec(dllexport)
#else
#define BSPATCH_API __declspec(dllimport)
#endif

#define CALLBACK __stdcall

#else
#define BSPATCH_API

#define CALLBACK
#define __stdcall

#endif


typedef enum
{
    EZBS_NEW_DATA_BUF_FULL = -1,   ///< recv buf was full,need clean
    EZBS_SUCESS = 0,               ///< success
    EZBS_GENERAL = 1,              ///< general err
    EZBS_INVALID_PARAM = 2,        ///< invalid param
    EZBS_INVALID_PATCH_TYPE = 3,   ///< invalid patch type
    EZBS_UNEXPECTED_HEADER = 4,    ///< invalid head data
    EZBS_UNEXPECTED_CDATA = 5,     ///< ctrl block  invaild
    EZBS_UNEXPECTED_DDATA = 6,     ///< diff block invaild
    EZBS_UNEXPECTED_EDATA = 7,     ///< extra blcok invaild
    EZBS_UNEXPECTED_RDATA_LEN = 8, ///< unexpected par patch size
    EZBS_OLDFILE_OPEN = 9,         ///< open origin file err
    EZBS_OLDFILE_READ = 10,        ///< read origin file err
    EZBS_NO_MEMORY = 11,           ///< malloc err
} EZ_RV;

/**
 * @brief 
 * 
 */
typedef struct
{
    void *(*open)(const char *filename, int type, int *filesize);
    int (*read)(const void *fd, int offset, int length, void *buffer);
    int (*close)(void *fd);
} ez_file_cb_t;

/**
 * @brief 
 * 
 */
typedef struct
{
    ez_file_cb_t file_cb;
    int fd;
    int buf_size;
    char *buf;
    int data_len;
    int data_off;
    int new_size;
    int new_off;
    int old_size;
    int old_off;
    int old_base;
    void *cstrm;
    void *dstrm;
    void *estrm;
    off_t ctrl[3];
} ez_bspatch_ctx_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 
     * 
     * @param oldfilename old file path
     * @param patch diff patch 
     * @param patchsize diff patch size
     * @param new new packet 
     * @param newsize new packet size
     * @param file_cb file cb
     * @return int 0 success  !0 failed
     */
    BSPATCH_API EZ_RV ez_bspatch(const char *oldfilename, const char *patch, int patchsize,char *newdata, int *newsize, ez_file_cb_t *file_cb);

    /**
     * @brief 
     * 
     * @param ctx context
     * @param file_cb   file cb
     * @param oldfilename  old file path
     * @param oldoff        old file offset
     * @param oldsize       old file size
     * @param patch         patch data
     * @param patchsize     patch data size
     * @param bufsizemax    max buf
     * @return BSPATCH_API EZ_RV
     */
    BSPATCH_API EZ_RV ez_patch_init(ez_bspatch_ctx_t *ctx, ez_file_cb_t *file_cb, const char *oldfilename,
                                    int oldoff, int oldsize, const char *patch, int patchsize, int bufsizemax);

    /**
     * @brief 
     * 
     * @param ctx 
     * @param newdata 
     * @param newsize
     * @return 
     */
    BSPATCH_API EZ_RV ez_patch_update(ez_bspatch_ctx_t *ctx, char *newdata, int *newsize);

    /**
     * @brief 
     * 
     * @param ctx 
     * @return BSPATCH_API ez_patch_finit 
     */
    BSPATCH_API EZ_RV ez_patch_finit(ez_bspatch_ctx_t *ctx);

    /**
     * @brief 
     * 
     * @return int8_t* 
     */
    char *ez_patch_version(void);

#ifdef __cplusplus
}
#endif