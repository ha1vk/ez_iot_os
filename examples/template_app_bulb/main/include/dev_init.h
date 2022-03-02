#ifndef _EZVIZ_INIT_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _EZVIZ_INIT_H_

typedef enum {
    GPIO_DRIVER_MODE_IO = 0,
    GPIO_DRIVER_MODE_PWM = 1,
    GPIO_DRIVER_MODE_I2C = 2, 
    GPIO_DRIVER_MODE_SPI = 3, 
}gpio_driver_mode_t;

typedef enum {
    LIGHT = 0,
    MCU = 1,
}product_type_t;

int set_gpio_config(void);
int mk_soft_version(char *pversion_buf);
int light_start(void);

#ifdef __cplusplus
}
#endif

#endif //_EZVIZ_INIT_H_
