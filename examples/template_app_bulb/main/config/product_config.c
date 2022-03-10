#include "product_config.h"
#include "dev_info.h"
#ifdef HAL_ESP
#include "esp_partition.h"
#endif
#include <stdio.h>
#include <string.h>
#include "cJSON.h"
#include "ezlog.h"
#include <stdlib.h>

#define PRODUCT_CONFIG_MAX_SIZE (4 * 1024) //4KB
#define PRODUCT_CONFIG_OFFSET (4 * 1024)

static product_config_t g_product_config = {0};

void print(void)
{
    ezlog_v(TAG_APP, "test ap_prefix[%s]", g_product_config.device.ap_prefix);
    ezlog_v(TAG_APP, "test category[%s]", g_product_config.device.category);
    ezlog_v(TAG_APP, "test user_defined_type[%s]", g_product_config.device.user_defined_type);
    ezlog_v(TAG_APP, "test dev_name[%s]", g_product_config.device.dev_name);
    ezlog_v(TAG_APP, "test module_type[%s]", g_product_config.device.module_type);
    ezlog_v(TAG_APP, "test ap_timval[%d]", g_product_config.ap_timval);

    ezlog_v(TAG_APP, "test country_code[%s]", g_product_config.country_code);

    ezlog_v(TAG_APP, "test PDIT[%s]", g_product_config.PTID);
    ezlog_v(TAG_APP, "test product_type[%d]", g_product_config.product_type);
    ezlog_v(TAG_APP, "test product_subtype[%d]", g_product_config.product_subtype);

    ezlog_v(TAG_APP, "test baud_rate[%d]", g_product_config.baud_config.baud_rate);
    ezlog_v(TAG_APP, "test data_bits[%d]", g_product_config.baud_config.data_bits);
    ezlog_v(TAG_APP, "test stop_bits[%d]", g_product_config.baud_config.stop_bits);
    ezlog_v(TAG_APP, "test parity[%d]", g_product_config.baud_config.parity);
    ezlog_v(TAG_APP, "test flowcontrol[%d]", g_product_config.baud_config.flowcontrol);

    for (int i = 0; i < g_product_config.param.io_config_num; i++)
    {
        ezlog_v(TAG_APP, "===================================================\n");
        ezlog_v(TAG_APP, "test name[%d]", g_product_config.param.io_config[i].name);
        ezlog_v(TAG_APP, "test enable[%d]", g_product_config.param.io_config[i].enable);
        ezlog_v(TAG_APP, "test mode[%d]", g_product_config.param.io_config[i].mode);
        ezlog_v(TAG_APP, "test drive_mode[%d]", g_product_config.param.io_config[i].drive_mode);
        ezlog_v(TAG_APP, "test intr_type[%d]", g_product_config.param.io_config[i].param.intr_type);

        ezlog_v(TAG_APP, "test name[%d]", g_product_config.param.io_config[i].light);

    }
	#ifdef MCU_VERSION
    for (int i = 0; i < g_product_config.param.io_func_num; i++)
    {
        ezlog_v(TAG_APP, "===================================================\n");
        ezlog_v(TAG_APP, "test func[%d]", g_product_config.param.io_func[i].func);
        ezlog_v(TAG_APP, "test lower[%d]", g_product_config.param.io_func[i].lower);
        ezlog_v(TAG_APP, "test upper[%d]", g_product_config.param.io_func[i].upper);
    }

    ezlog_v(TAG_APP, "test indicate_ilght_r[%d]", g_product_config.param.io_indicator_light_r);
    ezlog_v(TAG_APP, "test indicate_ilght_g[%d]", g_product_config.param.io_indicator_light_g);
	#endif

	ezlog_v(TAG_APP,"chenhao test brightness_upper[%d]\n", g_product_config.param.function.brightness_upper);
	ezlog_v(TAG_APP,"chenhao test brightness_lower[%d]\n", g_product_config.param.function.brightness_lower);
	ezlog_v(TAG_APP,"chenhao test color_mode[%d]\n", g_product_config.param.function.color_mode);
	ezlog_v(TAG_APP,"chenhao test power_memory[%d]\n", g_product_config.param.function.power_memory);
	ezlog_v(TAG_APP,"chenhao test power_on_light[%d]\n", g_product_config.param.function.power_on_light);
	ezlog_v(TAG_APP,"chenhao test memory_timval[%d]\n", g_product_config.param.function.memory_timval);
	ezlog_v(TAG_APP,"chenhao test reset_time_lower[%d]\n", g_product_config.param.function.reset_time_lower);
	ezlog_v(TAG_APP,"chenhao test reset_time_upper[%d]\n", g_product_config.param.function.reset_time_upper);
	ezlog_v(TAG_APP,"chenhao test default_cct[%d]\n", g_product_config.param.function.default_cct);

}

