
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ezlog.h"
#include "bulb_business.h"
#include "cJSON.h"

#define TAG_CONFIG "T_CONF"

#define COLOR_BLOCK_LENGTH 12
void json2param_colorblock(char *cb_string, light_scene_t *p_scene)
{
    int i = 0;
    int rgb = 0;
    int ct = 0;
    int brightness = 0;
    int len = strlen(cb_string);
    int cb_count = len / COLOR_BLOCK_LENGTH;
    p_scene->cb_count = cb_count;

    for (i = 0; i < cb_count; i++)
    {
        sscanf(cb_string + COLOR_BLOCK_LENGTH * i, "%6x%4x%2x", &rgb, &ct, &brightness);
        printf("%d,%d,%d\n", rgb, ct, brightness);

        p_scene->brightness[i] = brightness;

        if (p_scene->brightness[i] <= 1)
        {
            p_scene->brightness[i] = 1;
        }
        else if (p_scene->brightness[i] >= 100)
        {
            p_scene->brightness[i] = 100;
        }

        p_scene->color_rgb[i] = rgb;

        if (p_scene->color_rgb[i] <= 1)
        {
            p_scene->brightness[i] = 1;
        }
        else if (p_scene->brightness[i] >= 0xffffff)
        {
            p_scene->brightness[i] = 0xffffff;
        }

        p_scene->color_temperature[i] = ct;

        if (p_scene->color_temperature[i] < 2700)
        {
            p_scene->color_temperature[i] = 2700;
        }
        else if (p_scene->color_temperature[i] > 6500)
        {
            p_scene->color_temperature[i] = 6500;
        }

        if (0 == p_scene->color_rgb[i])
        {
            p_scene->type[i] = 0; //whitle
        }
        else
        {
            p_scene->type[i] = 1; //color
        }
    }
}

/*单条场景进行解析，物模型协议是将多个场景做为一个列表带下来，其中使能的协议只有一个*/
int json2param_scene(cJSON *js_obj, light_scene_t *p_scene)
{
    light_scene_t sence_tmp = {0};
    int speed_percent=0;
    cJSON *js_colorblock_string = cJSON_GetObjectItem(js_obj, "colorBlock");
    if (NULL == js_colorblock_string)
    {
        ezlog_e(TAG_APP, "js_colorblock_array array absent.");
        return -1;
    }

    json2param_colorblock(js_colorblock_string->valuestring, &sence_tmp);

    cJSON *js_transform = cJSON_GetObjectItem(js_obj, "transformType"); //变化方式
    if (NULL == js_transform || cJSON_String!= js_transform->type)
    {
        ezlog_e(TAG_APP, "transform type absent.");
        return -1;
    }
    else
    {
        if (0 == strcmp(js_transform->valuestring,"static")) //静态
        {
            sence_tmp.duration = 0;
        }
        else if (1 == strcmp(js_transform->valuestring,"jump")) //跳变
        {
            sence_tmp.duration = 50;
        }
        else //呼吸变化
        {
            sence_tmp.duration = 1500;
        }
    }

    /*新物模型协议中duration         不是必须的，若要实现变化过程速度可控，需要传值，此时将覆盖默认的1500*/
    cJSON *js_duration = cJSON_GetObjectItem(js_obj, "duration");
    if (NULL == js_duration || cJSON_Number != js_duration->type)
    {
        ezlog_i(TAG_APP, "transform duration speed absent.,use default=%d", sence_tmp.duration);
    }
    else
    {
        sence_tmp.duration = js_duration->valueint;
    }

    cJSON *js_transformspeed = cJSON_GetObjectItem(js_obj, "transformSpeed");
    if (NULL == js_transform || cJSON_Number != js_transformspeed->type)
    {
        ezlog_e(TAG_APP, "transform breathing speed absent.");
        return -1;
    }
    else
    {
        speed_percent = js_transformspeed->valueint;
        sence_tmp.speed = speed_percent * 20;     //2000ms 分布到百分比0~100 区间
    }

    ezlog_i(TAG_APP, "scene param. duration:%d, speed:%d,count=%d", sence_tmp.duration, sence_tmp.speed,sence_tmp.cb_count);
    memcpy(p_scene, &sence_tmp, sizeof(light_scene_t));
    return 0;
}

