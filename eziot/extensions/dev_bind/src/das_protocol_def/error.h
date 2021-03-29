

#ifndef __PROTOCOL_ERROR_H__
#define __PROTOCOL_ERROR_H__

#define CIVIL_RESULT_GENERAL_NO_ERROR                                   0x00000000 
#define CIVIL_RESULT_GENERAL_UNKNOW_ERROR                               0x00000001 ///< δ֪����
#define CIVIL_RESULT_GENERAL_PARAMS_ERROR                               0x00000002 ///< ��������
#define CIVIL_RESULT_GENERAL_PARSE_FAILED                               0x00000003 ///< ���Ľ���ʧ��
#define CIVIL_RESULT_GENERAL_SYSTEM_ERROR                               0x00000004 ///< ϵͳ�ڲ�����
#define CIVIL_RESULT_GENERAL_DEVICE_NOT_EXIST                           0x00100009 ///< �豸������


#define CIVIL_RESULT_GENERAL_COMMAND_UNKNOW                             0x00000081 ///< �Ƿ�����
#define CIVIL_RESULT_GENERAL_COMMAND_NO_LONGER_SUPPORTED                0x00000082 ///< ��ʱ����
#define CIVIL_RESULT_GENERAL_COMMAND_NOT_SUITABLE                       0x00000083 ///< �����ʽ����ȷ
#define CIVIL_RESULT_GENERAL_COMMAND_NOT_ALLOW                          0x00000084 ///< δ��Ȩ����


#define CIVIL_RESULT_GENERAL_CHECKSUM_ERROR                             0x00000101 ///< ����У��ʹ���
#define CIVIL_RESULT_GENERAL_HEADER_INVALID                             0x00000102	// ��Ϣͷ�Ƿ�
#define CIVIL_RESULT_GENERAL_LENGTH_INVALID                             0x00000103	// ��Ϣͷ�Ƿ�

#define CIVIL_RESULT_GENERAL_VERSION_UNKNOW                             0x00000181 ///< Э��汾����
#define CIVIL_RESULT_GENERAL_VERSION_NO_LONGER_SUPPORTED                0x00000182 ///< Э��汾��֧��
#define CIVIL_RESULT_GENERAL_VERSION_FORBIDDEN                          0x00000183 ///< Э��汾����ֹ

#define CIVIL_RESULT_GENERAL_SERIAL_NOT_FOR_CIVIL                       0x00100001 ///< �������豸���к�
#define CIVIL_RESULT_GENERAL_SERIAL_FORBIDDEN                           0x00100002 ///< ���кű���ֹ
#define CIVIL_RESULT_GENERAL_SERIAL_DUPLICATE                           0x00100003 ///< ���к��ظ�
#define CIVIL_RESULT_GENERAL_SERIAL_FLUSHED_IN_A_SECOND                 0x00100004 ///< ��ͬ���кŶ�ʱ������ظ�����
#define CIVIL_RESULT_GENERAL_SERIAL_NO_LONGER_SUPPORTED                 0x00100005 ///< ���кŲ���֧��

#define CIVIL_RESULT_UPGRADE_PU_REQUEST_REFUSED                         0x00100551 ///< ƽ̨�ܾ���������
#define CIVIL_RESULT_UPGRADE_PU_REQUEST_VERSION_NOT_FOUND               0x00100552 ///< û���ҵ�����汾
#define CIVIL_RESULT_UPGRADE_PU_REQUEST_UNNEEDED                        0x00100553 ///< �豸����Ҫ����
#define CIVIL_RESULT_UPGRADE_PU_REQUEST_NO_SERVER_ONLINE                0x00100554 ///< û�п��ṩ�����ķ�����
#define CIVIL_RESULT_UPGRADE_PU_REQUEST_ALL_SERVER_BUSY                 0x00100555 ///< ��������æ

#define CIVIL_RESULT_UPGRADE_PU_UPGRADING                               0x00100561 ///< �豸��������
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_FAILED                           0x00100562 ///< �豸����ʧ��
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_WRITEFLASH_FAILED                0x00100563 ///< ���±���FLASHʧ��
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_LANGUAGE_DISMATCH                0x00100564 ///< ���������Բ�ƥ��

