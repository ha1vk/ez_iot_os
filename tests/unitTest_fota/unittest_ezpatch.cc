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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include "gtest.h"
#include "localcfg.h"
#include "ez_bspatch.h"
#include "utils.h"
#include "md5.h"

static void *open_cb(const char *filename, int type, int *filesize)
{
  int fd = open(filename, O_RDONLY, 0);
  if (-1 == fd)
  {
    return NULL;
  }

  *filesize = lseek(fd, 0, SEEK_END);
  if (-1 == *filesize)
  {
    return NULL;
  }

  return (void *)fd;
}

static int read_cb(const void *fd, int offset, int length, void *buffer)
{
  lseek((int)fd, offset, SEEK_SET);
  return read((int)fd, buffer, length);
}

static int close_cb(void *fd)
{
  return close((int)fd);
}

TEST(ezpatch, version)
{
  int8_t * pver = (int8_t*)ez_patch_version();
  printf("ver = %s\n", pver);
}

/**
 * @brief test simple merge method,which cause a great deal of memory
 * 
 */
TEST(ezpatch, simple)
{
  ez_file_cb_t file_cb = {open_cb, read_cb, close_cb};
  int fd = 0;
  char *patch = NULL;
  char *new_patch_buf = NULL;
  ssize_t patchsize, newsize = 0;
  int ret = 0;
  char old_path[256] = {0};
  char new_path[256] = {0};
  char patch_path[256] = {0};
  char patch2new_path[256] = {0};

  EXPECT_TRUE(clearCache());
  EXPECT_TRUE(makeCache());

  ASSERT_TRUE(getCfgSingleton().getItemString(CCfgLocal::old_file));
  ASSERT_TRUE(getCfgSingleton().getItemString(CCfgLocal::new_file));
  ASSERT_TRUE(getCfgSingleton().getItemString(CCfgLocal::patch_file));

  snprintf(old_path, sizeof(old_path), "%s/%s", dir_cache_name, getCfgSingleton().getItemString(CCfgLocal::old_file));
  snprintf(new_path, sizeof(new_path), "%s/%s", dir_cache_name, getCfgSingleton().getItemString(CCfgLocal::new_file));
  snprintf(patch_path, sizeof(patch_path), "%s/%s", dir_cache_name, getCfgSingleton().getItemString(CCfgLocal::patch_file));
  snprintf(patch2new_path, sizeof(patch2new_path), "%s/%s_patch", dir_cache_name, getCfgSingleton().getItemString(CCfgLocal::new_file));
  ASSERT_TRUE((fd = open(patch_path, O_RDONLY, 0)) > 0);
  ASSERT_TRUE((patchsize = lseek(fd, 0, SEEK_END)) != -1);
  ASSERT_TRUE((patch = (char *)malloc(patchsize + 1)) != NULL);
  ASSERT_TRUE(lseek(fd, 0, SEEK_SET) == 0);
  ASSERT_TRUE(read(fd, patch, patchsize) == patchsize);
  ASSERT_TRUE(close(fd) != -1);
  fd = 0;
  ret = ez_bspatch(old_path, patch, patchsize, NULL, &newsize, &file_cb);
  ASSERT_TRUE((0 == ret));

  new_patch_buf = (char *)malloc(newsize + 1);
  ASSERT_TRUE(new_patch_buf);

  ret = ez_bspatch(old_path, patch, patchsize, new_patch_buf, &newsize, &file_cb);
  ASSERT_TRUE((0 == ret));

  fd = open(patch2new_path, O_CREAT | O_TRUNC | O_WRONLY, 0666);
  ASSERT_TRUE((fd > 0));
  ASSERT_TRUE((write(fd, new_patch_buf, newsize) == newsize));
  ASSERT_TRUE(close(fd) != -1);
  fd = 0;

  if (patch)
    free(patch);

  if (new_patch_buf)
    free(new_patch_buf);

  /*check degist*/
  unsigned char new_file_md5[16] = {0};
  unsigned char new_file_patch_md5[16] = {0};
  ASSERT_TRUE(0 == ez_calc_file_md5(new_path, new_file_md5));
  ASSERT_TRUE(0 == ez_calc_file_md5(patch2new_path, new_file_patch_md5));
  ASSERT_TRUE(0 == memcmp(new_file_md5, new_file_patch_md5, sizeof(new_file_md5)));
}

/**
 * @brief slice merging method in small memory footprint
 * 
 */
