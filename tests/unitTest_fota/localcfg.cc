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
#include "localcfg.h"
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

#define REGKEY(key, type) \
    {                     \
        key, #key, type   \
    }

static bool shell_exec(const char *cmd)
{
    bool bRet = false;
    int status = system(cmd);

    do
    {
        if (-1 == status)
            break;

        if (0 == WIFEXITED(status))
            break;

        if (0 != WEXITSTATUS(status))
            break;

        bRet = true;
    } while (false);

    return bRet;
}

bool makeCache()
{
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "cp -rf ./%s ./%s", dir_res_name, dir_cache_name);
    return shell_exec(cmd);
}

bool clearCache()
{
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "rm -rf ./%s", dir_cache_name);
    return shell_exec(cmd);
}

bool deleteFile(const char *filename)
{
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "rm -f ./%s", filename);
    return shell_exec(cmd);
}

bool isFileExist(const char *filename)
{
    if (access(filename, F_OK) == -1)
    {
        return false;
    }

    return true;
}

bool setFileValue(const char *filename, const char *buf, size_t size)
{
    FILE *config_file = fopen(filename, "w");
    bool bRet = false;

    do
    {
        if (NULL == config_file)
            break;

        if (size != fwrite(buf, 1, size, config_file))
        {
            fclose(config_file);
            break;
        }

        fflush(config_file);
        fclose(config_file);
        bRet = true;
    } while (0);

    return bRet;
}

bool getFileValue(const char *filename, char *buf, size_t *size)
{
    bool bRet = false;
    char buffer[2048] = {0};
    size_t real_read = 0;
    FILE *config_file = fopen(filename, "r");

    do
    {
        if (NULL == config_file)
            break;

        if ((real_read = fread(buffer, 1, *size, config_file)) <= 0 || real_read > *size)
        {
            fclose(config_file);
            break;
        }

        fclose(config_file);
        memcpy(buf, buffer, real_read);
        *size = real_read;
        bRet = true;
    } while (0);

    return bRet;
}

bool CCfgBase::load()
{
    bool bRet = false;
    char buf[2048] = {0};
    size_t buf_len = sizeof(buf);

    do
    {
        if (0 == m_file_path.length())
            break;

        if (!getFileValue(m_file_path.c_str(), buf, &buf_len))
            break;

        if (m_json_ptr)
        {
            cJSON_Delete((cJSON *)m_json_ptr);
            m_json_ptr = NULL;
        }

        if (NULL == (m_json_ptr = cJSON_Parse(buf)))
            break;

        bRet = true;
    } while (false);

    return bRet;
}

bool CCfgBase::save()
{
    bool bRet = false;
    char *p = NULL;

    do
    {
        if (0 == m_file_path.length())
            break;

        if (NULL == (p = cJSON_PrintUnformatted((cJSON *)m_json_ptr)))
            break;

        if (!setFileValue(m_file_path.c_str(), p, strlen(p)))
            break;

        bRet = true;
    } while (false);

    if (p)
    {
        free(p);
    }

    return bRet;
}

cfgKey_t CCfgLocal::keyMaps[] = {
    REGKEY(version, cJSON_String),
    REGKEY(old_file, cJSON_String),
    REGKEY(new_file, cJSON_String),
    REGKEY(patch_file, cJSON_String),
    REGKEY(buf_max, cJSON_Number),
};

CCfgLocal::CCfgLocal(const std::string &file_path)
{
    m_file_path = file_path;
    m_json_ptr = NULL;
}

CCfgLocal::~CCfgLocal()
{
    if (m_json_ptr)
    {
        cJSON_Delete((cJSON *)m_json_ptr);
        m_json_ptr = NULL;
    }
}

bool CCfgLocal::bFormatValid()
{
    if (NULL == m_json_ptr)
        return false;

    if (root_cfg_key_max != sizeof(keyMaps) / sizeof(cfgKey_t))
        return false;

    for (size_t i = 0; i < root_cfg_key_max; i++)
    {
        cJSON *found = cJSON_GetObjectItem((cJSON *)m_json_ptr, keyMaps[i].keyName);
        if (NULL == found || found->type != keyMaps[i].cjsonType)
        {
            return false;
        }
    }

    return true;
}

int CCfgLocal::getItemInt(int key)
{
    if (NULL == m_json_ptr)
        return -1;

    cJSON *found = cJSON_GetObjectItem((cJSON *)m_json_ptr, keyMaps[key].keyName);
    if (NULL == found || cJSON_Number != found->type)
    {
        return -1;
    }

    return found->valueint;
}

char *CCfgLocal::getItemString(int key)
{
    if (NULL == m_json_ptr)
        return NULL;

    cJSON *found = cJSON_GetObjectItem((cJSON *)m_json_ptr, keyMaps[key].keyName);
    if (NULL == found || cJSON_String != found->type)
    {
        return NULL;
    }

    return found->valuestring;
}

CCfgLocal &getCfgSingleton()
{
    static CCfgLocal g_cfgRoot(std::string("./") + dir_res_name + "/" + file_config_name);
    return g_cfgRoot;
}