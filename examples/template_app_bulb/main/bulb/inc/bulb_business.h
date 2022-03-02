#ifndef EZ_IOT_LIGHT_BUSINESS_H
#define EZ_IOT_LIGHT_BUSINESS_H
#include <stdbool.h>
#include "ezos_def.h"
#include "ezos_time.h"
#define TAG_LIGHT                         "T_LIGHT"

typedef struct
{
    ez_int32_t brightness[8];
    ez_int32_t color_rgb[8];
    ez_int32_t color_temperature[8];
    ez_int8_t type[8];//0:white 1:color
    ez_int8_t transform_type;   //静止，动态
    ez_int8_t cb_count;
    ez_int8_t res[2];
    ez_int32_t duration; // 新协议不使用
    ez_int32_t speed;   //变化速度
    //ez_int32_t scene_id; //场景的id号，app需要用，设备不需要用
    //ez_int8_t szSceneName[16]; //场景名称，app需要用，设备不需要用
    bool random;
}light_scene_t;

typedef struct plan_tm
{
    ez_int8_t tm_min;
    ez_int8_t tm_hour;
	ez_int8_t tm_sec;
    ez_int8_t res[1];
}plan_tm_t;

/* 
唤醒入睡执行动作有多个，分调节亮度，颜色，色温，颜色和色温优先颜色，颜色为0时，执行色温
*/
typedef struct
{
    ez_int32_t brightness;
    ez_int32_t color_rgb;
    ez_int32_t color_temperature;
}light_action_t;

/*40字节*/
typedef struct
{
    plan_tm_t begin;                //< 计划开始时间
    ez_int8_t week_days[7];
    ez_int8_t enabled;
    ez_int32_t fade_time;                  //唤醒变化时间
    plan_tm_t end;                    //唤醒后持续一段时间,入睡不需要
    light_action_t action;
    ez_int32_t custom;
    ez_int32_t  start_count;  //唤醒计划开始执行的次数，定时器实现方式，用以多次执行动作
}light_sleep_metadata_t;

/*40*8+4= 324字节 */
typedef struct
{
    light_sleep_metadata_t plan[8];  //唤醒计划消耗内存以及flash较多，设计规格8个
    ez_int8_t count;
    ez_int8_t res[3];
} light_sleep_plan_t;

/*64 字节*/
typedef struct
{
    plan_tm_t begin; ///< 计划开始时间
    ez_int8_t week_days[7];
    ez_int8_t enabled;
    plan_tm_t end;   ///< 新协议为一个持续时间sustain,可转化为结束时间
    ez_int8_t action;         //动作只有一个，开与关
    ez_int8_t  start_count;  //定时计划开始执行的次数，定时器实现方式，用以多次进入时只执行一次
    ez_int8_t  end_count;     //定时计划结束执行的次数，定时器实现方式，用以多次进入时只执行一次
    ez_int8_t res[1];
} switch_plan_metadata_t;

/*64*8+4= 516字节 */
typedef struct
{
    switch_plan_metadata_t plan[8];  //定时计划消耗内存以及flash较多，设计规格8个
    ez_int8_t count;
    ez_int8_t res[3];
} switch_plan_t;

typedef struct
{
    plan_tm_t begin; ///< 色块开始时间
    light_action_t action;
    ez_int8_t enabled;
    plan_tm_t end;   ///< 新协议为一个持续时间sustain,可转化为结束时间
    ez_int8_t  start_count;  //定时计划开始执行的次数，定时器实现方式，用以多次进入时只执行一次
    ez_int8_t res2[3];
} biorhythm_metadata_t;

/*64*8+4= 516字节 */
typedef struct
{
    biorhythm_metadata_t plan[8];  //定时计划消耗内存以及flash较多，设计规格8个
    ez_int8_t count;
    ez_int8_t res1[3];
    ez_int8_t week_days[7];
    ez_int8_t enabled;
    ez_int8_t method ;
    ez_int8_t res2[3];
} biorhythm_t;

/*64 字节*/
typedef struct
{
    ez_int8_t enabled;
    ez_int8_t swit;                 //开关
    ez_int32_t time_remain;
    ezos_time_t time_stamp;
} switch_countdown_t;

/*灯的显示参数*/
typedef struct
{
    ez_int8_t swit;        //灯开关状态，0或者1
    ez_int8_t mode;        //mode版本号，mode表示灯是cct模式还是rgb模式
    ez_int16_t cct;         //cct版本号，2700~6500
    ez_int16_t brightness;  //灯的亮度0-1000
    ez_int8_t res[2];
    ez_int32_t rgb;         //rgb颜色值，0xffeebb  
    switch_countdown_t countdown;
    light_scene_t scene; //场景执行参数，协议下发多个协议，其中一个协议需要转化
    switch_plan_t swit_plan; //开关定时计划
    light_sleep_plan_t help_sleep_plan;
    light_sleep_plan_t wakeup_plan;
    biorhythm_t biorhythm_plan;
} light_param_t;
	
typedef enum
{
    LIGHT_COLOR,
    LIGHT_WHITE,
    LIGHT_SCENE,
    LIGHT_MUSIC,
} LIGHT_MODE_T;

ez_int32_t turn_on_lamp();
ez_int32_t turn_off_lamp();



bool get_light_switch();
ez_int32_t get_light_brightness();
ez_int32_t get_light_color_temperature();
ez_int32_t get_light_color_rgb();
ez_int32_t get_light_mode();
void set_light_mode();

//ez_int32_t get_light_countdown_on_left_time();
//ez_int32_t get_light_countdown_off_left_time();
//bool is_countdown_enable();
//bool is_countdown_on_or_off();

ez_int32_t adjust_light_brightness(ez_int32_t light_brightness);
ez_int32_t adjust_light_cct(ez_int32_t light_cct);
ez_int32_t light_brigtness_to_limit(ez_int32_t light_brightness);

ez_int32_t adjust_light_rgb(ez_int32_t light_rgb);

int set_light_scene(char *light_scene);

int set_light_wakeup(char *json_string);

int set_light_plan(char *json_string);

int set_light_helpsleep(char *json_string);

int set_light_biorhythm(char *json_string);

int set_light_countdown(char *json_string);


//ez_int32_t countdown_light_on(ez_int32_t count_down);
//ez_int32_t countdown_light_off(ez_int32_t count_down);
//ez_int32_t countdown_light_on_cancel();
//ez_int32_t countdown_light_off_cancel();

//ez_int32_t countdown_light_query();

ez_int32_t reset_light(ez_int32_t mode);
ez_int32_t reset_network();
ez_int32_t restart_light();

ez_int32_t get_brightness_limit();
void bulb_ctrl_init();

#endif
