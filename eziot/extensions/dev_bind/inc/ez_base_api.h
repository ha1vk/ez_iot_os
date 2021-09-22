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
#ifndef _H_EZ_BASE_API_H_
#define _H_EZ_BASE_API_H_

#include "file_interface.h"
#include "io_interface.h"
#include "mem_interface.h"
#include "network_interface.h"
#include "thread_interface.h"
#include "time_interface.h"
#include "base_typedef.h"

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	*  \brief  api return error code 
	*/
	typedef enum
	{
		EZ_SUCCESS = 0     , 
		EZ_FAILED          ,		   
		EZ_NOT_INITED      ,
		EZ_INITED          ,	   
		EZ_INVALID_PARAM   , 
		EZ_REG_ERR         ,  
		EZ_JSON_CREATE_ERR ,  
		EZ_JSON_PRINT_ERR  , 
		EZ_JSON_SEND_ERR   , 
	} ez_base_err;

	/**
	* \brief sdk set msg to device
	*/
	typedef enum
	{
		EZ_SET_USERID,       ///<  set userid to device ,/refer ez_set_userid_t
		EZ_TRANS_CMD,        ///<  /ref ez_trans_cmd_t
	} ez_msg2dev_type_e;
    
	/**
	*  \brief  set userid to device,if device was not bind the value of user_id is "0000000000000000",
	*/
	typedef struct
	{
		char user_id[64];
	} ez_set_userid_t;

	/**
	*  \brief  das msg to device
	*/
	typedef struct
	{
		int   type;   ///<  ez_msg2dev_type_e
		void* data;   ///<  struct  ez_set_userid_t/ ez_trans_cmd_t
		int   len;    ///<  sizeof(struct)
	} ez_msg2dev_t;

	/**
	 * \brief transfer the unresolved cmd 
	*/
	typedef struct 
	{
		char*         content;     ///< msg
		int           content_len; ///< msg len
		unsigned int  cmd_id;      ///< cmd id
		unsigned char msg_type;    ///< 0-req, 1-rsp
		unsigned int  msg_seq;     ///< msg seq
	}ez_trans_cmd_t;

	/**
	* \brief sdk set msg to device
	*/
	typedef enum
	{
		TOUCH_BIND,             ///<  TOUCH_BIND_TOKEN (for eth)
		BIND_USER,              ///<  BIND_USER_TOKEN(for ap)
	} ez_bind_type_e;

	/**
	 * \brief report token to bind device(only choose one type)
	*/
	typedef struct 
	{
		ez_bind_type_e type;      ///< bind token type
		unsigned int   int_token; ///< token with int type (use for eth, vlue:server_token +1), type:TOUCH_BIND
		char*          str_oken;  ///< token with char type (use for ap) type:BIND_USER
	}ez_bind_token_t;

	/**
	 * \brief msg attribute
	*/
	typedef struct 
	{
		unsigned int  msg_qos;     ///< 0-Qos0  !0-Q0s1
		unsigned char msg_type;    ///< 0-req, 1-rsp
		unsigned int  msg_seq;     ///< msg seq
	}ez_attr_t;

	/**
    * \brief  recieve das msg call back function
    */
	typedef struct
	{
		int (*recv_msg)(ez_msg2dev_t *msg);
	} ez_base_cb_t;

	/**
    * \brief	base init info
    */
	typedef struct
	{
		ez_base_cb_t cb;
	} ez_base_init_t;

    /** 
    * @brief ez_base_init,base init api 
    * @param pinit  init info 
    * return 0-success !0-failed   /ref ez_base_err
    */
	EZ_OS_API_EXTERN ez_base_err EZ_OS_API_CALL ez_base_init(const ez_base_init_t *pinit);

	  /** 
    * @brief ez_base_report_bind_token, report bind token 
    * @param ptoken  bind token info 
    * return 0-success !0-failed   /ref ez_base_err
    */
	EZ_OS_API_EXTERN ez_base_err EZ_OS_API_CALL ez_base_report_bind_token(const ez_bind_token_t *ptoken);
    /** 
    * @brief ez_base_send_msg, for sending user msg to das
    * @param buf      user msg buf
	* @param len      buf len
	* @param cmd_id   cmd_id info
	* @param msg_response  init info
	* @param msg_seq  init info
    * return 0-success !0-failed   /ref ez_base_err
    */
	EZ_OS_API_EXTERN ez_base_err EZ_OS_API_CALL ez_base_send_msg(const unsigned char* buf, const unsigned int len, const int cmd_id, ez_attr_t* msg_attr);
    /** 
    * @brief ez_base_set_operation_code, For device binding
    * @param pcode  operation_code
	* @param len    operation_code len ,less than 128
    * return 0-success !0-failed   /ref ez_base_err
    */
	EZ_OS_API_EXTERN ez_base_err EZ_OS_API_CALL ez_base_set_operation_code(const char *pcode, const int len);

	/** 
    * @brief ez_base_query_userid,For querying userid 
    * @param void 
    * return 0-success !0-failed   /ref ez_base_err
    */
	EZ_OS_API_EXTERN ez_base_err EZ_OS_API_CALL ez_base_query_userid();
    /** 
    * @brief ez_base_deinit
    * @param void 
    * return 0-success !0-failed   /ref ez_base_err
    */
	EZ_OS_API_EXTERN ez_base_err EZ_OS_API_CALL ez_base_deinit();

	

#ifdef __cplusplus
}
#endif

/*! \} */

#endif // _H_EZ_BASE_API_H_