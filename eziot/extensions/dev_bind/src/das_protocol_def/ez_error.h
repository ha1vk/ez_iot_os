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

#define CIVIL_RESULT_GENERAL_NO_ERROR                                   0x00000000 ///< 无错误
#define CIVIL_RESULT_GENERAL_UNKNOW_ERROR                               0x00000001 ///< 未知错误
#define CIVIL_RESULT_GENERAL_PARAMS_ERROR                               0x00000002 ///< 参数错误
#define CIVIL_RESULT_GENERAL_PARSE_FAILED                               0x00000003 ///< 报文解析失败
#define CIVIL_RESULT_GENERAL_SYSTEM_ERROR                               0x00000004 ///< 系统内部错误
#define CIVIL_RESULT_GENERAL_DEVICE_NOT_EXIST                           0x00100009 ///< 设备不存在


#define CIVIL_RESULT_GENERAL_COMMAND_UNKNOW                             0x00000081 ///< 非法命令
#define CIVIL_RESULT_GENERAL_COMMAND_NO_LONGER_SUPPORTED                0x00000082 ///< 过时命令
#define CIVIL_RESULT_GENERAL_COMMAND_NOT_SUITABLE                       0x00000083 ///< 命令格式不正确
#define CIVIL_RESULT_GENERAL_COMMAND_NOT_ALLOW                          0x00000084 ///< 未授权命令


#define CIVIL_RESULT_GENERAL_CHECKSUM_ERROR                             0x00000101 ///< 报文校验和错误
#define CIVIL_RESULT_GENERAL_HEADER_INVALID                             0x00000102	// 消息头非法
#define CIVIL_RESULT_GENERAL_LENGTH_INVALID                             0x00000103	// 消息头非法

#define CIVIL_RESULT_GENERAL_VERSION_UNKNOW                             0x00000181 ///< 协议版本错误
#define CIVIL_RESULT_GENERAL_VERSION_NO_LONGER_SUPPORTED                0x00000182 ///< 协议版本不支持
#define CIVIL_RESULT_GENERAL_VERSION_FORBIDDEN                          0x00000183 ///< 协议版本被禁止

#define CIVIL_RESULT_GENERAL_SERIAL_NOT_FOR_CIVIL                       0x00100001 ///< 非民用设备序列号
#define CIVIL_RESULT_GENERAL_SERIAL_FORBIDDEN                           0x00100002 ///< 序列号被禁止
#define CIVIL_RESULT_GENERAL_SERIAL_DUPLICATE                           0x00100003 ///< 序列号重复
#define CIVIL_RESULT_GENERAL_SERIAL_FLUSHED_IN_A_SECOND                 0x00100004 ///< 相同序列号短时间大量重复请求
#define CIVIL_RESULT_GENERAL_SERIAL_NO_LONGER_SUPPORTED                 0x00100005 ///< 序列号不再支持

#define CIVIL_RESULT_UPGRADE_PU_REQUEST_REFUSED                         0x00100551 ///< 平台拒绝升级请求
#define CIVIL_RESULT_UPGRADE_PU_REQUEST_VERSION_NOT_FOUND               0x00100552 ///< 没有找到请求版本
#define CIVIL_RESULT_UPGRADE_PU_REQUEST_UNNEEDED                        0x00100553 ///< 设备不需要升级
#define CIVIL_RESULT_UPGRADE_PU_REQUEST_NO_SERVER_ONLINE                0x00100554 ///< 没有可提供升级的服务器
#define CIVIL_RESULT_UPGRADE_PU_REQUEST_ALL_SERVER_BUSY                 0x00100555 ///< 服务器繁忙

#define CIVIL_RESULT_UPGRADE_PU_UPGRADING                               0x00100561 ///< 设备正在升级
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_FAILED                           0x00100562 ///< 设备升级失败
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_WRITEFLASH_FAILED                0x00100563 ///< 更新本地FLASH失败
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_LANGUAGE_DISMATCH                0x00100564 ///< 升级包语言不匹配