#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_PLATFORM_DISMATCH                0x00100565 ///< ���������豸Ӳ��ƽ̨��ƥ��
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_SPACE_DISMATCH                   0x00100566 ///< �������ռ䲻ƥ��
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_MEM_DISMATCH                     0x00100567 ///< �������ڴ治ƥ��
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_MAJORTYPE_DISMATCH               0x00100568 ///< �������豸�����Ͳ�ƥ��
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_MINORTYPE_DISMATCH               0x00100569 ///< �������豸�����Ͳ�ƥ��
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_FILE_NUMS_INVALID                0x0010056A ///< �������ļ�����Ч
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_PACK_LEN_INVALID                 0x0010056B ///< ���������ȷǷ�
#define CIVIL_RESULT_UPGRADE_PU_UPGRAD_CHECKSUM_ERR                     0x0010056C ///< ������У��ʧ��
#define CIVIL_RESULT_UPGRADE_PU_UPGRADE_FRONT_FAIL                      0x0010056D ///< ����ǰ�����������ʧ��
#define CIVIL_RESULT_UPGRADE_PU_NO_RESOURCE                             0x0010056E ///< ������Դ����
#define CIVIL_RESULT_UPGRADE_PU_OPER_NOPERMIT                           0x0010056F ///< û������Ȩ��
#define CIVIL_RESULT_UPGRADE_PU_REBOOTING                               0x00100570 ///< �����ɹ�����������
#define CIVIL_RESULT_UPGRADE_PU_NO_MEMORY                               0x00100571 ///<
#define CIVIL_RESULT_UPGRADE_PU_PARAM_ERR                               0x00100572 ///< ������������
#define CIVIL_RESULT_UPGRADE_PU_HEAD_DATA_ERR                           0x00100573 ///< ������ͷ�����ݴ���

#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FAILED                         0x00100600 ///< �豸����������ʧ��
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_PATH_ERR                       0x00100601 ///< ·�����ļ�������
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_PARAM_ERR                      0x00100602 ///< ���ز�������
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_ESTCMD_ERR                 0x00100603 ///< FTP �����������
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_CMD_FAILED                 0x00100604 ///< FTP ִ������ʧ��
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_CONNINIT_FAILED            0x00100605 ///< FTP ���ӳ�ʼ��ʧ��
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_TRANS_ABORT                0x00100606 ///< FTP �쳣�ж�
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_SELECT_ERR                 0x00100607 ///< FTP select����
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_GET_DATA_SOCK_ERR          0x00100608 ///< FTP ��ȡ�����׽��ֳ���
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_RECV_ERR                   0x00100609 ///< FTP �������ݳ���
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_BUFF_ERR                   0x0010060A ///< FTP ����������
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FILE_CHECK_ERR                 0x0010060B ///< �����ļ�У�����
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_CONN_ERR                   0x0010060C ///< FTP ���ӳ���
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_LOGIN_FAILED               0x0010060D ///< FTP ��½ʧ��
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_FTP_GET_FILEINFO_FAILED        0x0010060E ///< FTP ��ȡ�ļ���Ϣʧ��

#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_HTTP_FAILED                    0x00100700 ///< һ�����
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_HTTP_PATH_ERR                  0x00100701 ///< �ļ�·������
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_HTTP_CONN_ERR                  0x00100702 ///< ���ӷ�����ʧ��
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_HTTP_BUFF_ERR                  0x00100703 ///< �豸�ļ�����������
#define CIVIL_RESULT_UPGRADE_PU_DOWNLOAD_HTTP_RECV_ERR                  0x00100704 ///< �������ݳ���

#define CIVIL_RESULT_PU_PASSWORD_UPDATE_NO_USER_MATHCED                 0x00100581 ///< USERID��֤ʧ��
#define CIVIL_RESULT_PU_PASSWORD_UPDATE_ORIGINAL_PASSWORD_ERROR         0x00100582 ///< ԭ����Ƶ����������֤ʧ��
#define CIVIL_RESULT_PU_PASSWORD_UPDATE_NEW_PASSWORD_DECRYPTE_FAILED    0x00100583 ///<
#define CIVIL_RESULT_PU_PASSWORD_UPDATE_NEW_PASSWORD_CHECK_FAILED       0x00100584 ///< �����ʽ���Ϸ�
#define CIVIL_RESULT_PU_PASSWORD_UPDATE_WRITE_FLASH_FAILED              0x00100585 ///< ��������ʧ��
#define CIVIL_RESULT_PU_PASSWORD_UPDATE_OTHER_FAILURE                   0x00100586 ///<
#define CIVIL_RESULT_PU_PASSWORD_VERIFY_FAILED                          0x00100591 ///< ��֤����ʧ��

#define CIVIL_RESULT_PU_CHALLENGE_CODE_VERIFY_FAILED                    0x00100601 ///< ��֤��ս��ʧ��

#define CIVIL_RESULT_WIFI_PU_NOT_SUPPORT                                0x00100801 ///< �豸��֧��WIFI
#define CIVIL_RESULT_WIFI_PU_SEARCH_AP_FAIL                             0x00100802 ///< �豸����WIFI�б�ʧ��

#define CIVIL_RESULT_GENERAL_DEV_TYPE_INVAILED                          0x00100231 ///< �豸���ʹ���
#define CIVIL_RESULT_GENERAL_DEV_TYPE_NO_LONGGER_SUPPORTED              0x00100232 ///< ����֧�ָ������豸

#define CIVIL_RESULT_PLATFORM_CLIENT_REQUEST_NO_PU_FOUNDED              0x00100E01 ///< ������豸������
#define CIVIL_RESULT_PLATFORM_CLIENT_REQUEST_REFUSED_TO_PROTECT_PU      0x00100E02 ///< Ϊ�˱����豸���ܾ�����
#define CIVIL_RESULT_PLATFORM_CLIENT_REQUEST_PU_LIMIT_REACHED           0x00100E03 ///< �豸�ﵽ���ӵĿͻ�������
#define CIVIL_RESULT_PLATFORM_CLIENT_TEARDOWN_PU_CONNECTION             0x00100E04 ///< Ҫ��ͻ��˶Ͽ����豸������
#define CIVIL_RESULT_PU_REFUSE_CLIENT_CONNECTION                        0x00100E05 ///< �豸�ܾ��ͻ�������
#define CIVIL_RESULT_PU_PLATFORM_CLIENT_REQUEST_PU_PRIVACY_ENABLE       0x00100E07 ///< �豸��������˽״̬

#define CIVIL_RESULE_GENERAL_PU_BUSY                                    0x00101001 ///< �豸�޷���Ӧ
#define CIVIL_RESULT_GENERAL_OPERATION_FAILED                           0x00101002 ///< ���������
#define CIVIL_RESULT_PU_NO_CRYPTO_FOUND                                 0x00101003 ///< �豸��ƽ̨δ�ҵ������㷨
#define CIVIL_RESULT_GENERAL_PU_REFUSED                                 0x00101004 ///< �豸�ܾ�����
#define CIVIL_RESULT_GENERAL_PU_NO_RESOURCE                             0x00101005 ///< �豸��Դ����
#define CIVIL_RESULT_GENERAL_PU_CHANNEL_ERROR                           0x00101006 ///< �豸ͨ����
#define CIVIL_RESULT_SYSTEM_COMMAND_PU_COMMAND_UNSUPPORTED              0x00101007 ///< ��֧�ֵ�����
#define CIVIL_RESULT_SYSTEM_COMMAND_PU_NO_RIGHTS_TO_DO_COMMAND          0x00101008 ///< �豸û��Ȩ��ִ������
#define CIVIL_RESULT_GENERAL_NO_SESSION_FOUND                           0x00101009 ///< û���ҵ��Ự
#define CIVIL_RESULT_GENERAL_PU_NO_VALID_PRELINK                        0x0010100A ///< û�п��õ�P2PԤ������Դ
#define CIVIL_RESULT_GENERAL_PU_NO_INNER_RESOURCE                       0x0010100B ///< û�п��õ�ֱ����Դ(Ҳ��û��P2P��Դ)
#define CIVIL_RESULT_GENERAL_PU_NO_P2P_RESOURCE                         0x0010100C ///< û�п��õ�P2P��Դ

