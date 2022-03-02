#ifndef _EZVIZ_LED_CTRL_H_
#ifdef __cplusplus
extern "C"
{
#endif
#define _EZVIZ_LED_CTRL_H_

#include <stdint.h>
#include "ezos_def.h"

    /*24 byte*/
    typedef struct tag_LedCtrlParam
    {
        char cmdtype;
        char ver;
        char res[2];
        char res2[3];
        char keyFrame; //0代表小节奏，1代表大节奏
        short nbrightness;
        short nCctValue;      //2700-6500
        int iRgbValue;        //彩灯颜色，若不为空，则cctvalue 无效
        short nLowBrightness; //亮度先变高再变低时的最低亮度，1--100,若不需要变低，此值设置成0
        short nUpDuration;    //颜色变化时间  20ms-10000ms
        short nSpeed;         //最高亮度持续时间20ms - 10000ms,
        short nDownDuration;  //颜色变化时间  20ms-10000ms,依赖nLowBrightness
    } led_ctrl_t;

    int led_ctrl_rgb(int dst_lm, int dst_rgb, int sleep_ms, int rgb_loops);

    ez_int32_t led_ctrl_cct(int dst_cct, int dst_lm, int sleep_ms, int chg_loops);

    int led_ctrl_do_async(led_ctrl_t *struLedCtrl);

    int led_ctrl_do_sync(led_ctrl_t *struLedCtrl);

    void led_ctrl_Task();

#ifdef __cplusplus
}
#endif
#endif /* _EZVIZ_LED_CTRL_H_ */