int json2param_CustomSceneCfg(char *scene_json_string, light_scene_t *p_scene)
{
    cJSON *js_root = NULL;
    cJSON *js_obj = NULL;
    cJSON *js_obj_enable = NULL;
    int scene_count = 0;
    int i;
    
    js_root = cJSON_Parse(scene_json_string);
    if (NULL == js_root)
    {
        ezlog_e(TAG_APP, "light_scene parse error no memory.");
        return -1;
    }
    if (cJSON_Object!= js_root->type)
    {
        ezlog_e(TAG_APP, "light_scene parse error.");
        cJSON_Delete(js_root);
        return -1;
    }
    
    cJSON *scene_array = cJSON_GetObjectItem(js_root, "sceneList");
    if (NULL == scene_array || cJSON_Array != scene_array->type)
    {
        ezlog_e(TAG_APP, "parse json failed.");
        cJSON_Delete(js_root);
        return -1;
    }

    scene_count = cJSON_GetArraySize(scene_array);

    for (i = 0; i < scene_count; i++)
    {
        js_obj = cJSON_GetArrayItem(scene_array, i);
        if (NULL == js_obj)
        {
            ezlog_e(TAG_APP, "light_scene parse error.");
            cJSON_Delete(js_root);
            return -1;
        }

        js_obj_enable = cJSON_GetObjectItem(js_obj, "enabled");
        if (NULL == js_obj_enable)
        {
            ezlog_e(TAG_APP, "light_scene parse error.");
            cJSON_Delete(js_root);
            return -1;
        }

        if (true == js_obj_enable->valueint)
        {
            break;
        }
    }

    if (i > scene_count)
    {
        ezlog_e(TAG_APP, "light_scene parse error,no enabled secne.");
        return -1;
    }

    if (0 != json2param_scene(js_obj, p_scene))
    {
        ezlog_e(TAG_APP, "scene parse error,format is error.");
    }

    cJSON_Delete(js_root);
    return 0;
}

//json 字符串时分格式“10:20”，转化到结构体
static plan_tm_t json2param_string2time(const char *ts)
{
    plan_tm_t stru_plan_time = {0};

    char sep;
    int hour = 0, min = 0;

    sscanf(ts, "%d%c%d", &hour, &sep, &min);

    stru_plan_time.tm_hour = hour;
    stru_plan_time.tm_min = min;

    return stru_plan_time;
}

static plan_tm_t time_start2end(plan_tm_t start_time, int sustain)
{
    plan_tm_t stru_plan_time_end = {0};

    int hour_tmp = 0, min_tmp = 0;

    stru_plan_time_end.tm_min = start_time.tm_min + sustain;

    hour_tmp = stru_plan_time_end.tm_min / 60;
    min_tmp = stru_plan_time_end.tm_min % 60;

    stru_plan_time_end.tm_hour = start_time.tm_hour + hour_tmp;
    stru_plan_time_end.tm_min = min_tmp;

    return stru_plan_time_end;
}

static void plan_weekarray_json2param(cJSON *array_js, char week_day[7])
{
    char count;
    int tmp_days[7];
    int i;
    memset((void *)tmp_days, -1, sizeof(tmp_days));
    count = cJSON_GetArraySize(array_js);
    for (i = 0; i < count; i++)
    {
        cJSON *pSub = cJSON_GetArrayItem(array_js, i);
        tmp_days[i] = pSub->valueint;

        if (-1 == tmp_days[i] || tmp_days[i] > 7)
        {

            continue;
        }

        if (7 == tmp_days[i])
        {
            week_day[0] = 1;
        }
        else
        {
            week_day[tmp_days[i]] = 1;
        }
    }
}