#define CIVIL_RESULT_PREVIEW_CHANNEL_BUSY                               0x00101101 ///< ͨ������Ԥ��
#define CIVIL_RESULT_PREVIEW_CLIENT_BUSY                                0x00101102 ///< ȡ����ַ�ظ�
#define CIVIL_RESULT_PREVIEW_STREAM_UNSUPPORTED                         0x00101103 ///< ��֧�ֵ�������ʽ
#define CIVIL_RESULT_PREVIEW_TRANSPORT_UNSUPPORTED                      0x00101104 ///< ��֧�ֵĴ��䷽ʽ
#define CIVIL_RESULT_PREVIEW_CONNECT_SERVER_FAIL                        0x00101105 ///< �豸����Ԥ����ý�������ʧ��
#define CIVIL_RESULT_PREVIEW_QUERY_WLAN_INFO_FAIL                       0x00101106 ///< �豸��ѯ�������ڵ�ַʧ��
#define CIVIL_RESULT_PREVIEW_NO_VIDEO_FAIL                              0x00101107 ///< ����ƵԴ
#define CIVIL_RESULT_PREVIEW_SET_ENCODE_PARAM_FAIL                      0x00101108 ///< ���ñ������ʧ��
#define CIVIL_RESULT_PREVIEW_SET_PACK_TYPE_FAIL                         0x00101109 ///< ����������װ����ʧ��
#define CIVIL_RESULT_GENERAL_SESSION_FREED	                            0x0010101A ///< �ػ��Ѿ��ͷ�20161117(���ֹͣ���ֵ��豸)
#define CIVIL_RESULT_PREVIEW_P2P_NOT_FOUND                              0x0010110D ///< Ԥ����δ�����ɹ�
#define CIVIL_RESULT_PREVIEW_SEND_STREAM_HEADER_FAIL                    0x00101110 ///< ����ý�巢����ͷʧ��

#define CIVIL_RESULT_RECORD_SEARCH_START_TIME_ERROR                     0x00101481 ///< ����¼��ʼʱ�����
#define CIVIL_RESULT_RECORD_SEARCH_STOP_TIME_ERROR                      0x00101482 ///< ����¼�����ʱ�����
#define CIVIL_RESULT_RECORD_SEARCH_FAIL                                 0x00101483 ///< ����¼��ʧ��
#define CIVIL_RESULT_RECORD_SEARCH_LIST_ERROR                           0x00101484 ///< ¼�񲥷��б���������

#define CIVIL_RESULT_PLAYBACK_TYPE_UNSUPPORTED                          0x00101301 ///< ��֧�ֵĻط�����
#define CIVIL_RESULT_PLAYBACK_NO_FILE_MATCHED                           0x00101302 ///< û���ҵ��ļ�
#define CIVIL_RESULT_PLAYBACK_START_TIME_ERROR                          0x00101303 ///< �طŵĿ�ʼʱ�����
#define CIVIL_RESULT_PLAYBACK_STOP_TIME_ERROR                           0x00101304 ///< �طŵĽ���ʱ�����
#define CIVIL_RESULT_PLAYBACK_NO_FILE_FOUND                             0x00101305 ///< û���ҵ��ط��ļ�
#define CIVIL_RESULT_PLAYBACK_CONNECT_SERVER_FAIL                       0x00101306 ///< �豸���ӻط���ý�������ʧ��

#define CIVIL_RESULT_TALK_ENCODE_TYPE_UNSUPPORTED                       0x00101701 ///< �Խ��������Ͳ�֧��
#define CIVIL_RESULT_TALK_CHANNEL_BUSY                                  0x00101702 ///< ��ͨ�����ڶԽ�
#define CIVIL_RESULT_TALK_CLIENT_BUSY                                   0x00101703 ///< ��Ŀ�ĵ�ַ��������
#define CIVIL_RESULT_TALK_UNSUPPORTED                                   0x00101704 ///< �豸��֧�ֶԽ�
#define CIVIL_RESULT_TALK_CHANNO_ERROR                                  0x00101705 ///< �Խ�ͨ���Ŵ���
#define CIVIL_RESULT_TALK_CONNECT_SERVER_FAILED                         0x00101706 ///< ���ӶԽ���ý�������ʧ��
#define CIVIL_RESULT_TALK_REFUSED                                       0x00101707 ///< �豸�ܾ��Խ�
#define CIVIL_RESULT_TALK_CAPACITY_LIMITED                              0x00101708 ///< �豸��Դ���ޣ��޷��Խ�

#define CIVIL_RESULT_FORMAT_NO_LOCAL_STORAGE                            0x00101801 ///< û�б��ش洢
#define CIVIL_RESULT_FORMAT_FORMATING                                   0x00101802 ///< ���ڸ�ʽ��
#define CIVIL_RESULT_FORMAT_FAILED                                      0x00101803 ///< ��ʽ��ʧ��

#define CIVIL_RESULT_DEFENCE_TYPE_UNSUPPORTED                           0x00101901 ///< ��֧�ֲ���������

#define CIVIL_RESULT_CLOUD_NOT_FOUND                                    0x00101C01 ///< û���ҵ��ƴ洢������
#define CIVIL_RESULT_CLOUD_PU_NO_USER                                   0x00101C02 ///< ���豸û�ж�Ӧ���ƴ洢�û�
#define CIVIL_RESULT_CLOUD_DBA_TIMEOUT                                  0x00101C03 ///< ��ѯ�û���Ϣ��ʱ
#define CIVIL_RESULT_CLOUD_PU_SHOULD_ENABLE_CLOUD_STORAGE               0x00101C04 ///< �豸�������ƴ洢
#define CIVIL_RESULT_CLOUD_FILE_TAIL_REACHED                            0x00101C05 ///< �ļ��ѵ���β
#define CIVIL_RESULT_CLOUD_INVALID_SESSION                              0x00101C06 ///< ��Ч��session
#define CIVIL_RESULT_CLOUD_INVALID_HANDLE                               0x00101C07 ///< ��Ч���ļ����
#define CIVIL_RESULT_CLOUD_UNKNOWN_CLOUD                                0x00101C08 ///< δ֪���ƴ洢����
#define CIVIL_RESULT_CLOUD_UNSUPPORT_FILETYPE                           0x00101C09 ///< ��֧�ֵ��ļ�����
#define CIVIL_RESULT_CLOUD_INVALID_FILE                                 0x00101C0a ///< ��Ч���ļ�
#define CIVIL_RESULT_CLOUD_QUOTA_IS_FULL                                0x00101C0b ///< �������
#define CIVIL_RESULT_CLOUD_FILE_IS_FULL                                 0x00101C0c ///< �ļ�����

#define CIVIL_RESULT_CAPTURE_PIC_LOCAL_FAILED                           0x00101D00 ///< �豸����ץͼʧ��
#define CIVIL_RESULT_CAPTURE_PIC_APPLY_CACHE_FAILED                     0x00101D01 ///< ͼƬ��������ʧ��
#define CIVIL_RESULT_CAPTURE_PIC_PARSE_PMS_DOMAIN_FAILED                0x00101D02 ///< PMS ������������
#define CIVIL_RESULT_CAPTURE_PIC_CONNECT_PMS_FAILED                     0x00101D03 ///< PMS ����ʧ��
#define CIVIL_RESULT_CAPTURE_PIC_CREATE_PMS_PACKET_FAILED               0x00101D04 ///< ����PMS ���Ĵ���
#define CIVIL_RESULT_CAPTURE_PIC_SEND_PMS_FAILED                        0x00101D05 ///< PMS �������ݴ���
#define CIVIL_RESULT_CAPTURE_PIC_RECV_PMS_FAILED                        0x00101D06 ///< PMS �������ݴ���
#define CIVIL_RESULT_CAPTURE_PIC_PARSE_PMS_RESPONSE_FAILED              0x00101D07 ///< PMS Ӧ���Ľ�������
#define CIVIL_RESULT_CAPTURE_PIC_GET_URL_FAILED                         0x00101D08 ///< ��ȡURLʧ��

#define CIVIL_RESULT_REG_CANNOT_AFFORD_PU                               0x00102003 ///< �������޷������豸����
#define CIVIL_RESULT_REG_CRYPTO_UNMATCHED                               0x00102004 ///< �豸�����㷨��ƥ��

#define CIVIL_RESULT_LBS_SERVER_TYPE_ERROR                              0x00104001 ///< ����ķ��������ʹ���
#define CIVIL_RESULT_LBS_SERVER_TYPE_NO_LONGGER_SUPPORTED               0x00104002 ///< ��֧������ķ���������
#define CIVIL_RESULT_LBS_NO_REQUEST_SERVER_ONLINE                       0x00104003 ///< û�п��õķ�����
#define CIVIL_RESULT_LBS_NO_AVAILABLE_REQUEST_SERVER                    0x00104004 ///< û�п�����Ӧ�ķ�����

#define CIVIL_RESULT_LBS_CERTIFICATION_ERROR                            0x00105001 ///< �豸�ļ��ܴ�����ʧ��
#define CIVIL_RESULT_REG_DEV_LOCAL_ADDRESS_INVAILED                     0x00105002 ///< �豸�ı���IP��ַ�Ƿ�
#define CIVIL_RESULT_REG_DEV_UPNP_ADDRESS_INVAILED                      0x00105003 ///< �豸��UPnP��ַ�Ƿ�
#define CIVIL_RESULT_REG_PU_UNREIGSTER                                  0x00105004 ///< �豸��δע��
#define CIVIL_RESULT_REG_PU_AUTHORIZATION_FAILED                        0x00105005 ///< ��Ȩ�����
#define CIVIL_RESULT_REG_REFUSE_PU_OFFLINE                              0x00105006 ///< �������ܾ��豸��������

#define CIVIL_RESULT_PTZ_CONTROL_CALLING_PRESET_FAILED          0x00140000	///<���ڵ���Ԥ�õ�
#define CIVIL_RESULT_PTZ_CONTROL_SOUND_LACALIZATION_FAILED      0x00140001	///<��ǰ������Դ��λ     
#define CIVIL_RESULT_PTZ_CONTROL_CRUISE_TRACK_FAILED            0x00140002	///<��ǰ���ڹ켣Ѳ��     
#define CIVIL_RESULT_PTZ_PRESET_INVALID_POSITION_FAILED         0x00140003	///<Ԥ�õ���Ч           
#define CIVIL_RESULT_PTZ_PRESET_CURRENT_POSITION_FAILED         0x00140004	///<���ڵ�ǰԤ�õ�       
#define CIVIL_RESULT_PTZ_RESPONSE_SOUND_LOCALIZATION_FAILED     0x00140005	///<������Ӧ��Դ��λ     
#define CIVIL_RESULT_PTZ_PRESET_PRESETING_FAILED                0x00140006	///�ظ�����ʹ�� 
#define CIVIL_RESULT_PTZ_OPENING_PRIVACY_FAILED                 0x00140007	///<��ǰ���ڴ���˽�ڱ� 
#define CIVIL_RESULT_PTZ_CLOSING_PRIVACY_FAILED                 0x00140008	///<��ǰ���ڹر���˽�ڱ� 
#define CIVIL_RESULT_PTZ_FAILED                                 0x00140009	///��ǰ����ʧ��    
#define CIVIL_RESULT_PTZ_PRESET_EXCEED_MAXNUM_FAILED            0x0014000A	///<Ԥ�õ㳬�������     
#define CIVIL_RESULT_PTZ_PRIVACYING_FAILED                      0X0014000B  ///<�豸������˽ģʽ     
#define CIVIL_RESULT_PTZ_MIRRORING_FAILED                       0X0014000C  ///<���ھ������         
#define CIVIL_RESULT_PTZ_CONTROLING_FAILED                      0X0014000D  ///<���ڼ��ز���         
#define CIVIL_RESULT_PTZ_TTSING_FAILED                          0X0014000E  ///<���������Խ�         
#define CIVIL_RESULT_PTZ_ROTATION_UP_LIMIT_FAILED               0X0014000F  ///<��̨��ת������       
#define CIVIL_RESULT_PTZ_ROTATION_DOWN_LIMIT_FAILED             0X00140010  ///<��̨��ת������       
#define CIVIL_RESULT_PTZ_ROTATION_LEFT_LIMIT_FAILED             0X00140011  ///<��̨��ת������      
#define CIVIL_RESULT_PTZ_ROTATION_RIGHT_LIMIT_FAILED            0X00140012  ///<��̨��ת������  
#define CIVIL_RESULT_PTZ_CRUISE_PRESET_ANGLE_ERROR              0X00140013  ///<Ѳ��Ԥ�õ�Ƕȼ��̫С(С��15��)

