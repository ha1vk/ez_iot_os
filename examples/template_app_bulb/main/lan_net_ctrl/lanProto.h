/** \file P2PV3Protocol.h
 * \copyright Copyright (c) 2017 by HangZhou Ezviz Network Co., Ltd. All Right Reserved.
 * \brief This file describes definition of class CRelayProto.  http://nvwa.hikvision.com.cn/pages/viewpage.action?pageId=263258429
 * 
 * \author liwei
 * \date   2020/10/14
 *
 * \code history 
 *          [2020/10/14] Pi.Kongxuan
 *          Initialize the file
 */ 

#ifndef __LAN_PROTO_H__
#define __LAN_PROTO_H__

// 命令编码定义, 1 Bytes
typedef enum tag_LanCMDType
{
    LAN_CMD_Heartbeat                   = 0X01,           ///< 客户端和设备端心跳维持
    LAN_CMD_DataTransfer                = 0XFF            ///< 数据传输
}LAN_CMD_TYPE_E;

//报文属性字符定义, 报文KeyCode定义, 1 Byte
typedef enum tag_LanKeyCode
{
    LAN_KC_Result           = 0X01,         ///< 信令处理响应结果
    LAN_KC_Content          = 0X02,         ///< 透传信令里的透传内容字段
}LAN_KEY_CODE;

// P2P Server返回错误码定义
typedef enum tag_LanErrorCode
{
    LAN_ERR_NO_ERROR               = 0,		    ///< 成功
    LAN_ERR_PARAMETER              = 1,         ///< 参数错误
    LAN_ERR_PARSE_PACKET           = 2,         ///< 解析报文错误
    LAN_ERR_PROTO                  = 3,         ///< 协议格式不对
}ERROR_CODE_E;

/*8 byte*/
typedef struct tag_LanHeader
{
    unsigned char   flag;       ///< 私有协议头标识符，一个字节，值只能为'$'
    unsigned char   type;       ///< 具体含义，参见 #LanKeyCode
    unsigned short  len;        ///< 消息体长度
    unsigned short  seq;         //请求序号
    unsigned char   ver;         //协议版本, 从1开始
    unsigned char   reserv;      //预留字段 
}LAN_HEADER_T;

#define HPR_INFINITE 0xFFFFFFFF
#define LAN_TYPE_HEARBEAT 0x1
#define LAN_TYPE_REGISTER_REQUEST 0x2
#define LAN_TYPE_REGISTER_RESPONSE 0x3
#define LAN_TYPE_DATA_TRANSFER 0xFF


//KLV 的K值，代表消息体的类型，实际同一LAN_TYPE下，可以有多个KLV，比较灵活、
#define LAN_KEY_RESULT 0x1              //request对应的response 的消息体类型
#define LAN_KEY_CONTENT 0x2              //request透传
#define LAN_KEY_TOKEN 0x3 


#define LAN_KEY_RESULT_SUC 0		
#define LAN_KEY_RESULT_FAIL -1		

typedef struct tag_LanResult
{
    unsigned char   key;       // 
	unsigned short	len;	   // 
	unsigned char	value;	   //
}LAN_RESULT_T;
//3字节
typedef struct tag_LanResult1
{
    unsigned char   key;       // 
	unsigned short	len;	   // 
}LAN_RESULT1_T;

typedef struct tag_LanResult2
{
	unsigned char	value;	   //
}LAN_RESULT2_T;



#define HPR_INFINITE 0xFFFFFFFF

enum
{
    HPR_SOCKET_STATUS_OK = 0,
    HPR_SOCKET_STATUS_ERROR = -1,
    HPR_SOCKET_STATUS_OVERTIME = -2,
    HPR_SOCKET_STATUS_REMOTE_CLOSED = -3,
};

#define MUSIC_LED_CTRL 1

#endif

