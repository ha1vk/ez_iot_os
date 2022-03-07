#ifndef _EZVIZ_PRODUCT_CONFIG_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _EZVIZ_PRODUCT_CONFIG_H_

#define IO_NUMS 5 // 模组对外可配的IO个数，目前暂定为5个
#define FUNC_NUMS 3// 模组可配置功能项，目前为3个，分别是重启、重置wifi、恢复出厂设置

typedef struct{
	char ap_prefix[23];           // AP前缀，最大22个字节，wifi的ssid = 前缀 + "_" + 序列号
	char category[16];			  // 设备类目，内部判断设备类型用的
	char user_defined_type[64];	  // 用户需要的展类型
	char dev_name[64];         	  // 设备名称
	char module_type[32];      	  // 模组型号
}device_t;

typedef union{
	int intr_type;				// IO中断触发方式，0：disable, 1:上升沿，2：下降沿，3：上升沿和下降沿 4：低电平 5：高电平
	int pwm_frequency; //灯控应用，PWM频率（默认1000HZ）
}drive_mode_param_t; 

typedef struct{
	char name;					// GPIO的名字，目前有IO 4、5、12、13、14
	char enable;				// 0:低电平 ；1:高电平
	char mode;					// 1: 输入 ；2：输出
	char drive_mode;			// IO驱动方式 0:PWM驱动 1：I2C 2:i/o 3:spi	
	char light;      // 0：冷光 ；1：暖光；2：红 ；3：绿；4：蓝；5：CCT；6：亮度,(5,6须同时存在）
    char i2c;        //LED_I2C_SDA=0;LED_I2C_CLK=1;
	char holder[2];
	drive_mode_param_t param;	// 驱动方式需要的一些特殊参数
}IO_config_t;

typedef struct{
	char func;  // 管脚功能 0:重启 1:重置wifi 2:重置
	char lower; // 单位s，按键时长下限
	char upper; // 单位s，按键时长上限
	char name;  // 对应的io口
}function_t;

typedef struct{
	char brightness_upper;//亮度最大值(%)上限，调节力度最小1%
	char brightness_lower;//亮度最大值(%)下限
	char color_mode; // 色温调节驱动方式 0:空；1：CW-冷光/暖光；2：CCT-亮度/色温 3:RGB+CCT（色温的最小调节当位）
	char power_memory; //0：空 1：断电记忆（定义成5s） 2：开关后恢复到默认状态
	char power_on_light;//产品上电点灯模式 0：空 1：开关过程无渐变 2：开关过程中有渐变（需要定义）
	char memory_timval;//记忆模式间隔时间，默认5s，单位s
	char reset_time_lower;//进入配网重置次数下限值
	char reset_time_upper;//进入配网重置次数上限值
    short default_cct;//默认色温值，默认为3000K，单位K
	char holder[1];
}BULB_FUNCTION_T;

typedef struct{
	char order[6];//亮灯的顺序，C：冷光 ；W	:暖光；B：蓝色；G：绿色；R：红色，用户可以随意配置
	char holder[2];  //占位符，保持字节对齐
	int step1time;//第一个阶段持续的时间，单位s，默认60s
}BULB_TEST_T;

typedef struct{
	IO_config_t io_config[IO_NUMS];//IO引脚配置
	char io_config_num;//用户配置的IO数量
	char ap_key; //配网按键，表示哪个按键用来进行设备重置，目前有IO 4、5、12、13、14可选
	char reset_switch_times;// 表示几次，现在默认为3次
	char holder;  //占位符，保持字节对齐
	BULB_FUNCTION_T function;//产品需要的配置信息
	BULB_TEST_T product_test_param;//灯泡产测参数
}PRODUCT_BULB_PARAM_T;


/* 每一个产品的配置个体差异在这里定义*/
typedef struct{
	IO_config_t io_config[IO_NUMS];	// IO引脚配置
	function_t	io_func[FUNC_NUMS]; // 按键功能配置   
	char io_config_num;				// 用户配置的IO数量
	char io_func_num;
	char io_indicator_light_r;			// 指示灯管脚，ff为不支持，其他为可用的IO配置
	char io_indicator_light_g;			// 指示灯管脚，ff为不支持，其他为可用的IO配置
	char holder[1];

	char reset_switch_times;// 表示几次，现在默认为3次
	char holder1[3];  //占位符，保持字节对齐
	BULB_FUNCTION_T function;//灯泡产品需要的配置信息
	BULB_TEST_T product_test_param;//灯泡产测参数// 占位符，保持字节对齐

}product_param_t;

typedef struct{
	int baud_rate;	//9600 115200 460800
	char data_bits;	//0:5bits 1:6bits 2:7bits 3:8bits
	char stop_bits;	//1:1bits 2:1.5bits 3:2bits
	char parity;	//0:disable 2:even 3:odd
	char flowcontrol;//not support
}baud_rate_t;

typedef struct{
	device_t device;   		//设备信息
	baud_rate_t baud_config;//波特率信息
	char PTID[64];          //产品配置信息索引
	int product_type;  		//0:灯，以后有新产品再增加
	int product_subtype;    //产品子类型
	int ap_timval;			// 配网窗口期，默认15分钟
	char country_code[4];		//产品所在的国家 //0->CN 1->EU 2->US 3->JP 4->BR 6->OTHER 
	product_param_t param;	//mcu产品配置
}product_config_t;


typedef enum
{	
	REBOOT_OTA_SUCCEED= 0,		 // OTA升级失败启动
	REBOOT_OTA_FAILED= 1,		 // OTA升级失败启动
	REBOOT_NORMAL = 2,		 // 正常启动
} OTA_REBOOT_TYPE;

int product_config_init(void);

product_config_t * get_product_config(void);
device_t* get_product_device_config(void);
IO_config_t* get_product_io_config(void);
char get_product_io_config_num(void);
int get_product_type(void);
int get_product_subtype(void);
char *get_product_PTID(void);
int get_ap_timval(void);
char *get_user_defined_type(void);
baud_rate_t* get_product_baud_config(void);
function_t* get_product_func_config(void);
char get_product_func_num(void);
char get_product_io_indicator_light_r(void);
char get_product_io_indicator_light_g(void);

char *get_product_country_code(void); 
void set_product_country_code(char *CountryCode);


#ifdef BULB_VERSION
char get_product_reset_switch_times(void);

int get_default_cct(void);
int get_memory_timval(void);
int get_reset_time_upper(void);
int get_power_on_light(void);
BULB_TEST_T* get_product_test_param(void);
BULB_FUNCTION_T* get_product_function(void);

typedef enum
{
    LIGHT_W = 0,         // 0,暖黄单色灯
    LIGHT_WC,            // 1,双色CW灯
    LIGHT_RGB,           // 2,三色RGB灯
    LIGHT_RGBW,          // 3,四色RGBW灯
    LIGHT_RGBWC,         // 4,五色RGBWC灯

    LIGHT_CCT,           // 5,双色CCT灯
    LIGHT_RGBCCT,        // 6,五色RGBCCT灯

    LIGHT_C,			 //	冷白单色灯
} light_type;



#endif

#ifdef __cplusplus
}
#endif

#endif //_EZVIZ_PRODUCT_CONFIG_H_
