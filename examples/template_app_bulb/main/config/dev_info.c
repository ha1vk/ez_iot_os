#include "dev_info.h"
#include "cJSON.h"
#include "ezlog.h"
#include "product_config.h"

#include <string.h>

#define TAG "T_DEVINFO"

static product_dev_info_t g_product_dev_info = {0};

char *get_dev_productKey()
{
    return g_product_dev_info.dev_productKey;
}

char *get_dev_deviceName()
{
    return g_product_dev_info.dev_deviceName;
}

char *get_dev_License()
{
    return g_product_dev_info.dev_deviceLicense;
}

char get_dev_auth_mode()
{
    return g_product_dev_info.dev_auth_mode;
}

static int parse_sap_config(char *buf, int buf_size)
{
    BOOT_PARAMS boot_param = {0};
    memcpy(&boot_param, buf, sizeof(BOOT_PARAMS));
    if (boot_param.magicNumber != MAGIC_NUMBER && boot_param.magicNumber != MAGIC_NUMBER_INVERT)
    {
        ezlog_e(TAG, "not a sap config. magic number: %x", boot_param.magicNumber);
        return -1;
    }

    memset(&g_product_dev_info, 0, sizeof(product_dev_info_t));
    memcpy(g_product_dev_info.dev_productKey, get_product_PTID(), sizeof(g_product_dev_info.dev_productKey));
    memcpy(g_product_dev_info.dev_deviceName, boot_param.prodNo, sizeof(boot_param.prodNo));
    memcpy(g_product_dev_info.dev_deviceLicense, boot_param.rand_code, sizeof(boot_param.rand_code));
    g_product_dev_info.dev_auth_mode = 0;
    return 0;
}

int parse_lic_config(char *buf, int buf_size)
{
    int rv = -1;
    cJSON *cjson_lic = NULL;
    cJSON *found = NULL;

    do
    {
        for (size_t i = 0; i < buf_size; i++)
        {
            if (buf[i] == 0xff)
            {
                buf[i] = 0x0;
                break;
            }
        }

        if (NULL == (cjson_lic = cJSON_Parse(buf)))
        {
            ezlog_e(TAG_APP, "cjson_lic parse!");
            break;
        }

        if (NULL == (found = cJSON_GetObjectItem(cjson_lic, "dev_productKey")) || cJSON_String != found->type)
        {
            break;
        }

        strncpy(g_product_dev_info.dev_productKey, found->valuestring, sizeof(g_product_dev_info.dev_productKey) - 1);

        if (NULL == (found = cJSON_GetObjectItem(cjson_lic, "dev_deviceName")) || cJSON_String != found->type)
        {
            break;
        }

        strncpy(g_product_dev_info.dev_deviceName, found->valuestring, sizeof(g_product_dev_info.dev_deviceName) - 1);

        if (NULL == (found = cJSON_GetObjectItem(cjson_lic, "dev_deviceLicense")) || cJSON_String != found->type)
        {
            break;
        }

        strncpy(g_product_dev_info.dev_deviceLicense, found->valuestring, sizeof(g_product_dev_info.dev_deviceLicense) - 1);
        g_product_dev_info.dev_auth_mode = 1;

        rv = 0;
    } while (0);

    if (cjson_lic)
    {
        cJSON_Delete(cjson_lic);
    }

    return rv;
}

int parse_dev_config(char *buf, int buf_size)
{
    int ret = 0;
    do
    {
        ret = parse_sap_config(buf, buf_size);
        if (0 == ret)
        {
            ezlog_i(TAG, "it is sap config.");
            break;
        }

        ret = parse_lic_config(buf, buf_size);
        if (0 == ret)
        {
            ezlog_i(TAG, "it is lic config.");
            break;
        }
    } while (0);

    return ret;
}