void json2param_plan_conf(char *json_string,switch_plan_t *p_swit_plan)
{

    cJSON *js_root = NULL;

    switch_plan_t swit_plan_tmp;
    memset(&swit_plan_tmp, 0, sizeof(swit_plan_tmp));

    js_root = cJSON_Parse(json_string);
    if (NULL == js_root || cJSON_Object != js_root->type)
    {
        ezlog_e(TAG_APP, "parse json failed.");
        goto exit;
    }
    
    cJSON *plan_array = cJSON_GetObjectItem(js_root, "plan");
    if (NULL == plan_array || cJSON_Array != plan_array->type)
    {
        ezlog_e(TAG_APP, "parse json failed.");
        goto exit;
    }

    int value_size = cJSON_GetArraySize(plan_array);

    if (value_size > sizeof(swit_plan_tmp.plan) / sizeof(switch_plan_metadata_t))
    {
        value_size = sizeof(swit_plan_tmp.plan) / sizeof(switch_plan_metadata_t);
    }
    swit_plan_tmp.count = 0;

    for (int k = 0; k < value_size; k++)
    {
        cJSON *js_obj = cJSON_GetArrayItem(plan_array, k);
        cJSON *js_enabled = cJSON_GetObjectItem(js_obj, "enabled");
        cJSON *js_datelist = cJSON_GetObjectItem(js_obj, "RepeatPeriod");
        cJSON *js_actionlist = cJSON_GetObjectItem(js_obj, "Action");
        cJSON *js_start = cJSON_GetObjectItem(js_obj, "startTime");
        cJSON *js_sustain = cJSON_GetObjectItem(js_obj, "sustain");

        if (NULL == js_obj || NULL == js_enabled || NULL == js_datelist ||
            NULL == js_actionlist || NULL == js_start || NULL == js_sustain)
        {
            break;
        }

        swit_plan_tmp.count++;

        int action_count = cJSON_GetArraySize(js_actionlist);
        if (action_count > 1)
        {
            ezlog_i(TAG_APP, "plan action is more than 1,ignor");
        }

        cJSON *js_action_obj = cJSON_GetArrayItem(js_actionlist, 0);
        if (NULL == js_action_obj)
        {
            break;
        }

        cJSON *js_action = cJSON_GetObjectItem(js_action_obj, "startValue");
        if (NULL == js_action)
        {
            break;
        }

        if (0 == strcmp(js_action->valuestring, "on"))
        {
            swit_plan_tmp.plan[k].action = 1;
        }
        else if (0 == strcmp(js_action->valuestring, "off"))
        {
            swit_plan_tmp.plan[k].action = 0;
        }
        else
        {
            ezlog_e(TAG_APP, "plan value invaild.");
            swit_plan_tmp.plan[k].enabled = 0;
            continue;
        }

        swit_plan_tmp.plan[k].enabled = js_enabled->valueint;
        swit_plan_tmp.plan[k].begin = json2param_string2time(js_start->valuestring);
        swit_plan_tmp.plan[k].end = time_start2end(swit_plan_tmp.plan[k].begin, js_sustain->valueint);

        plan_weekarray_json2param(js_datelist, swit_plan_tmp.plan[k].week_days);

        ezlog_i(TAG_APP, "plan[%d], action=%d, enable=%d,begin.h=%d,begin.min=%d,end.h=%d,end.min=%d", k, swit_plan_tmp.plan[k].action,
                swit_plan_tmp.plan[k].enabled,
                swit_plan_tmp.plan[k].begin.tm_hour, swit_plan_tmp.plan[k].begin.tm_min,
                swit_plan_tmp.plan[k].end.tm_hour, swit_plan_tmp.plan[k].end.tm_min);

        ezlog_i(TAG_APP, "plan[%d], d1:%d d2:%d d3:%d d4:%d d5:%d d6:%d d7:%d", k, swit_plan_tmp.plan[k].week_days[1],
                swit_plan_tmp.plan[k].week_days[2], swit_plan_tmp.plan[k].week_days[3],
                swit_plan_tmp.plan[k].week_days[4], swit_plan_tmp.plan[k].week_days[5],
                swit_plan_tmp.plan[k].week_days[6], swit_plan_tmp.plan[k].week_days[0]);
    }
    memcpy(p_swit_plan, &swit_plan_tmp, sizeof(switch_plan_t));

exit:
    js_root ? cJSON_Delete(js_root) : 0;
}