#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_PLATFORM_DISMATCH                0x00100565 ///< 升级包的设备硬件平台不匹配
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_SPACE_DISMATCH                   0x00100566 ///< 升级包空间不匹配
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_MEM_DISMATCH                     0x00100567 ///< 升级包内存不匹配
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_MAJORTYPE_DISMATCH               0x00100568 ///< 升级包设备主类型不匹配
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_MINORTYPE_DISMATCH               0x00100569 ///< 升级包设备此类型不匹配
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_FILE_NUMS_INVALID                0x0010056A ///< 升级包文件数无效
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_PACK_LEN_INVALID                 0x0010056B ///< 升级包长度非法
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_CHECKSUM_ERR                     0x0010056C ///< 升级包校验失败
#define CIVIL_RESULT_UPGRADE_PU_UPGRADE_FRONT_FAIL                      0x0010056D ///< 升级前端数字摄像机失败
#define CIVIL_RESULT_UPGRADE_PU_NO_RESOURCE                             0x0010056E ///< 升级资源不足
#define CIVIL_RESULT_UPGRADE_PU_OPER_NOPERMIT                           0x0010056F ///< 没有升级权限
#define CIVIL_RESULT_UPGRADE_PU_REBOOTING                               0x00100570 ///< 升级成功，正在重启
#define CIVIL_RESULT_UPGRADE_PU_NO_MEMORY                               0x00100571 ///<
#define CIVIL_RESULT_UPGRADE_PU_PARAM_ERR                               0x00100572 ///< 升级参数错误
#define CIVIL_RESULT_UPGRADE_PU_HEAD_DATA_ERR                           0x00100573 ///< 升级包头部数据错误

#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FAILED                         0x00100600 ///< 设备下载升级包失败
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_PATH_ERR                       0x00100601 ///< 路径或文件名错误
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_PARAM_ERR                      0x00100602 ///< 下载参数错误
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_ESTCMD_ERR                 0x00100603 ///< FTP 建立命令出错
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_CMD_FAILED                 0x00100604 ///< FTP 执行命令失败
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_CONNINIT_FAILED            0x00100605 ///< FTP 连接初始化失败
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_TRANS_ABORT                0x00100606 ///< FTP 异常中断
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_SELECT_ERR                 0x00100607 ///< FTP select出错
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_GET_DATA_SOCK_ERR          0x00100608 ///< FTP 获取数据套接字出错
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_RECV_ERR                   0x00100609 ///< FTP 接收数据出错
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_BUFF_ERR                   0x0010060A ///< FTP 缓冲区出错
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FILE_CHECK_ERR                 0x0010060B ///< 下载文件校验错误
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_CONN_ERR                   0x0010060C ///< FTP 连接出错
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_LOGIN_FAILED               0x0010060D ///< FTP 登陆失败
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_GET_FILEINFO_FAILED        0x0010060E ///< FTP 获取文件信息失败

#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_HTTP_FAILED                    0x00100700 ///< 一般错误
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_HTTP_PATH_ERR                  0x00100701 ///< 文件路径错误
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_HTTP_CONN_ERR                  0x00100702 ///< 连接服务器失败
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_HTTP_BUFF_ERR                  0x00100703 ///< 设备文件缓冲区错误
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_HTTP_RECV_ERR                  0x00100704 ///< 接收数据出错

#define CIVIL_RESULT_PU_PASSWORD_UPDATE_NO_USER_MATHCED                 0x00100581 ///< USERID验证失败
#define CIVIL_RESULT_PU_PASSWORD_UPDATE_ORIGINAL_PASSWORD_ERROR         0x00100582 ///< 原音视频加密密码验证失败
#define CIVIL_RESULT_PU_PASSWORD_UPDATE_NEW_PASSWORD_DECRYPTE_FAILED    0x00100583 ///<
#define CIVIL_RESULT_PU_PASSWORD_UPDATE_NEW_PASSWORD_CHECK_FAILED       0x00100584 ///< 密码格式不合法
#define CIVIL_RESULT_PU_PASSWORD_UPDATE_WRITE_FLASH_FAILED              0x00100585 ///< 保存密码失败
#define CIVIL_RESULT_PU_PASSWORD_UPDATE_OTHER_FAILURE                   0x00100586 ///<
#define CIVIL_RESULT_PU_PASSWORD_VERIFY_FAILED                          0x00100591 ///< 验证密码失败