void parse_product_config_baud(cJSON *baud)
{
    cJSON *found = NULL;

    if (NULL == baud)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem baud error!");
    }

    if ((found = cJSON_GetObjectItem(baud, "baud_rate")))
    {
        g_product_config.baud_config.baud_rate = found->valuedouble;
    }
    else
    {
        g_product_config.baud_config.baud_rate = 115200;
    }

    if ((found = cJSON_GetObjectItem(baud, "data_bits")))
    {
        g_product_config.baud_config.data_bits = found->valuedouble;
    }
    else
    {
        g_product_config.baud_config.data_bits = 3;
    }

    if ((found = cJSON_GetObjectItem(baud, "stop_bits")))
    {
        g_product_config.baud_config.stop_bits = found->valuedouble;
    }
    else
    {
        g_product_config.baud_config.stop_bits = 1;
    }

    if ((found = cJSON_GetObjectItem(baud, "parity")))
    {
        g_product_config.baud_config.parity = found->valuedouble;
    }
    else
    {
        g_product_config.baud_config.parity = 0;
    }

    if ((found = cJSON_GetObjectItem(baud, "flowcontrol")))
    {
        g_product_config.baud_config.flowcontrol = found->valuedouble;
    }
    else
    {
        g_product_config.baud_config.flowcontrol = 0;
    }
}

void parse_product_config_device(cJSON *device)
{
    cJSON *found = NULL;

    if (NULL == device)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem device error!");
    }

    device_t *device_cfg = &g_product_config.device;
    memset(device_cfg, 0, sizeof(device_t));

    if ((found = cJSON_GetObjectItem(device, "ap_prefix")))
    {
        strncpy(device_cfg->ap_prefix, found->valuestring, sizeof(device_cfg->ap_prefix) - 1);
    }

    if ((found = cJSON_GetObjectItem(device, "category")))
    {
        strncpy(device_cfg->category, found->valuestring, sizeof(device_cfg->category));
    }

    if ((found = cJSON_GetObjectItem(device, "user_defined_type")))
    {
        strncpy(device_cfg->user_defined_type, found->valuestring, sizeof(device_cfg->user_defined_type));
    }

    if ((found = cJSON_GetObjectItem(device, "dev_name")))
    {
        strncpy(device_cfg->dev_name, found->valuestring, sizeof(device_cfg->dev_name));
    }

    if ((found = cJSON_GetObjectItem(device, "module_type")))
    {
        strncpy(device_cfg->module_type, found->valuestring, sizeof(device_cfg->module_type));
    }
}


