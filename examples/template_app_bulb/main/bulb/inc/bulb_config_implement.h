#ifndef _BULB_CONFIG_IMPLEMENT_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "ezos_def.h"
#define _BULB_CONFIG_IMPLEMENT_H_

/**
 * @brief 
 * 
 * @param cmd 
 * @param arg 
 * @return 参考 ez_core_err_e
 */
int json2param_CustomSceneCfg(char *scene_json_string, light_scene_t *p_scene);

/**
* @brief 
* 
* @param cmd 
* @param arg 
* @return 参考 ez_core_err_e
*/
void json2param_plan_conf(char *json_string,switch_plan_t *p_swit_plan);


/**
* @brief 
* 
* @param cmd 
* @param arg 
* @return 参考 ez_core_err_e
*/
int json2param_sleep_conf(char *json_string, light_sleep_plan_t *p_sleep_plan);

/**
* @brief 
* 
* @param cmd 
* @param arg 
* @return 参考 ez_core_err_e
*/
int json2param_countdown(char *json_string, switch_countdown_t *p_countdown);

/**
* @brief 
* 
* @param cmd 
* @param arg 
* @return 参考 ez_core_err_e
*/
void json2param_biorhythm(char *json_string,biorhythm_t *p_biorhythm_plan);
  	
#ifdef __cplusplus
}
#endif
#endif /* _EZVIZ_CONFIG_IMPLEMENT_H_ */
