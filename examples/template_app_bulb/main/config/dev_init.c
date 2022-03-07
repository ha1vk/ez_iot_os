#include "dev_init.h"

#include "bulb_led_drv_pwm.h"

#include "product_config.h"
#include <stdlib.h>
#include <string.h>
#include "config_implement.h"
#include "config_type.h"
#include "ezlog.h"

#ifdef HAL_ESP
#include "driver/gpio.h"
#endif
#define SOFT_MAJOR_VERSION	2
#define SOFT_MINOR_VERSION 	0
#define SOFT_REVISION		0


int set_light_gpio_config(void)
{
	led_gpio_init_t gpio_init = {0};
#ifdef HAL_ESP

    gpio_config_t io_conf;
#endif
    char io_num = get_product_io_config_num();

    memset(&gpio_init,0xff ,sizeof(gpio_init));
    if (io_num > IO_NUMS)
	{
		ezlog_w(TAG_APP, "set_gpio_config io_num[%d] error!\n", io_num);
		return -1;
	}
	
	IO_config_t *io_config = get_product_io_config();
	if (NULL == io_config)
	{
		ezlog_w(TAG_APP, "set_gpio_config get_product_io_config error!\n");
		return -1;
	}
	int i = 0;

    gpio_init.white_control_mode = WHITE_CONTROL_CW;

	for (i = 0; i < io_num; i++)
	{

		
		if(CONFIG_CCT == io_config[i].light )
		{
            gpio_init.white_control_mode = WHITE_CONTROL_CCT;
		}
		
		gpio_init.led_gpio_config[i].led_gpio_name = io_config[i].name;
	
		if(LED_GPIO_DRIVER_MODE_I2C == io_config[i].drive_mode)
        {
            gpio_init.white_control_mode = LED_CONTROL_I2C; //有一个引脚为i2c 方式时，整灯驱动方式为i2c
            gpio_init.led_gpio_config[i].led_i2c_name = io_config[i].i2c;            
        }
        else
        {            
            gpio_init.driver_config.pwm_frequency = io_config[i].param.pwm_frequency;  //暂且只用一个引脚的pwm 频率,配置文件的所有引脚频率都一样
            gpio_init.led_gpio_config[i].led_color_name = io_config[i].light;
            gpio_init.led_gpio_config[i].led_gpio_level = io_config[i].enable;
            gpio_init.led_gpio_config[i].led_gpio_mode = ((0 == io_config[i].mode) ? LED_GPIO_MODE_INPUT : LED_GPIO_MODE_OUTPUT);

        }
        
	}
	
	for (i = io_num; i < IO_NUMS; i++)
	{
		gpio_init.led_gpio_config[i].led_color_name = 0xff;
        gpio_init.led_gpio_config[i].led_gpio_name = 0xff;
    
        //gpio_init.led_gpio_config[i].led_gpio_level = io_config[i].enable;
        //gpio_init.led_gpio_config[i].led_gpio_mode = ((0 == io_config[i].mode) ? LED_GPIO_MODE_INPUT : LED_GPIO_MODE_OUTPUT);
	}
	
	
    #ifdef HAL_ESP  //乐鑫需要将15 io 拉低，减少电流
    memset(&io_conf, 0, sizeof(gpio_config_t));
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<15); 
    
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    #endif

    if((WHITE_CONTROL_CW == gpio_init.white_control_mode)
        ||(WHITE_CONTROL_CCT == gpio_init.white_control_mode)
        )
    {
        if (0 != bulb_leds_pwm_init(&gpio_init))
        {
            ezlog_w(TAG_APP, "Set gpio config failed.\n");
            return -1;
        }
    }
    else if(LED_CONTROL_I2C == gpio_init.white_control_mode)
    {
        if (0 != bulb_leds_i2c_bp5758_config(&gpio_init))
        {
            ezlog_w(TAG_APP, "Set gpio config failed.\n");
            return -1;
        }
    }
    else
    {
        ezlog_w(TAG_APP, "config gpio error .\n");
        return -1;
    }

	
    return 0;
}



int set_gpio_config(void)
{
	int ret = 0;
	int product_type = get_product_type();

	//根据产品类型设置不同的配置
	switch (product_type)
	{
		case LIGHT:
			set_light_gpio_config();
			break;
		case MCU:		
			break;
		default :
			break;
	}

	return ret;
}

int mk_soft_version(char *pversion_buf)
{
	int year = 0;
	int month = 0;
	int day = 0;
	char month_name[4]; // 编译日期的月份
	char *all_mon_names[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	memset(month_name, 0, sizeof(month_name));

	sscanf(__DATE__, "%s%d%d", month_name, &day, &year);

	for (month = 0; month < 12; month++)
	{
		if (strcmp(month_name, all_mon_names[month]) == 0)
		{
			break;
		}
	}

	month++;

    year -= 2000;
	

	if (NULL != pversion_buf)
	{
		sprintf(pversion_buf, "V%d.%d.%d build %02d%02d%02d", SOFT_MAJOR_VERSION, SOFT_MINOR_VERSION, SOFT_REVISION, year, month, day);
	}

	return 0;
}
