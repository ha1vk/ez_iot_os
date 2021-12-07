#ifndef EZ_IOT_LIGHT_BUSINESS_H
#define EZ_IOT_LIGHT_BUSINESS_H
#include <stdbool.h>
#include "ezos_def.h"

#define TAG_LIGHT                         "T_LIGHT"

typedef struct
{
    int brightness[8];
    int color_rgb[8];
    int color_temperature[8];
    int type[8];//0:white 1:color
    int low;
    int duration;
    int speed;
    int index;
    char szSceneName[16];
}light_scene_t;


typedef struct
{
    ez_int8_t swit;        //灯开关状态，0或者1
    ez_int8_t mode;        //mode版本号，mode表示灯是cct模式还是rgb模式
    ez_int16_t cct;         //cct版本号，2700~6500
    ez_int16_t brightness;  //灯的亮度0-1000
    ez_int8_t res[2];
    ez_int32_t rgb;         //rgb版本号，0xffeebb        
    //time_zone_t time_zone;
    //light_scene_t scene; //场景执行参数
    //scene_conf; // 存储在fflash 的多个场景
    //switch_plan_t swit_plan; //开关定时计划版本号
} light_param_t;
	
typedef enum
{
    LIGHT_COLOR,
    LIGHT_WHITE,
    LIGHT_SCENE,
    LIGHT_MUSIC,
} LIGHT_MODE_T;

int turn_on_lamp();
int turn_off_lamp();



bool get_light_switch();
int get_light_brightness();
int get_light_color_temperature();
int get_light_color_rgb();
int get_light_mode();
int get_light_countdown_on_left_time();
int get_light_countdown_off_left_time();
bool is_countdown_enable();
bool is_countdown_on_or_off();



int adjust_light_brightness(int light_brightness);
int adjust_light_cct(int light_cct);
int light_brigtness_to_limit(int light_brightness);

int adjust_light_rgb(int light_rgb);
int countdown_light_on(int count_down);
int countdown_light_off(int count_down);

int countdown_light_on_cancel();
int countdown_light_off_cancel();

int countdown_light_query();

int reset_light(int mode);
int reset_network();
int restart_light();

int get_brightness_limit();
void bulb_ctrl_init();

#endif