int parse_product_config(cJSON *root)
{
    cJSON *device = NULL;
    cJSON *found = NULL;
#ifdef MCU_VERSION
    cJSON *baud = NULL;
    baud = cJSON_GetObjectItem(root, "baud_config");
    parse_product_config_baud(baud)
#endif

    device = cJSON_GetObjectItem(root, "device");
    parse_product_config_device(device);

    cJSON *PTID = cJSON_GetObjectItem(root, "PTID");
    if (NULL == PTID)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem PTID error!");
        return -1;
    }
    memcpy(g_product_config.PTID, PTID->valuestring, sizeof(g_product_config.PTID));

    cJSON *product_type = cJSON_GetObjectItem(root, "product_type");
    if (NULL == product_type)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem product_type error!");
        return -1;
    }
    g_product_config.product_type = product_type->valuedouble;

    cJSON *product_subtype = cJSON_GetObjectItem(root, "product_subtype");
    if (NULL == product_subtype)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem product_subtype error!");
        return -1;
    }
    g_product_config.product_subtype = product_subtype->valuedouble;

    cJSON *ap_timval = cJSON_GetObjectItem(root, "ap_timval");
    if (NULL == ap_timval)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem ap_timval error,set it to default");
        g_product_config.ap_timval = 12;
    }
    else
    {
        g_product_config.ap_timval = ap_timval->valuedouble;
    }
    

    cJSON *country_code = cJSON_GetObjectItem(root, "country_code");
    if (NULL == country_code)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem country_code error!");
        strncpy(g_product_config.country_code, "CN", sizeof(g_product_config.country_code));
    }
    else
    {
        strncpy(g_product_config.country_code, country_code->valuestring, sizeof(g_product_config.country_code));
    }
    cJSON *product_param = cJSON_GetObjectItem(root, "product_param");
    if (NULL == product_param)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem product_param error!");
        return -1;
    }

    cJSON *IO_config = cJSON_GetObjectItem(product_param, "IO_config");
    if (NULL == IO_config)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem IO_config error!");
        return -1;
    }

    cJSON* reset_switch_times = cJSON_GetObjectItem(product_param, "reset_switch_times");
    if (NULL == reset_switch_times)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem reset_switch_times error!,set to 3\n");
        g_product_config.param.reset_switch_times = 3;

        return -1;
    }
    else
    {
        g_product_config.param.reset_switch_times = reset_switch_times->valuedouble;
    }

    g_product_config.param.reset_switch_times = reset_switch_times->valuedouble;
    
    int IO_config_Num = cJSON_GetArraySize(IO_config);
    int i = 0;
    cJSON *signal_IO_config = NULL;
    g_product_config.param.io_config_num = IO_config_Num;
    memset(g_product_config.param.io_config,0xff,sizeof(g_product_config.param.io_config));
    ezlog_v(TAG_APP, "init ioconfig ,length=%d",sizeof(g_product_config.param.io_config));
    for (i = 0; i < IO_config_Num; i++)
    {
        if (i >= IO_NUMS)
        {
            break;
        }
       
        
        signal_IO_config = cJSON_GetArrayItem(IO_config, i);
        if ((found = cJSON_GetObjectItem(signal_IO_config, "name")))
        {
            g_product_config.param.io_config[i].name = found->valuedouble;       
        }

        if ((found = cJSON_GetObjectItem(signal_IO_config, "enable")))
        {
            g_product_config.param.io_config[i].enable = found->valuedouble;
        }

        if ((found = cJSON_GetObjectItem(signal_IO_config, "light")))
        {
            g_product_config.param.io_config[i].light = found->valuedouble;
        }

        if ((found = cJSON_GetObjectItem(signal_IO_config, "i2c")))
        {
            g_product_config.param.io_config[i].i2c = found->valuedouble;
        }

		
        if ((found = cJSON_GetObjectItem(signal_IO_config, "mode")))
        {
            g_product_config.param.io_config[i].mode = found->valuedouble;
        }

        if ((found = cJSON_GetObjectItem(signal_IO_config, "drive_mode")))
        {
            g_product_config.param.io_config[i].drive_mode = found->valuedouble;
        }

        cJSON *drive_mode_param = NULL;
        if ((drive_mode_param = cJSON_GetObjectItem(signal_IO_config, "drive_mode_param")))
        {
            if ((found = cJSON_GetObjectItem(drive_mode_param, "intr_type")))
            {
                g_product_config.param.io_config[i].param.intr_type = found->valuedouble;
            }
            
            if ((found = cJSON_GetObjectItem(drive_mode_param, "pwm_frequency")))
            {
                g_product_config.param.io_config[i].param.pwm_frequency = found->valuedouble;
            }
            
        }
    }
