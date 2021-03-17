#ifndef _EZ_IOT_ERRNO_H_
#define _EZ_IOT_ERRNO_H_

typedef enum ez_err
{
    ez_errno_succ,                                              ///< success
    ez_errno_not_init               = ez_errno_succ + 0x01,     ///< The sdk core module is not initialized
    ez_errno_not_ready              = ez_errno_succ + 0x02,     ///< The sdk core module is not started
    ez_errno_param_invalid          = ez_errno_succ + 0x03,     ///< The input parameters is illegal, it may be that some parameters can not be null or out of range
    ez_errno_internal               = ez_errno_succ + 0x04,     ///< Unknown error
    ez_errno_memory                 = ez_errno_succ + 0x05,     ///< Out of memory

    ez_errno_ota_base               = 0x00020000,               ///< Tsl interface error code base
    ez_errno_ota_not_init           = ez_errno_ota_base + 0x01, ///< The ota module is not initialized
    ez_errno_ota_not_ready          = ez_errno_ota_base + 0x02, ///< The sdk core module is not started
    ez_errno_ota_param_invalid      = ez_errno_ota_base + 0x03, ///< The input parameters is illegal, it may be that some parameters can not be null or out of range
    ez_errno_ota_internal           = ez_errno_ota_base + 0x04, ///< Unknown error
    ez_errno_ota_memory             = ez_errno_ota_base + 0x05, ///< Out of memory
    ez_errno_ota_register_failed    = ez_errno_ota_base + 0x06, ///< register_failed 
    ez_errno_ota_json_creat_err     = ez_errno_ota_base + 0x07, ///< json_creat_err
    ez_errno_ota_json_format_err    = ez_errno_ota_base + 0x08, ///< json_format_err 
    ez_errno_ota_msg_send_err       = ez_errno_ota_base + 0x09, ///< msg_send_err
    ez_errno_ota_download_already                             ,  ///< download already 
} ez_err_e;

#endif