#define CIVIL_RESULT_PU_CHALLENGE_CODE_VERIFY_FAILED                    0x00100601 ///< 验证挑战码失败

#define CIVIL_RESULT_WIFI_PU_NOT_SUPPORT                                0x00100801 ///< 设备不支持WIFI
#define CIVIL_RESULT_WIFI_PU_SEARCH_AP_FAIL                             0x00100802 ///< 设备搜索WIFI列表失败

#define CIVIL_RESULT_GENERAL_DEV_TYPE_INVAILED                          0x00100231 ///< 设备类型错误
#define CIVIL_RESULT_GENERAL_DEV_TYPE_NO_LONGGER_SUPPORTED              0x00100232 ///< 不再支持该类型设备

#define CIVIL_RESULT_PLATFORM_CLIENT_REQUEST_NO_PU_FOUNDED              0x00100E01 ///< 请求的设备不在线
#define CIVIL_RESULT_PLATFORM_CLIENT_REQUEST_REFUSED_TO_PROTECT_PU      0x00100E02 ///< 为了保护设备，拒绝请求
#define CIVIL_RESULT_PLATFORM_CLIENT_REQUEST_PU_LIMIT_REACHED           0x00100E03 ///< 设备达到连接的客户端上线
#define CIVIL_RESULT_PLATFORM_CLIENT_TEARDOWN_PU_CONNECTION             0x00100E04 ///< 要求客户端断开于设备的连接
#define CIVIL_RESULT_PU_REFUSE_CLIENT_CONNECTION                        0x00100E05 ///< 设备拒绝客户端连接
#define CIVIL_RESULT_PU_PLATFORM_CLIENT_REQUEST_PU_PRIVACY_ENABLE       0x00100E07 ///< 设备启用了隐私状态

#define CIVIL_RESULE_GENERAL_PU_BUSY                                    0x00101001 ///< 设备无法响应
#define CIVIL_RESULT_GENERAL_OPERATION_FAILED                           0x00101002 ///< 操作码错误
#define CIVIL_RESULT_PU_NO_CRYPTO_FOUND                                 0x00101003 ///< 设备或平台未找到加密算法
#define CIVIL_RESULT_GENERAL_PU_REFUSED                                 0x00101004 ///< 设备拒绝操作
#define CIVIL_RESULT_GENERAL_PU_NO_RESOURCE                             0x00101005 ///< 设备资源紧张
#define CIVIL_RESULT_GENERAL_PU_CHANNEL_ERROR                           0x00101006 ///< 设备通道错
#define CIVIL_RESULT_SYSTEM_COMMAND_PU_COMMAND_UNSUPPORTED              0x00101007 ///< 不支持的命令
#define CIVIL_RESULT_SYSTEM_COMMAND_PU_NO_RIGHTS_TO_DO_COMMAND          0x00101008 ///< 设备没有权限执行命令
#define CIVIL_RESULT_GENERAL_NO_SESSION_FOUND                           0x00101009 ///< 没有找到会话
#define CIVIL_RESULT_GENERAL_PU_NO_VALID_PRELINK                        0x0010100A ///< 没有可用的P2P预链接资源
#define CIVIL_RESULT_GENERAL_PU_NO_INNER_RESOURCE                       0x0010100B ///< 没有可用的直连资源(也就没有P2P资源)
#define CIVIL_RESULT_GENERAL_PU_NO_P2P_RESOURCE                         0x0010100C ///< 没有可用的P2P资源

