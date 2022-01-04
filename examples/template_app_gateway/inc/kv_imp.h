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
 * XuRongjun (xurongjun@ezvizlife.com) - Implementation of ezos kv interface, adapted to kvdb library by file mode.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-16     xurongjun    first version 
 *******************************************************************************/

#include <stddef.h>


struct fdb_kvdb ez_kvdb;

int kv_init(const void *default_kv);

int kv_raw_set(const char *key, const void *value, size_t length);

int kv_raw_get(const char *key, void *value, size_t *length);

int kv_del(const char *key);

int kv_del_by_prefix(const char *key_prefix);

void kv_print(void);

int kv_deinit(void);