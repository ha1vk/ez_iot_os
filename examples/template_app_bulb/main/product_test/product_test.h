#ifndef _EZVIZ_PRODUCT_TEST_H
#define _EZVIZ_PRODUCT_TEST_H

typedef enum
{
    LAMP_W  = 0,        // 单色灯
    LAMP_WC,            // 双色CW灯
    LAMP_RGB,           // 三色RGB灯
    LAMP_RGBW,          // 四色RGBW灯
    LAMP_RGBWC,         // 五色RGBWC灯

    LAMP_CCT,           // 双色CCT灯
    LAMP_RGBCCT,        // 五色RGBCCT灯

    LAMP_C,
} PRODUCT_TYPE_E;

/*
 *  @brief  设置产品类型，参见：PRODUCT_TYPE
 *  @return 0：支持的产品类型，其他不支持的产品类型
 *  @info   参数从配置文件读取，在产测流程开始之前调用
 *  @brief  产测流程开始
 */
int ez_product_test(int type);

/**
 * @brief 判断产测流程是否结束
 * 
 * @return int 
 */
int is_product_test_done();

#endif