#define CIVIL_RESULT_PREVIEW_CHANNEL_BUSY                               0x00101101 ///< 通道正在预览
#define CIVIL_RESULT_PREVIEW_CLIENT_BUSY                                0x00101102 ///< 取流地址重复
#define CIVIL_RESULT_PREVIEW_STREAM_UNSUPPORTED                         0x00101103 ///< 不支持的码流方式
#define CIVIL_RESULT_PREVIEW_TRANSPORT_UNSUPPORTED                      0x00101104 ///< 不支持的传输方式
#define CIVIL_RESULT_PREVIEW_CONNECT_SERVER_FAIL                        0x00101105 ///< 设备连接预览流媒体服务器失败
#define CIVIL_RESULT_PREVIEW_QUERY_WLAN_INFO_FAIL                       0x00101106 ///< 设备查询外网出口地址失败
#define CIVIL_RESULT_PREVIEW_NO_VIDEO_FAIL                              0x00101107 ///< 无视频源
#define CIVIL_RESULT_PREVIEW_SET_ENCODE_PARAM_FAIL                      0x00101108 ///< 设置编码参数失败
#define CIVIL_RESULT_PREVIEW_SET_PACK_TYPE_FAIL                         0x00101109 ///< 设置码流封装类型失败
#define CIVIL_RESULT_GENERAL_SESSION_FREED	                            0x0010101A ///< 回话已经释放20161117(多次停止出现的设备)
#define CIVIL_RESULT_PREVIEW_P2P_NOT_FOUND                              0x0010110D ///< 预连接未建立成功
#define CIVIL_RESULT_PREVIEW_SEND_STREAM_HEADER_FAIL                    0x00101110 ///< 向流媒体发送流头失败
#define CIVIL_RESULT_PREVIEW_CHANNEL_NOT_RELATED                        0X00101111 ///< 通道未关联
#define CIVIL_RESULT_PREVIEW_CHANNEL_NOT_ONLINE                         0X00101112 ///< 通道不在线
#define CIVIL_RESULT_PREVIEW_NOT_SUPPORT_ENCRYPT                        0X00101113 ///< 不支持加密预览

#define CIVIL_RESULT_RECORD_SEARCH_START_TIME_ERROR                     0x00101481 ///< 查找录像开始时间错误
#define CIVIL_RESULT_RECORD_SEARCH_STOP_TIME_ERROR                      0x00101482 ///< 查找录像结束时间错误
#define CIVIL_RESULT_RECORD_SEARCH_FAIL                                 0x00101483 ///< 查找录像失败
#define CIVIL_RESULT_RECORD_SEARCH_LIST_ERROR                           0x00101484 ///< 录像播放列表数量错误

#define CIVIL_RESULT_PLAYBACK_TYPE_UNSUPPORTED                          0x00101301 ///< 不支持的回放类型
#define CIVIL_RESULT_PLAYBACK_NO_FILE_MATCHED                           0x00101302 ///< 没有找到文件
#define CIVIL_RESULT_PLAYBACK_START_TIME_ERROR                          0x00101303 ///< 回放的开始时间错误
#define CIVIL_RESULT_PLAYBACK_STOP_TIME_ERROR                           0x00101304 ///< 回放的结束时间错误
#define CIVIL_RESULT_PLAYBACK_NO_FILE_FOUND                             0x00101305 ///< 没有找到回放文件
#define CIVIL_RESULT_PLAYBACK_CONNECT_SERVER_FAIL                       0x00101306 ///< 设备连接回放流媒体服务器失败

#define CIVIL_RESULT_TALK_ENCODE_TYPE_UNSUPPORTED                       0x00101701 ///< 对讲编码类型不支持
#define CIVIL_RESULT_TALK_CHANNEL_BUSY                                  0x00101702 ///< 该通道正在对讲
#define CIVIL_RESULT_TALK_CLIENT_BUSY                                   0x00101703 ///< 和目的地址已有连接
#define CIVIL_RESULT_TALK_UNSUPPORTED                                   0x00101704 ///< 设备不支持对讲
#define CIVIL_RESULT_TALK_CHANNO_ERROR                                  0x00101705 ///< 对讲通道号错误
#define CIVIL_RESULT_TALK_CONNECT_SERVER_FAILED                         0x00101706 ///< 连接对讲流媒体服务器失败
#define CIVIL_RESULT_TALK_REFUSED                                       0x00101707 ///< 设备拒绝对讲
#define CIVIL_RESULT_TALK_CAPACITY_LIMITED                              0x00101708 ///< 设备资源受限，无法对讲