#ifdef MCU_VERSION

    cJSON *func_config = cJSON_GetObjectItem(product_param, "func_config");
    if (NULL == func_config)
    {
        ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem func_config error!");
        return -1;
    }

    int IO_func_num = cJSON_GetArraySize(func_config);
    cJSON *IO_func_config = NULL;
    g_product_config.param.io_func_num = IO_func_num;   //对应配置文件中的func_config
    for (i = 0; i < IO_func_num; i++)
    {
        if (i >= FUNC_NUMS)
        {
            break;
        }
        IO_func_config = cJSON_GetArrayItem(func_config, i);
        if ((found = cJSON_GetObjectItem(IO_func_config, "func")))
        {
            g_product_config.param.io_func[i].func = found->valuedouble;
        }

        if ((found = cJSON_GetObjectItem(IO_func_config, "lower")))
        {
            g_product_config.param.io_func[i].lower = found->valuedouble;
        }
        else
        {
            g_product_config.param.io_func[i].lower = 1;
        }

        if ((found = cJSON_GetObjectItem(IO_func_config, "upper")))
        {
            g_product_config.param.io_func[i].upper = found->valuedouble;
        }
        else
        {
            g_product_config.param.io_func[i].upper = 3;
        }

        if ((found = cJSON_GetObjectItem(IO_func_config, "name")))
        {
            g_product_config.param.io_func[i].name = found->valuedouble;
        }
        else
        {
            ezlog_i(TAG_APP, "parse_product_config cJSON_GetObjectItem io func wrong error!");
            g_product_config.param.io_func[i].name = 0xff;
        }
    }

    cJSON *indicator_light = cJSON_GetObjectItem(product_param, "indicator_light_r");
    if (NULL == indicator_light)
    {
        ezlog_i(TAG_APP, "parse_product_config cJSON_GetObjectItem indicator_light error!");
        g_product_config.param.io_indicator_light_r = 0xff;
    }
    else
    {
        g_product_config.param.io_indicator_light_r = indicator_light->valuedouble;
    }

    indicator_light = cJSON_GetObjectItem(product_param, "indicator_light_g");
    if (NULL == indicator_light)
    {
        ezlog_i(TAG_APP, "parse_product_config cJSON_GetObjectItem indicator_light error!");
        g_product_config.param.io_indicator_light_g = 0xff;
    }
    else
    {
        g_product_config.param.io_indicator_light_g = indicator_light->valuedouble;
    }
	#endif
	cJSON* function = cJSON_GetObjectItem(product_param, "function");
		if (NULL == function)
		{
			ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem function error!\n");
			return -1;
		}
	
		if ((found = cJSON_GetObjectItem(function, "brightness_upper")))
		{
			g_product_config.param.function.brightness_upper = found->valuedouble;
		}
	
		if ((found = cJSON_GetObjectItem(function, "brightness_lower")))
		{
			g_product_config.param.function.brightness_lower = found->valuedouble;
		}
	
		if ((found = cJSON_GetObjectItem(function, "color_mode")))
		{
			g_product_config.param.function.color_mode = found->valuedouble;
		}
	
		if ((found = cJSON_GetObjectItem(function, "power_memory")))
		{
			g_product_config.param.function.power_memory = found->valuedouble;
		}
	
		if ((found = cJSON_GetObjectItem(function, "power_on_light")))
		{
			g_product_config.param.function.power_on_light = found->valuedouble;
		}
	
		if ((found = cJSON_GetObjectItem(function, "memory_timval")))
		{
			g_product_config.param.function.memory_timval = found->valuedouble;
		}

        if ((found = cJSON_GetObjectItem(function, "ap_timval")))
		{
			g_product_config.ap_timval = found->valuedouble;
		}
	
		if ((found = cJSON_GetObjectItem(function, "reset_time_lower")))
		{
			g_product_config.param.function.reset_time_lower = found->valuedouble;
		}
	
		if ((found = cJSON_GetObjectItem(function, "reset_time_upper")))
		{
			g_product_config.param.function.reset_time_upper = found->valuedouble;
		}
	
		if ((found = cJSON_GetObjectItem(function, "default_cct")))
		{
			g_product_config.param.function.default_cct = found->valuedouble;
		}
	
		cJSON* product_test_param = cJSON_GetObjectItem(product_param, "product_test_param");
		if (NULL == product_test_param)
		{
			ezlog_e(TAG_APP, "parse_product_config cJSON_GetObjectItem product_test_param error!\n");
			return -1;
		}
	
		if ((found = cJSON_GetObjectItem(product_test_param, "order")))
		{
			strncpy(g_product_config.param.product_test_param.order, found->valuestring, sizeof(g_product_config.param.product_test_param.order) - 1);
		}
	
		if ((found = cJSON_GetObjectItem(product_test_param, "step1time")))
		{
			g_product_config.param.product_test_param.step1time = found->valuedouble;
		}


    //print();

    return 0;
}

