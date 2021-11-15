/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
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
 * XuRongjun (xurongjun@ezvizlife.com)
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-11-05     xurongjun    first version 
 *******************************************************************************/

#ifndef _EZ_KERNEL_LOWLVL_H_
#define _EZ_KERNEL_LOWLVL_H_

#include <ezos.h>
#include "ez_iot_core_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 
     * 
     * @param psrv_info 
     * @param pdev_info 
     * @param devid 
     * @param kernel_event_notice_cb 
     * @return EZOS_API 
     */
    EZOS_API ez_err_t EZOS_CALL ez_kernel_init(const ez_server_info_t *psrv_info, const ez_dev_info_t *pdev_info,
                                               const ez_byte_t *devid, sdk_kernel_event_notice kernel_event_notice_cb);

    /**
     * @brief 
     * 
     * @return EZOS_API 
     */
    EZOS_API ez_err_t EZOS_CALL ez_kernel_start();

    /**
     * @brief 
     * 
     * @return EZOS_API 
     */
    EZOS_API ez_err_t EZOS_CALL ez_kernel_stop();

    /**
     * @brief 
     * 
     * @return EZOS_API 
     */
    EZOS_API ez_err_t EZOS_CALL ez_kernel_fini();

    /**
     * @brief 
     * 
     * @return EZOS_API 
     */
    EZOS_API ez_err_t EZOS_CALL ez_kernel_yield();

    /**
     * @brief 
     * 
     * @return EZOS_API 
     */
    EZOS_API ez_err_t EZOS_CALL ez_kernel_yield_user();

    /**
     * @brief 
     * 
     * @param external_extend 
     * @return EZOS_API 
     */
    EZOS_API ez_err_t EZOS_CALL ez_kernel_extend_load(const ez_kernel_extend_t *external_extend);

    /**
     * @brief 
     * 
     * @param pubmsg 
     * @return EZOS_API 
     */
    EZOS_API ez_err_t EZOS_CALL ez_kernel_send(ez_kernel_pubmsg_t *pubmsg);

    /**
     * @brief 
     * 
     * @param external_extend 
     * @return EZOS_API 
     */
    EZOS_API ez_err_t EZOS_CALL ez_kernel_extend_load_v3(const ez_kernel_extend_v3_t *external_extend);

    /**
     * @brief 
     * 
     * @param pubmsg 
     * @return EZOS_API 
     */
    EZOS_API ez_err_t EZOS_CALL ez_kernel_send_v3(ez_kernel_pubmsg_v3_t *pubmsg);

    /**
     * @brief 
     * 
     * @param key 
     * @return EZOS_API const* 
     */
    EZOS_API const ez_char_t *EZOS_CALL ez_kernel_getdevinfo_bykey(const ez_char_t *key);

#ifdef __cplusplus
}
#endif

#endif