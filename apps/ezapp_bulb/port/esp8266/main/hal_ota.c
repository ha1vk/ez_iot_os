#include "ezlog.h"
#include "esp_system.h"     //乐鑫头文件
esp_partition_t *pUpdate_partition; //乐鑫备份升级中的分区
esp_ota_handle_t update_handle;     //乐鑫的升级写flash句柄

ez_err_t hal_ota_begin(ez_size_t image_size)
{
    int rv = -1;
    pUpdate_partition = esp_ota_get_next_update_partition(NULL);
    if (!pUpdate_partition)
    {
        ezlog_e(TAG_OTA, "get next partition failed");
        return rv;
    }
    rv = esp_ota_begin(pUpdate_partition, image_size, &update_handle); //乐鑫的ota的begin函数
    return rv;
}

ez_err_t hal_ota_write(const ez_void_t *data, ez_size_t size)
{
    int rv = -1;
    rv = esp_ota_write(update_handle, data, size);
    return rv;
}

ez_err_t hal_ota_end(ez_void_t)
{
    int rv = -1;
    rv = esp_ota_end(update_handle);
    return rv;
}

ez_err_t hal_ota_action(ez_void_t)
{
    esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL));
    esp_restart();
    return;
}