TEST(ezpatch, fragment)
{
  ez_file_cb_t file_cb = {open_cb, read_cb, close_cb};
  ez_bspatch_ctx_t ctx = {0};
  int fd = 0, fd_new = 0;
  char *patch = NULL;
  char *new_patch_buf = NULL;
  ssize_t patchsize, newsize = 0;
  int ret = 0;
  char old_path[256] = {0};
  char new_path[256] = {0};
  char patch_path[256] = {0};
  char patch2new_path[256] = {0};

  EXPECT_TRUE(clearCache());
  EXPECT_TRUE(makeCache());

  ASSERT_TRUE(getCfgSingleton().getItemString(CCfgLocal::old_file));
  ASSERT_TRUE(getCfgSingleton().getItemString(CCfgLocal::new_file));
  ASSERT_TRUE(getCfgSingleton().getItemString(CCfgLocal::patch_file));

  snprintf(old_path, sizeof(old_path), "%s/%s", dir_cache_name, getCfgSingleton().getItemString(CCfgLocal::old_file));
  snprintf(new_path, sizeof(new_path), "%s/%s", dir_cache_name, getCfgSingleton().getItemString(CCfgLocal::new_file));
  snprintf(patch_path, sizeof(patch_path), "%s/%s", dir_cache_name, getCfgSingleton().getItemString(CCfgLocal::patch_file));
  snprintf(patch2new_path, sizeof(patch2new_path), "%s/%s_patch", dir_cache_name, getCfgSingleton().getItemString(CCfgLocal::new_file));

  /* Open and read patch file */
  ASSERT_TRUE((fd = open(patch_path, O_RDONLY, 0)) > 0);
  ASSERT_TRUE((patchsize = lseek(fd, 0, SEEK_END)) != -1);
  ASSERT_TRUE((patch = (char *)malloc(patchsize + 1)) != NULL);
  ASSERT_TRUE(lseek(fd, 0, SEEK_SET) == 0);
  ASSERT_TRUE(read(fd, patch, patchsize) == patchsize);
  ASSERT_TRUE(close(fd) != -1);
  fd_new = open(patch2new_path, O_CREAT | O_TRUNC | O_WRONLY, 0666);
  ASSERT_TRUE((fd_new > 0));
  ASSERT_TRUE(lseek(fd_new, 0, SEEK_SET) == 0);
  ASSERT_TRUE((new_patch_buf = (char *)malloc(getCfgSingleton().getItemInt(CCfgLocal::buf_max))) != NULL);
  fd = 0;

  /*do patch*/
  ret = ez_patch_init(&ctx, &file_cb, old_path, 0, 0, patch, patchsize, getCfgSingleton().getItemInt(CCfgLocal::buf_max));
  ASSERT_TRUE((0 == ret));

  do
  {
    int newsize = getCfgSingleton().getItemInt(CCfgLocal::buf_max);
    memset(new_patch_buf, 0, newsize);

    ret = ez_patch_update(&ctx, new_patch_buf, &newsize);
    ASSERT_TRUE((ret <= 0));
    ASSERT_TRUE((write(fd_new, new_patch_buf, newsize) == newsize));
  } while (EZBS_NEW_DATA_BUF_FULL == ret);

  ret = ez_patch_finit(&ctx);
  ASSERT_TRUE((0 == ret));
  ASSERT_TRUE(close(fd_new) != -1);

  if (patch)
    free(patch);

  if (new_patch_buf)
    free(new_patch_buf);

  /*check degist*/
  unsigned char new_file_md5[16] = {0};
  unsigned char new_file_patch_md5[16] = {0};
  ASSERT_TRUE(0 == ez_calc_file_md5(new_path, new_file_md5));
  ASSERT_TRUE(0 == ez_calc_file_md5(patch2new_path, new_file_patch_md5));
  ASSERT_TRUE(0 == memcmp(new_file_md5, new_file_patch_md5, sizeof(new_file_md5)));
}

/**
 * @brief Single partition upgrade mode
 * 
 */