#define CIVIL_RESULT_FORMAT_NO_LOCAL_STORAGE                            0x00101801 ///< 没有本地存储
#define CIVIL_RESULT_FORMAT_FORMATING                                   0x00101802 ///< 正在格式化
#define CIVIL_RESULT_FORMAT_FAILED                                      0x00101803 ///< 格式化失败

#define CIVIL_RESULT_DEFENCE_TYPE_UNSUPPORTED                           0x00101901 ///< 不支持布撤防功能

#define CIVIL_RESULT_CLOUD_NOT_FOUND                                    0x00101C01 ///< 没有找到云存储服务器
#define CIVIL_RESULT_CLOUD_PU_NO_USER                                   0x00101C02 ///< 该设备没有对应的云存储用户
#define CIVIL_RESULT_CLOUD_DBA_TIMEOUT                                  0x00101C03 ///< 查询用户信息超时
#define CIVIL_RESULT_CLOUD_PU_SHOULD_ENABLE_CLOUD_STORAGE               0x00101C04 ///< 设备不启用云存储
#define CIVIL_RESULT_CLOUD_FILE_TAIL_REACHED                            0x00101C05 ///< 文件已到结尾
#define CIVIL_RESULT_CLOUD_INVALID_SESSION                              0x00101C06 ///< 无效的session
#define CIVIL_RESULT_CLOUD_INVALID_HANDLE                               0x00101C07 ///< 无效的文件句柄
#define CIVIL_RESULT_CLOUD_UNKNOWN_CLOUD                                0x00101C08 ///< 未知的云存储类型
#define CIVIL_RESULT_CLOUD_UNSUPPORT_FILETYPE                           0x00101C09 ///< 不支持的文件类型
#define CIVIL_RESULT_CLOUD_INVALID_FILE                                 0x00101C0a ///< 无效的文件
#define CIVIL_RESULT_CLOUD_QUOTA_IS_FULL                                0x00101C0b ///< 配额已满
#define CIVIL_RESULT_CLOUD_FILE_IS_FULL                                 0x00101C0c ///< 文件已满

#define CIVIL_RESULT_CAPTURE_PIC_LOCAL_FAILED                           0x00101D00 ///< 设备本地抓图失败
#define CIVIL_RESULT_CAPTURE_PIC_APPLY_CACHE_FAILED                     0x00101D01 ///< 图片缓存申请失败
#define CIVIL_RESULT_CAPTURE_PIC_PARSE_PMS_DOMAIN_FAILED                0x00101D02 ///< PMS 域名解析错误
#define CIVIL_RESULT_CAPTURE_PIC_CONNECT_PMS_FAILED                     0x00101D03 ///< PMS 连接失败
#define CIVIL_RESULT_CAPTURE_PIC_CREATE_PMS_PACKET_FAILED               0x00101D04 ///< 创建PMS 报文错误
#define CIVIL_RESULT_CAPTURE_PIC_SEND_PMS_FAILED                        0x00101D05 ///< PMS 发送数据错误
#define CIVIL_RESULT_CAPTURE_PIC_RECV_PMS_FAILED                        0x00101D06 ///< PMS 接收数据错误
#define CIVIL_RESULT_CAPTURE_PIC_PARSE_PMS_RESPONSE_FAILED              0x00101D07 ///< PMS 应答报文解析错误
#define CIVIL_RESULT_CAPTURE_PIC_GET_URL_FAILED                         0x00101D08 ///< 获取URL失败

#define CIVIL_RESULT_REG_CANNOT_AFFORD_PU                               0x00102003 ///< 服务器无法接受设备请求
#define CIVIL_RESULT_REG_CRYPTO_UNMATCHED                               0x00102004 ///< 设备加密算法不匹配

