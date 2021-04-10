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
* Contributors:
 *    shenhongyin - initial API and implementation and/or initial documentation
 *******************************************************************************/
#ifndef _ERROR_H_
#define _ERROR_H_

#define CIVIL_RESULT_GENERAL_NO_ERROR                                   0x00000000 ///< no err
#define CIVIL_RESULT_GENERAL_UNKNOW_ERROR                               0x00000001 ///< unknow err
#define CIVIL_RESULT_GENERAL_PARAMS_ERROR                               0x00000002 ///< param err
#define CIVIL_RESULT_GENERAL_PARSE_FAILED                               0x00000003 ///< protocol parse err
#define CIVIL_RESULT_GENERAL_SYSTEM_ERROR                               0x00000004 ///< system err(common err)
#define CIVIL_RESULT_GENERAL_DEVICE_NOT_EXIST                           0x00100009 ///< device not exist
#define CIVIL_RESULT_GENERAL_COMMAND_UNKNOW                             0x00000081 ///< illegal cmd
#define CIVIL_RESULT_GENERAL_COMMAND_NOT_SUITABLE                       0x00000083 ///< incorrect command format
#define CIVIL_RESULT_GENERAL_COMMAND_NOT_ALLOW                          0x00000084 ///< unauthorized command
#define CIVIL_RESULT_PU_CHALLENGE_CODE_VERIFY_FAILED                    0x00100601 ///< verify challenge code failed
 
#endif // _ERROR_H_