#define CIVIL_RESULT_CUSTOMVOICE_DOWNLOAD_FAILED                        0X00170000	//�����ļ�����ʧ��
#define CIVIL_RESULT_CUSTOMVOICE_NOT_EXIST                              0X00170001	//�����ļ�������
#define CIVIL_RESULT_CUSTOMVOICE_IS_BUSY                                0X00170002 	//�����ļ�����ʹ����
#define CIVIL_RESULT_CUSTOMVOICE_COUNT_LIMITED                          0X00170003	//�����ļ������ﵽ����


#define CIVIL_RESULT_DOORLOCK_VERIFY_NOT_TRIGGER                0x00180001  // ���� - δ����������֤
#define CIVIL_RESULT_DOORLOCK_VERIFY_PROCESSING                 0x00180002  // ���� - ������֤��
#define CIVIL_RESULT_DOORLOCK_VERIFY_BY_OTHER_PEOPLE            0x00180003  // ���� - �����û�����������֤��
#define CIVIL_RESULT_DOORLOCK_VERIFY_FAILED                     0x00180004  // ���� - ������֤ʧ��
#define CIVIL_RESULT_DOORLOCK_VERIFY_NO_MAIN_USR                0x00180006  // ���� - ������֤û�����û�
#define CIVIL_RESULT_DOORLOCK_NOT_BACK_ALARM_FULL               0x00181001  // δ�ؼ����� - �ƻ��Ѵ�����
#define CIVIL_RESULT_DOORLOCK_NOT_BACK_ALARM_NOT_EXIST          0x00181002  // δ�ؼ����� - �ƻ�������
#define CIVIL_RESULT_DOORLOCK_NOT_BACK_ALARM_EXIST              0x00181003  // δ�ؼ����� - ���û��ƻ��Ѵ���
#define CIVIL_RESULT_DOORLOCK_USER_FULL                         0x00182001  // ���û� - �û��Ѵ�����
#define CIVIL_RESULT_DOORLOCK_USER_NOT_EXIST                    0x00182002  // ���û� - �û�������
#define CIVIL_RESULT_DOORLOCK_USER_CONTENT_EXIST                0x00182003  // ���û� - ���ŷ�ʽ�Ѵ���
#define CIVIL_RESULT_DOORLOCK_USER_CONTENT_FULL                 0x00182004  // ���û� - ���ŷ�ʽ����
#define CIVIL_RESULT_DOORLOCK_USER_CONTENT_NOT_EXIST            0x00182005  // ���û� - ���ŷ�ʽ������
#define CIVIL_RESULT_DOORLOCK_USER_EXIST                        0x00182006  // ���û� - �û��Ѵ���
#define CIVIL_RESULT_DOORLOCK_USER_DELETE_PROHIBITED            0x00182007  // ���û� - ���û���ֹɾ��
#define CIVIL_RESULT_DOORLOCK_SNAP_PASSWORD_FULL                0x00183001  // ��ʱ���� - ��ʱ�����Ѵ�����
#define CIVIL_RESULT_DOORLOCK_SNAP_PASSWORD_NOT_EXIST           0x00183002  // ��ʱ���� - ��ʱ���벻����
#define CIVIL_RESULT_DOORLOCK_SNAP_PASSWORD_EXIST               0x00183003  // ��ʱ���� - ��ʱ�����Ѵ���




////////////////////////// internal error /////////////////////////
#define RECV_CMD_ERROR    0xAAA
#define SEND_CMD_ERROR    0xBBB
#define PARSE_BODY_ERROR  0xFFF

#endif // __PROTOCOL_ERROR_H__