#define CIVIL_RESULT_LBS_SERVER_TYPE_ERROR                              0x00104001 ///< 请求的服务器类型错误
#define CIVIL_RESULT_LBS_SERVER_TYPE_NO_LONGGER_SUPPORTED               0x00104002 ///< 不支持请求的服务器类型
#define CIVIL_RESULT_LBS_NO_REQUEST_SERVER_ONLINE                       0x00104003 ///< 没有可用的服务器
#define CIVIL_RESULT_LBS_NO_AVAILABLE_REQUEST_SERVER                    0x00104004 ///< 没有可以响应的服务器

#define CIVIL_RESULT_LBS_CERTIFICATION_ERROR                            0x00105001 ///< 设备的加密串描述失败
#define CIVIL_RESULT_REG_DEV_LOCAL_ADDRESS_INVAILED                     0x00105002 ///< 设备的本地IP地址非法
#define CIVIL_RESULT_REG_DEV_UPNP_ADDRESS_INVAILED                      0x00105003 ///< 设备的UPnP地址非法
#define CIVIL_RESULT_REG_PU_UNREIGSTER                                  0x00105004 ///< 设备尚未注册
#define CIVIL_RESULT_REG_PU_AUTHORIZATION_FAILED                        0x00105005 ///< 授权码错误
#define CIVIL_RESULT_REG_REFUSE_PU_OFFLINE                              0x00105006 ///< 服务器拒绝设备下线请求

#define CIVIL_RESULT_PTZ_CONTROL_CALLING_PRESET_FAILED          0x00140000	///<正在调用预置点
#define CIVIL_RESULT_PTZ_CONTROL_SOUND_LACALIZATION_FAILED      0x00140001	///<当前正在声源定位     
#define CIVIL_RESULT_PTZ_CONTROL_CRUISE_TRACK_FAILED            0x00140002	///<当前正在轨迹巡航     
#define CIVIL_RESULT_PTZ_PRESET_INVALID_POSITION_FAILED         0x00140003	///<预置点无效           
#define CIVIL_RESULT_PTZ_PRESET_CURRENT_POSITION_FAILED         0x00140004	///<已在当前预置点       
#define CIVIL_RESULT_PTZ_RESPONSE_SOUND_LOCALIZATION_FAILED     0x00140005	///<正在响应声源定位     
#define CIVIL_RESULT_PTZ_PRESET_PRESETING_FAILED                0x00140006	///重复，不使用 
#define CIVIL_RESULT_PTZ_OPENING_PRIVACY_FAILED                 0x00140007	///<当前正在打开隐私遮蔽 
#define CIVIL_RESULT_PTZ_CLOSING_PRIVACY_FAILED                 0x00140008	///<当前正在关闭隐私遮蔽 
#define CIVIL_RESULT_PTZ_FAILED                                 0x00140009	///当前操作失败    
#define CIVIL_RESULT_PTZ_PRESET_EXCEED_MAXNUM_FAILED            0x0014000A	///<预置点超过最大数     
#define CIVIL_RESULT_PTZ_PRIVACYING_FAILED                      0X0014000B  ///<设备处于隐私模式     
#define CIVIL_RESULT_PTZ_MIRRORING_FAILED                       0X0014000C  ///<正在镜像操作         
#define CIVIL_RESULT_PTZ_CONTROLING_FAILED                      0X0014000D  ///<正在键控操作         
#define CIVIL_RESULT_PTZ_TTSING_FAILED                          0X0014000E  ///<正在语音对讲         
#define CIVIL_RESULT_PTZ_ROTATION_UP_LIMIT_FAILED               0X0014000F  ///<云台旋转到上限       
#define CIVIL_RESULT_PTZ_ROTATION_DOWN_LIMIT_FAILED             0X00140010  ///<云台旋转到下限       
#define CIVIL_RESULT_PTZ_ROTATION_LEFT_LIMIT_FAILED             0X00140011  ///<云台旋转到左限      
#define CIVIL_RESULT_PTZ_ROTATION_RIGHT_LIMIT_FAILED            0X00140012  ///<云台旋转到右限  
#define CIVIL_RESULT_PTZ_CRUISE_PRESET_ANGLE_ERROR              0X00140013  ///<巡航预置点角度间隔太小(小余15度)