void json2param_action_conf(cJSON *js_array, light_action_t *action)
{
    int action_count = cJSON_GetArraySize(js_array);
    int i;
    int r, g, b;

    for (i = 0; i < action_count; i++)
    {
        cJSON *js_action_obj = cJSON_GetArrayItem(js_array, i);
        if (NULL == js_action_obj)
        {
            ezlog_e(TAG_APP, "sleep action value invaild.");
            break;
        }

        cJSON *js_action = cJSON_GetObjectItem(js_action_obj, "startValue");
        cJSON *js_action_uri = cJSON_GetObjectItem(js_action_obj, "uri");

        if (NULL == js_action || NULL == js_action_uri)
        {
            break;
        }

        if (0 == strcmp(js_action_uri->valuestring, "b")) //某些色块动作定义成string，可以为颜色或者亮度
        {
            if (cJSON_Number == js_action->type)
            {
                action->brightness = js_action->valueint;
            }
            else if (cJSON_String == js_action->type)
            {
                action->brightness = atoi(js_action->valuestring);
            }
            else
            {
                ezlog_e(TAG_APP, "light action brightness value invaild.");
                continue;
            }
        }
        else if (0 == strcmp(js_action_uri->valuestring, "ct"))
        {
            if (cJSON_Number == js_action->type)
            {
                action->color_temperature = js_action->valueint;
            }
            else if (cJSON_String == js_action->type)
            {
                action->color_temperature = atoi(js_action->valuestring);
            }
            else
            {
                ezlog_e(TAG_APP, "light action color_temperature value invaild.");
                continue;
            }
        }
        else if (0 == strcmp(js_action_uri->valuestring, "c"))
        {
            if (cJSON_String == js_action->type)
            {
                sscanf((char *)js_action->valuestring, "%2x%2x%2x", &r, &g, &b);
                action->color_rgb = r * 256 * 256 + g * 256 + b;
            }
            else
            {
                ezlog_e(TAG_APP, "light action color_rgb value invaild.");
                continue;
            }
        }
        else
        {
            ezlog_e(TAG_APP, "light action uri value invaild.");
            continue;
        }
    }
}

int json2param_sleep_conf(char *json_string, light_sleep_plan_t *p_sleep_plan)
{
    cJSON *js_array = cJSON_Parse(json_string);
    if (NULL == js_array)
    {
        ezlog_e(TAG_APP, "light_scene parse error no memory.");
        return -1;
    }
    if (NULL == js_array || cJSON_Array != js_array->type)
    {
        ezlog_e(TAG_APP, "wakeup parse error.");
        cJSON_Delete(js_array);
        return -1;
    }

    light_sleep_plan_t sleep_plan_tmp;
    memset(&sleep_plan_tmp, 0, sizeof(sleep_plan_tmp));

    sleep_plan_tmp.count = 0;

    int value_size = cJSON_GetArraySize(js_array);

    if (value_size > sizeof(sleep_plan_tmp.plan) / sizeof(light_sleep_metadata_t))
    {
        value_size = sizeof(sleep_plan_tmp.plan) / sizeof(light_sleep_metadata_t);
    }

    for (int k = 0; k < value_size; k++)
    {
        cJSON *js_obj = cJSON_GetArrayItem(js_array, k);
        cJSON *js_enabled = cJSON_GetObjectItem(js_obj, "enabled");
        cJSON *js_datelist = cJSON_GetObjectItem(js_obj, "repeatPeriod");
        cJSON *js_actionlist = cJSON_GetObjectItem(js_obj, "action");
        cJSON *js_start = cJSON_GetObjectItem(js_obj, "startTime");

        cJSON *js_custom = cJSON_GetObjectItem(js_obj, "custom");
        cJSON *js_fade = cJSON_GetObjectItem(js_obj, "fade");

        if (NULL == js_obj || NULL == js_enabled || NULL == js_datelist ||
            NULL == js_actionlist || NULL == js_start)
        {
            ezlog_e(TAG_APP, "sleep conf key absent.");

            break;
        }

        sleep_plan_tmp.count++;

        //"action":[{"startValue":"#7F00ff","uri":"c"},{"startValue":"87","uri":"b"}]
        json2param_action_conf(js_actionlist, &sleep_plan_tmp.plan[k].action);

        sleep_plan_tmp.plan[k].custom = js_custom->valueint;
        sleep_plan_tmp.plan[k].fade_time = js_fade->valueint;
        sleep_plan_tmp.plan[k].enabled = js_enabled->valueint;
        sleep_plan_tmp.plan[k].begin = json2param_string2time(js_start->valuestring);

        cJSON *js_sustain = cJSON_GetObjectItem(js_obj, "sustain");
        if (NULL == js_sustain)
        {
            sleep_plan_tmp.plan[k].end = sleep_plan_tmp.plan[k].begin;
        }
        else
        {
            sleep_plan_tmp.plan[k].end = time_start2end(sleep_plan_tmp.plan[k].begin, js_sustain->valueint);
        }

        plan_weekarray_json2param(js_datelist, sleep_plan_tmp.plan[k].week_days);

        ezlog_i(TAG_APP, "plan[%d], action.rgb=0x%x,%d,%d, custom=%d,enable=%d,begin.h=%d,begin.min=%d,end.h=%d,end.min=%d,",
                k, sleep_plan_tmp.plan[k].action.color_rgb,
                sleep_plan_tmp.plan[k].action.color_temperature,
                sleep_plan_tmp.plan[k].action.brightness,
                sleep_plan_tmp.plan[k].custom,
                sleep_plan_tmp.plan[k].enabled,
                sleep_plan_tmp.plan[k].begin.tm_hour, sleep_plan_tmp.plan[k].begin.tm_min,
                sleep_plan_tmp.plan[k].end.tm_hour, sleep_plan_tmp.plan[k].end.tm_min);

        ezlog_i(TAG_APP, "plan[%d], d1:%d d2:%d d3:%d d4:%d d5:%d d6:%d d7:%d", k, sleep_plan_tmp.plan[k].week_days[1],
                sleep_plan_tmp.plan[k].week_days[2], sleep_plan_tmp.plan[k].week_days[3],
                sleep_plan_tmp.plan[k].week_days[4], sleep_plan_tmp.plan[k].week_days[5],
                sleep_plan_tmp.plan[k].week_days[6], sleep_plan_tmp.plan[k].week_days[0]);
    }
    memcpy(p_sleep_plan, &sleep_plan_tmp, sizeof(light_sleep_plan_t));

    js_array ? cJSON_Delete(js_array) : 0;
    return 0;
}

