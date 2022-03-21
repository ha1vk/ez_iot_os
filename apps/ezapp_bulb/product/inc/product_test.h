/*******************************************************************************
 * Copyright © 2017-2022 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 * 
 * Contributors:
 * ChenTengfei (chentengfei5@ezvizlife.com) - Smart bulb application finished product production test
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-03-15     ChenTengfei  first version 
 *******************************************************************************/

#ifndef _PRODUCT_TEST_H_
#define _PRODUCT_TEST_H_

#include "ezos.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        LAMP_W = 0, // 单色灯
        LAMP_WC,    // 双色CW灯
        LAMP_RGB,   // 三色RGB灯
        LAMP_RGBW,  // 四色RGBW灯
        LAMP_RGBWC, // 五色RGBWC灯

        LAMP_CCT,    // 双色CCT灯
        LAMP_RGBCCT, // 五色RGBCCT灯

        LAMP_C,
    } product_type_e;

   /**
    * @brief 智能灯成品产测
    * 
    * @warning 产测1必选，未完成无法正常使用，只能测试一次。
    *          产测2可选，可反复测试。
    * 
    * @param type 灯类型
    * 
    * @return 0-已完成产测; !0-异常情况（内存不足/配置有问题）; 阻塞未返回-产测未通过
    */
    ez_int32_t product_test_do(product_type_e type);

#ifdef __cplusplus
}
#endif

#endif