#define CIVIL_RESULT_CUSTOMVOICE_DOWNLOAD_FAILED                0X00170000	//语音文件下载失败
#define CIVIL_RESULT_CUSTOMVOICE_NOT_EXIST                      0X00170001	//语音文件不存在
#define CIVIL_RESULT_CUSTOMVOICE_IS_BUSY                        0X00170002 	//语音文件正在使用中
#define CIVIL_RESULT_CUSTOMVOICE_COUNT_LIMITED                  0X00170003	//语音文件个数达到上限


#define CIVIL_RESULT_DOORLOCK_VERIFY_NOT_TRIGGER                0x00180001  // 门锁 - 未触发身份认证
#define CIVIL_RESULT_DOORLOCK_VERIFY_PROCESSING                 0x00180002  // 门锁 - 身份认证中
#define CIVIL_RESULT_DOORLOCK_VERIFY_BY_OTHER_PEOPLE            0x00180003  // 门锁 - 其它用户处于身份认证中
#define CIVIL_RESULT_DOORLOCK_VERIFY_FAILED                     0x00180004  // 门锁 - 身份认证失败
#define CIVIL_RESULT_DOORLOCK_VERIFY_NO_MAIN_USR                0x00180006  // 门锁 - 身份认证没有主用户
#define CIVIL_RESULT_DOORLOCK_NOT_BACK_ALARM_FULL               0x00181001  // 未回家提醒 - 计划已达上限
#define CIVIL_RESULT_DOORLOCK_NOT_BACK_ALARM_NOT_EXIST          0x00181002  // 未回家提醒 - 计划不存在
#define CIVIL_RESULT_DOORLOCK_NOT_BACK_ALARM_EXIST              0x00181003  // 未回家提醒 - 锁用户计划已存在
#define CIVIL_RESULT_DOORLOCK_USER_FULL                         0x00182001  // 锁用户 - 用户已达上限
#define CIVIL_RESULT_DOORLOCK_USER_NOT_EXIST                    0x00182002  // 锁用户 - 用户不存在
#define CIVIL_RESULT_DOORLOCK_USER_CONTENT_EXIST                0x00182003  // 锁用户 - 开门方式已存在
#define CIVIL_RESULT_DOORLOCK_USER_CONTENT_FULL                 0x00182004  // 锁用户 - 开门方式已满
#define CIVIL_RESULT_DOORLOCK_USER_CONTENT_NOT_EXIST            0x00182005  // 锁用户 - 开门方式不存在
#define CIVIL_RESULT_DOORLOCK_USER_EXIST                        0x00182006  // 锁用户 - 用户已存在
#define CIVIL_RESULT_DOORLOCK_USER_DELETE_PROHIBITED            0x00182007  // 锁用户 - 该用户禁止删除
#define CIVIL_RESULT_DOORLOCK_USER_AUTH_DELETE_PROHIBITED       0x00182008  // 锁用户 - 该用户最后一个认证方式禁止删除
#define CIVIL_RESULT_DOORLOCK_SNAP_PASSWORD_FULL                0x00183001  // 临时密码 - 临时密码已达上限
#define CIVIL_RESULT_DOORLOCK_SNAP_PASSWORD_NOT_EXIST           0x00183002  // 临时密码 - 临时密码不存在
#define CIVIL_RESULT_DOORLOCK_SNAP_PASSWORD_EXIST               0x00183003  // 临时密码 - 临时密码已存在


#define RECV_CMD_ERROR    0xAAA
#define SEND_CMD_ERROR    0xBBB
#define PARSE_BODY_ERROR  0xFFF

#endif // _ERROR_H_