int json2param_countdown(char *json_string, switch_countdown_t *p_countdown)
{
    switch_countdown_t countdown_tmp;
    cJSON *js_root = cJSON_Parse(json_string);
    if (NULL == js_root)
    {
        ezlog_e(TAG_APP, "light_scene parse error no memory.");
        return -1;
    }
    if (NULL == js_root || cJSON_Object != js_root->type)
    {
        ezlog_e(TAG_APP, "countdown parse error.");
        cJSON_Delete(js_root);
        return -1;
    }
    memset(&countdown_tmp, 0, sizeof(countdown_tmp));

    cJSON *js_enable = cJSON_GetObjectItem(js_root, "enable");
    cJSON *js_switch = cJSON_GetObjectItem(js_root, "switch");
    cJSON *js_timeRemaining = cJSON_GetObjectItem(js_root, "timeRemaining");
    
    
    if (NULL == js_enable || NULL == js_switch || NULL == js_timeRemaining)
    {
        return -1;
    }

    countdown_tmp.enabled = js_enable->valueint;
    countdown_tmp.swit = js_switch->valueint;
    countdown_tmp.time_remain_config = js_timeRemaining->valueint;

    cJSON *js_time_stamp = cJSON_GetObjectItem(js_root, "reportTimestamp");
    if(NULL == js_time_stamp)
    {
        countdown_tmp.time_stamp = ezos_time(NULL);
    }
    else
    {
        countdown_tmp.time_stamp = js_time_stamp->valueint;
    }
    ezlog_i(TAG_APP, "countdown swit=%d,enable=%d,timermain=%d,time_stamp=%ld",
                countdown_tmp.swit,
                countdown_tmp.enabled,
                countdown_tmp.time_remain_config,
                countdown_tmp.time_stamp);

    memcpy(p_countdown, &countdown_tmp, sizeof(countdown_tmp));

    js_root ? cJSON_Delete(js_root) : 0;
    return 0;
}

void json2param_biorhythm(char *json_string,biorhythm_t *p_biorhythm_plan)