TEST(ezpatch, single_partition)
{
  ez_file_cb_t file_cb = {open_cb, read_cb, close_cb};
  ez_bspatch_ctx_t ctx = {0};
  int fd = 0, fd_new = 0;
  char *patch_1 = NULL;
  char *patch_2 = NULL;
  char *new_patch_buf = NULL;
  ssize_t patchsize_1, patchsize_2, newsize = 0;
  int ret = 0;
  char old_path[256] = {0};
  char new_path[256] = {0};
  char old_path_1[256] = {0};
  char old_path_2[256] = {0};

  char patch_path_1[256] = {0};
  char patch_path_2[256] = {0};
  int old_file_off = 0;
  int old_file_size = 0;
  char patch2new_path[256] = {0};
  ssize_t old1size = 0;
  ssize_t old2size = 0;

  EXPECT_TRUE(clearCache());
  EXPECT_TRUE(makeCache());

  ASSERT_TRUE(getCfgSingleton().getItemString(CCfgLocal::old_file));
  ASSERT_TRUE(getCfgSingleton().getItemString(CCfgLocal::new_file));
  ASSERT_TRUE(getCfgSingleton().getItemString(CCfgLocal::patch_file));

  snprintf(old_path, sizeof(old_path), "%s/%s/%s", dir_cache_name, "single_partition", getCfgSingleton().getItemString(CCfgLocal::old_file));
  snprintf(new_path, sizeof(new_path), "%s/%s/%s", dir_cache_name, "single_partition", getCfgSingleton().getItemString(CCfgLocal::new_file));
  snprintf(patch_path_1, sizeof(patch_path_1), "%s/%s/%s", dir_cache_name, "single_partition", "app_patch_1.img");
  snprintf(patch_path_2, sizeof(patch_path_2), "%s/%s/%s", dir_cache_name, "single_partition", "app_patch_2.img");
  snprintf(old_path_1, sizeof(patch_path_2), "%s/%s/%s", dir_cache_name, "single_partition", "app_old_1.img");
  snprintf(old_path_2, sizeof(patch_path_2), "%s/%s/%s", dir_cache_name, "single_partition", "app_old_2.img");

  ASSERT_TRUE((fd = open(old_path_1, O_RDONLY, 0)) > 0);
  ASSERT_TRUE((old1size = lseek(fd, 0, SEEK_END)) != -1);
  ASSERT_TRUE(close(fd) != -1);

  ASSERT_TRUE((fd = open(old_path_2, O_RDONLY, 0)) > 0);
  ASSERT_TRUE((old2size = lseek(fd, 0, SEEK_END)) != -1);
  ASSERT_TRUE(close(fd) != -1);

  /* Open and read patch file 1 */
  ASSERT_TRUE((fd = open(patch_path_1, O_RDONLY, 0)) > 0);
  ASSERT_TRUE((patchsize_1 = lseek(fd, 0, SEEK_END)) != -1);
  ASSERT_TRUE((patch_1 = (char *)malloc(patchsize_1 + 1)) != NULL);
  ASSERT_TRUE(lseek(fd, 0, SEEK_SET) == 0);
  ASSERT_TRUE(read(fd, patch_1, patchsize_1) == patchsize_1);
  ASSERT_TRUE(close(fd) != -1);
  fd = 0;

 /* Open and read patch file 2 */
  ASSERT_TRUE((fd = open(patch_path_2, O_RDONLY, 0)) > 0);
  ASSERT_TRUE((patchsize_2 = lseek(fd, 0, SEEK_END)) != -1);
  ASSERT_TRUE((patch_2 = (char *)malloc(patchsize_2 + 1)) != NULL);
  ASSERT_TRUE(lseek(fd, 0, SEEK_SET) == 0);
  ASSERT_TRUE(read(fd, patch_2, patchsize_2) == patchsize_2);
  ASSERT_TRUE(close(fd) != -1);

  ASSERT_TRUE((new_patch_buf = (char *)malloc(getCfgSingleton().getItemInt(CCfgLocal::buf_max))) != NULL);

  /*write to old file*/
  ASSERT_TRUE((fd_new = open(old_path, O_RDWR, 0666)) > 0);
  ASSERT_TRUE(lseek(fd_new, 0, SEEK_SET) == 0);

  /*do patch init*/
  ret = ez_patch_init(&ctx, &file_cb, old_path, old_file_off, old1size, patch_1, patchsize_1, 0);
  ASSERT_TRUE((0 == ret));

  do
  {
    int newsize = getCfgSingleton().getItemInt(CCfgLocal::buf_max);
    memset(new_patch_buf, 0, newsize);

    ret = ez_patch_update(&ctx, new_patch_buf, &newsize);
    ASSERT_TRUE((ret <= 0));
    ASSERT_TRUE((write(fd_new, new_patch_buf, newsize) == newsize));
    old_file_off += newsize;
  } while (EZBS_NEW_DATA_BUF_FULL == ret);

  ret = ez_patch_finit(&ctx);
  ASSERT_TRUE((0 == ret));

  // ASSERT_TRUE(lseek(fd_new, old_file_off, SEEK_SET) == old_file_off);

  /*do patch init*/
  ret = ez_patch_init(&ctx, &file_cb, old_path, old_file_off, old2size, patch_2, patchsize_2, 0);
  ASSERT_TRUE((0 == ret));

  do
  {
    int newsize = getCfgSingleton().getItemInt(CCfgLocal::buf_max);
    memset(new_patch_buf, 0, newsize);

    ret = ez_patch_update(&ctx, new_patch_buf, &newsize);
    ASSERT_TRUE((ret <= 0));
    ASSERT_TRUE((write(fd_new, new_patch_buf, newsize) == newsize));
  } while (EZBS_NEW_DATA_BUF_FULL == ret);

  ret = ez_patch_finit(&ctx);
  ASSERT_TRUE((0 == ret));

  ASSERT_TRUE(close(fd_new) != -1);

  if (patch_1)
    free(patch_1);

  if (patch_2)
    free(patch_2);

  if (new_patch_buf)
    free(new_patch_buf);

  /*check degist*/
  unsigned char new_file_md5[16] = {0};
  unsigned char new_file_patch_md5[16] = {0};
  ASSERT_TRUE(0 == ez_calc_file_md5(new_path, new_file_md5));
  ASSERT_TRUE(0 == ez_calc_file_md5(old_path, new_file_patch_md5));
  ASSERT_TRUE(0 == memcmp(new_file_md5, new_file_patch_md5, sizeof(new_file_md5)));

  bscomptls_md5_starts_2();
}

TEST(ezpatch, single_partition_precheck)
{
}