int product_config_init(void)
{
    
    int rv = -1;
    char *buf = NULL;
    cJSON *cjson_product = NULL;
    const esp_partition_t *partition = NULL;

    do
    {
        if (NULL == (buf = (char *)malloc(PRODUCT_CONFIG_MAX_SIZE)))
        {
            break;
        }
        memset(buf, 0, PRODUCT_CONFIG_MAX_SIZE);

        partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, "factory_data");

        if (NULL == partition)
        {
            ezlog_e(TAG_APP, "find fdata part.");
            break;
        }

        if (ESP_OK != esp_partition_read(partition, PRODUCT_CONFIG_OFFSET, buf, PRODUCT_CONFIG_MAX_SIZE))
        {
            ezlog_e(TAG_APP, "esp_partition_read error!");
            break;
        }

        ezlog_v(TAG_APP, "%s", buf);

        if (NULL == (cjson_product = cJSON_Parse(buf)))
        {
            ezlog_e(TAG_APP, "cjson_product parse!");
            break;
        }

        if (0 != parse_product_config(cjson_product))
        {
            ezlog_e(TAG_APP, "parse_product_config!");
            break;
        }

		memset(buf, 0, PRODUCT_CONFIG_MAX_SIZE);
        
        if (ESP_OK != esp_partition_read(partition, 0, buf, PRODUCT_CONFIG_MAX_SIZE))
        {
            ezlog_e(TAG_APP, "read fdata part 1");
            break;
        }

        ezlog_v(TAG_APP, "%s", buf);
        if (0 != parse_dev_config(buf, PRODUCT_CONFIG_MAX_SIZE))
        {
            ezlog_e(TAG_APP, "parse_lic_config!");
            break;
        }

        rv = 0;
    } while (0);

    if (cjson_product)
    {
        cJSON_Delete(cjson_product);
    }

    if (buf)
    {
        free(buf);
    }

    return rv;

}

product_config_t *get_product_config(void)
{
    return &g_product_config;
}

device_t *get_product_device_config(void)
{
    return &g_product_config.device;
}

IO_config_t *get_product_io_config(void)
{
    return g_product_config.param.io_config;
}
#ifdef MCU_VERSION

char get_product_io_indicator_light_r(void)
{
    return g_product_config.param.io_indicator_light_r;
}

char get_product_io_indicator_light_g(void)
{
    return g_product_config.param.io_indicator_light_g;
}
#endif

char *get_product_country_code(void)
{
    return g_product_config.country_code;
}

void set_product_country_code(char *CountryCode)
{
    memcpy(g_product_config.country_code,CountryCode, strlen(CountryCode));
}

baud_rate_t *get_product_baud_config(void)
{
    return &g_product_config.baud_config;
}

#ifdef MCU_VERSION
function_t *get_product_func_config(void)
{
    return g_product_config.param.io_func;   //按键功能
}

char get_product_func_num(void)
{
    return g_product_config.param.io_func_num;    //用户定义的按键功能点
}
#endif

char get_product_io_config_num(void)
{
    return g_product_config.param.io_config_num;
}

int get_product_type(void)
{
    return g_product_config.product_type;
}

int get_product_subtype(void)
{
    return g_product_config.product_subtype;
}

char *get_product_PTID(void)
{
    return g_product_config.PTID;
}

char *get_user_defined_type(void)
{
    return g_product_config.device.user_defined_type;
}

int get_ap_timval(void)
{
    return g_product_config.ap_timval;
}

#ifdef BULB_VERSION
char get_product_reset_switch_times(void)
{
    return g_product_config.param.reset_switch_times;
}

int get_default_cct(void)
{
    return g_product_config.param.function.default_cct;
}

int get_memory_timval(void)
{
    return g_product_config.param.function.memory_timval;
}

int get_reset_time_upper(void)
{
    return g_product_config.param.function.reset_time_upper;
}

int get_power_on_light(void)
{
    return g_product_config.param.function.power_on_light;
}

BULB_TEST_T* get_product_test_param(void)
{
    return &g_product_config.param.product_test_param;
}

BULB_FUNCTION_T* get_product_function(void)
{
    return &g_product_config.param.function;
}

#endif