{
    cJSON *js_root = NULL;

    biorhythm_t biorhythm_tmp;
    memset(&biorhythm_tmp, 0, sizeof(biorhythm_tmp));

    js_root = cJSON_Parse(json_string);
    if (NULL == js_root || cJSON_Object != js_root->type)
    {
        ezlog_e(TAG_APP, "parse json failed.");
        goto exit;
    }

    cJSON *js_enabled = cJSON_GetObjectItem(js_root, "enable");
    cJSON *js_datelist = cJSON_GetObjectItem(js_root, "RepeatPeriod");
    cJSON *js_method = cJSON_GetObjectItem(js_root, "gradientWay");

    if (NULL == js_enabled || NULL == js_datelist || NULL == js_method)
    {
        ezlog_e(TAG_APP, "biorhythm_conf_json2param json absent.");
        goto exit;
    }

    plan_weekarray_json2param(js_datelist, biorhythm_tmp.week_days);

    biorhythm_tmp.enabled = js_enabled->valueint;

    if (0 == strcmp(js_method->valuestring, "entireGradient")) //全程渐变
    {
        biorhythm_tmp.method = 1;
    }
    else if (0 == strcmp(js_method->valuestring, "directGradient")) // 直接渐变
    {
        biorhythm_tmp.method = 0;
    }
    else
    {
        ezlog_e(TAG_APP, "biorhythm gradientWay method invaild.");
        goto exit;
    }

    cJSON *plan_array = cJSON_GetObjectItem(js_root, "rhythmPlan");
    if (NULL == js_root || cJSON_Array != plan_array->type)
    {
        ezlog_e(TAG_APP, "parse json failed.");
        goto exit;
    }

    int value_size = cJSON_GetArraySize(plan_array);

    if (value_size > sizeof(biorhythm_tmp.plan) / sizeof(biorhythm_metadata_t))
    {
        value_size = sizeof(biorhythm_tmp.plan) / sizeof(biorhythm_metadata_t);
    }

    biorhythm_tmp.count = 0;

    for (int k = 0; k < value_size; k++)
    {
        cJSON *js_obj = cJSON_GetArrayItem(plan_array, k);

        cJSON *js_actionlist = cJSON_GetObjectItem(js_obj, "action");
        cJSON *js_start = cJSON_GetObjectItem(js_obj, "startTime");
        cJSON *js_plan_enable = cJSON_GetObjectItem(js_obj, "enable");
        cJSON *js_plan_sustain = cJSON_GetObjectItem(js_obj, "sustain");

        if (NULL == js_actionlist || NULL == js_plan_enable || NULL == js_start)
        {
            ezlog_e(TAG_APP, "parase rhythmPlan json failed.");
            continue;
        }

        biorhythm_tmp.count++;

        json2param_action_conf(js_actionlist, &biorhythm_tmp.plan[k].action);

        biorhythm_tmp.plan[k].enabled = js_plan_enable->valueint;
        biorhythm_tmp.plan[k].begin = json2param_string2time(js_start->valuestring);

        if (NULL == js_plan_sustain)
        {
            biorhythm_tmp.plan[k].end = biorhythm_tmp.plan[k].begin;
        }
        else
        {
            biorhythm_tmp.plan[k].end = time_start2end(biorhythm_tmp.plan[k].begin, js_plan_sustain->valueint);
        }

        ezlog_i(TAG_APP, "plan[%d], action.rgb=0x%x,%d,%d, enable=%d,begin.h=%d,begin.min=%d,end.h=%d,end.min=%d",
                k, biorhythm_tmp.plan[k].action.color_rgb,
                biorhythm_tmp.plan[k].action.color_temperature,
                biorhythm_tmp.plan[k].action.brightness,
                biorhythm_tmp.plan[k].enabled,
                biorhythm_tmp.plan[k].begin.tm_hour, biorhythm_tmp.plan[k].begin.tm_min,
                biorhythm_tmp.plan[k].end.tm_hour, biorhythm_tmp.plan[k].end.tm_min);

        ezlog_i(TAG_APP, "plan[%d], d1:%d d2:%d d3:%d d4:%d d5:%d d6:%d d7:%d", k, biorhythm_tmp.week_days[1],
                biorhythm_tmp.week_days[2], biorhythm_tmp.week_days[3],
                biorhythm_tmp.week_days[4], biorhythm_tmp.week_days[5],
                biorhythm_tmp.week_days[6], biorhythm_tmp.week_days[0]);
    }

    memcpy(p_biorhythm_plan, &biorhythm_tmp, sizeof(biorhythm_t));

exit:
    js_root ? cJSON_Delete(js_root) : 0;
}

