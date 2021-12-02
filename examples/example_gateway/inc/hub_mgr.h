#ifndef _HUB_MGR_H_
#define _HUB_MGR_H_

#include<stdbool.h>
//#include"ez_iot.h"
#include"ez_iot_hub.h"
//#include "ez_iot_ap.h"


#define  SUBDEV_ALIVE_DEAL_INTERVALTIME  5  //默认子设备心跳处理间隔时间

#define EZ_SUBDEV_KEY_ALIVELIST   "subdev_alivelist"  //子设备的心跳参数链表


#define  SUBDEV_TEST_SN   "ATOAUV0AY9S0"
#define  SUBDEV_TEST_PID  "243671E149854925BF18DD"
#define  SUBDEV_TEST_LICENSE  "GaxjvGH6tdSPKk3xYfehbG"
#define  SUBDEV_TEST_VERSION  "V1.2.2 build 210521"


void timer_deal(void);


EZ_INT subdev_alive_init(void);

ez_int_t subdev_alive_update_example(void);

EZ_INT ez_subdev_alive_del(const ez_char_t *subdev_sn);

void adddev_overtime_close_window(void);

void add_subdev_example(void);

void del_subdev_example(void);

void open_add_subdev(char *value);

void close_add_subdev(void);

ez_int_t ez_subdev_alive_add(const ez_subdev_info_t *subdev_info);


extern ez_char_t *g_addsubdev_UUID;

#define S2J_JSON_SET_int_ELEMENT(to_json, from_struct, _element) \
    cJSON_AddNumberToObject(to_json, #_element, (from_struct)->_element);

#define S2J_JSON_SET_double_ELEMENT(to_json, from_struct, _element) \
    cJSON_AddNumberToObject(to_json, #_element, (from_struct)->_element);

#define S2J_JSON_SET_string_ELEMENT(to_json, from_struct, _element) \
    cJSON_AddStringToObject(to_json, #_element, (from_struct)->_element);

#define S2J_JSON_SET_BASIC_ELEMENT(to_json, from_struct, type, _element) \
    S2J_JSON_SET_##type##_ELEMENT(to_json, from_struct, _element)


/* Set basic type element for JSON object */
#define s2j_json_set_basic_element(to_json, from_struct, type, element) \
    S2J_JSON_SET_BASIC_ELEMENT(to_json, from_struct, type, element)




typedef struct
{
    ez_int8_t subdev_sn[64];	///< 子设备序列号
    ez_int32_t alive_intervaltime;	///< 子设备心跳间隔时间
    ez_int32_t subdev_overtime_s; ///< 子设备超时时间
} hub_subdev_alive_t;

typedef struct              //描述本次添加的子设备信息，结构体
{
    ez_uchar_t pass[10];   //0表示未认证  ，1表示通过
    ez_bool_t identify_finish;  //0表示认证中 ， 1表示认证已完成
    ez_int8_t subdev_sn[64];	///< 子设备序列号
    ez_int32_t adding_time_s;     //添加过程计时
    
} subdev_type_t;


typedef struct              //描述本次添加的子设备信息，结构体
{
    ez_int8_t addsubdev_num;    ///本次添加的子设备数量
    ez_int8_t addsubdev_pass_num;  ///< 验证通过的子设备数量
    subdev_type_t addsubdev[COMPONENT_HUB_SUBLIST_MAX];   ///< 子设备信息
} add_subdev_info_t;



#endif


