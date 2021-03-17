#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ez_sdk_ota.h"


static ota_msg_cb_t g_ota_cb;

void  ez_ota_user_init(ota_msg_cb_t cb)
{
    g_ota_cb = cb;
}

ota_msg_cb_t *ez_ota_get_callback()
{
    return &g_ota_cb;
}
