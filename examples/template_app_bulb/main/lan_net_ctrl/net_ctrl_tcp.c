#include "ezos_libc.h"

#include "lwip/err.h" 
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "bulb_led_ctrl.h"

#include "ezlog.h"
#include "lanProto.h"
#include "bulb_business.h"

int listen_sock;
int g_exit_music;
TaskHandle_t g_pthreadTaskTcpServer = NULL;
 
#define MUSIC_PORT 8484

int user_recvn_witherr(
    int iSockFd,
    void *pBuf,
    int iBufCount,
    int nTimeOut,
    int *p_iSocket_Status)
{
    int iTmpLen = 0;
	int iRet = 0;
    int iRecvLen = 0;
	struct fd_set read_fd;
	struct timeval tmval = {nTimeOut / 1000, (nTimeOut % 1000) * 1000};

    if (NULL == pBuf || 0 == iBufCount || -1 == iSockFd)
    {
        return false;
    }

    if (HPR_INFINITE == nTimeOut)
    {
        /**
         * \note ...
         *  please do not use this nTimeOut==WAIT_FOREVER if possible.
         */
        while (1)
        {
            iTmpLen = recv(iSockFd, (char *)pBuf + iRecvLen, iBufCount - iRecvLen,0);
            if (iTmpLen > 0)
            {
                iRecvLen += iTmpLen;
                if (iRecvLen == iBufCount)
                {
                    if (p_iSocket_Status)
                    {
                        *p_iSocket_Status = HPR_SOCKET_STATUS_OK;
                    }
                    return iRecvLen;
                }
            }
            else if (iTmpLen == 0)
            {
                if (p_iSocket_Status)
                {
                    *p_iSocket_Status = HPR_SOCKET_STATUS_REMOTE_CLOSED;
                }
                return iRecvLen;
            }
            else
            {
                if (p_iSocket_Status)
                {
                    *p_iSocket_Status = HPR_SOCKET_STATUS_ERROR;
                }
                return iRecvLen;
            }
        }
    }

    while (1)
    {

		
		FD_ZERO(&read_fd);
		FD_SET(iSockFd, &read_fd);
		
		
		iRet = select(iSockFd + 1, &read_fd, NULL, NULL, &tmval);

        if (iRet > 0)
        {
            iTmpLen = recv(iSockFd, (char *)pBuf + iRecvLen, iBufCount - iRecvLen,0);
            if (iTmpLen > 0)
            {
                iRecvLen += iTmpLen;
                if (iRecvLen == iBufCount)
                {
                    if (p_iSocket_Status)
                    {
                        *p_iSocket_Status = HPR_SOCKET_STATUS_OK;
                    }
                    return iRecvLen;
                }
                else
				{
					ezlog_i(TAG_APP, "receive not enough data %d,need", iRecvLen,iBufCount);
				}
            }
            else if (iTmpLen == 0)
            {
                if (p_iSocket_Status)
                {
                    *p_iSocket_Status = HPR_SOCKET_STATUS_REMOTE_CLOSED;
                }
                return iRecvLen;
            }
            else
            {
                if (p_iSocket_Status)
                {
                    *p_iSocket_Status = HPR_SOCKET_STATUS_ERROR;
                }
                return iRecvLen;
            }
        }
        else if (iRet == 0)
        {
            if (p_iSocket_Status)
            {
                *p_iSocket_Status = HPR_SOCKET_STATUS_OVERTIME;
            }
            return iRecvLen;
        }
        else
        {
            if (p_iSocket_Status)
            {
                *p_iSocket_Status = HPR_SOCKET_STATUS_ERROR;
            }
            return iRecvLen;
        }
    }


    if (p_iSocket_Status)
    {
        *p_iSocket_Status = HPR_SOCKET_STATUS_ERROR;
    }

    return iRecvLen;
}



int ledctrl_netparam_convert(led_ctrl_t *       pstruLedCtrl)
{
	short   nbrightness=0;       
    short   nCctValue=0;    
    int  	iRgbValue=0; 
    short	 nUpDuration;
	short   nLowBrightness=0;  
    short    nSpeed=0;   
    short	 nDownDuration=0;  

	if(NULL == pstruLedCtrl)
	{
		ezlog_e(TAG_APP, "invalid ledctrl param \n");
	}


	nbrightness = ntohs(pstruLedCtrl->nbrightness);
	nCctValue = ntohs(pstruLedCtrl->nCctValue);
	iRgbValue = ntohl(pstruLedCtrl->iRgbValue);
	nLowBrightness = ntohs(pstruLedCtrl->nLowBrightness);
	nUpDuration = ntohs(pstruLedCtrl->nUpDuration);
	nSpeed = ntohs(pstruLedCtrl->nSpeed);
	nDownDuration = ntohs(pstruLedCtrl->nDownDuration);

	if(nbrightness<0 || nbrightness > 100)
	{
		ezlog_v(TAG_APP, "invalid nbrightness %d", nbrightness);
		nbrightness = 100;
	}

	if(nLowBrightness<0 || nLowBrightness > 100)  //lowbrightness 为0 ，则一个周期不会降低
	{
		ezlog_v(TAG_APP, "invalid nLowBrightness %d", nLowBrightness);
		nLowBrightness = 10;
	}
	
	if(nCctValue<2700 || nCctValue > 6500)
	{
		
		ezlog_v(TAG_APP, "invalid nCctValue %d", nCctValue);
		nCctValue = 3000;
	}

	if(iRgbValue< 0 || iRgbValue > 0xffffff)
	{
		
		ezlog_v(TAG_APP, "invalid iRgbValue %d", iRgbValue);
		iRgbValue = 0xff;
	}

	if(nUpDuration<10)
	{
		
		ezlog_v(TAG_APP, "invalid nUpDuration %d,the smallest is 10", nUpDuration);
		nUpDuration = 10;
	}
	
	if(nUpDuration > 1500 )   //音乐律动单次指令的变化过程时间最大1.5s，10ms 150次。
	{		
		ezlog_v(TAG_APP, "invalid nUpDuration %d,the bigest is 1500", nUpDuration);
		nUpDuration = 1500;
	}

	if(nSpeed<0 || nSpeed > 10000)
	{
		
		ezlog_v(TAG_APP, "invalid nSpeed %d", nSpeed);
		nSpeed = 0;
	}

	if(nDownDuration<10 )
	{
		
		ezlog_v(TAG_APP, "invalid nDownDuration %d", nDownDuration);
		nDownDuration = 10;
	}

	if(nDownDuration > 1500)
	{
		
		ezlog_v(TAG_APP, "invalid nDownDuration %d,the bigest is 1500", nDownDuration);
		nDownDuration = 1500;
	}


	pstruLedCtrl->nbrightness = nbrightness;
	pstruLedCtrl->nCctValue = nCctValue;
	pstruLedCtrl->iRgbValue = iRgbValue;
	pstruLedCtrl->nLowBrightness = nLowBrightness;
	pstruLedCtrl->nUpDuration = nUpDuration;
	pstruLedCtrl->nSpeed = nSpeed;
	pstruLedCtrl->nDownDuration = nDownDuration;

	ezlog_v(TAG_APP, "b=%hd,cct=%hd,rgbvalue=0x%x,lowb=%hd,updur=%hd,speed=%d,downdura=%d"
	, pstruLedCtrl->nbrightness,pstruLedCtrl->nCctValue,pstruLedCtrl->iRgbValue,pstruLedCtrl->nLowBrightness,
	pstruLedCtrl->nUpDuration,pstruLedCtrl->nSpeed,pstruLedCtrl->nDownDuration);
	
	return 0;
	
}

/*
	struHeader 中包含request 中的一些sequence，版本信息，复用收到的数据
*/
int build_register_response(char *tx_buffer,LAN_HEADER_T struHeader,unsigned char value)
{
	//LAN_HEADER_T struHeader={0};
	//response  result 返回，用结构体，有对齐问题
	unsigned char keyType = 0;
	unsigned short valueLength = 0;

	int iSendLen = 0;

	struHeader.type = LAN_TYPE_REGISTER_RESPONSE;
	struHeader.len = 3 + sizeof(unsigned char);     //KLV消息体长度  response resutl只有1个字节
	struHeader.len = htons(struHeader.len);
	
	ezos_memcpy(tx_buffer, &struHeader, sizeof(struHeader));
	
	keyType = LAN_KEY_RESULT;

	valueLength = sizeof(value);

	valueLength = htons(valueLength);

	iSendLen = sizeof(struHeader);
	ezos_memcpy(tx_buffer + iSendLen,&keyType,1);
	iSendLen +=1;
	ezos_memcpy(tx_buffer + iSendLen,&valueLength,sizeof(valueLength));
	iSendLen += sizeof(valueLength);
	ezos_memcpy(tx_buffer + iSendLen,&value,sizeof(value));
	iSendLen += sizeof(value);
	return iSendLen;

}


void tcp_server_task(void *pvParameters)
{
    char rx_buffer[128];
    char tx_buffer[128];
    char addr_str[32];
    int addr_family;
    int ip_protocol;
	int sock =0;
	int iSocket_Status=0;
	int len=0; 
	int iSendLen = 0;
	int reuse = 1;

	int iSoketOptTmp = 0;

	LAN_HEADER_T struHeader={0};
	led_ctrl_t struLedCtrl={0};
	

	unsigned char flag =0;

    short dataLen = 0;

	ezlog_i(TAG_APP, "enter %s", __func__);
    
    struct sockaddr_in destAddr;
    destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(MUSIC_PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;
    inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1); 

    listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ezlog_i(TAG_APP, "Unable to create socket: errno %d", errno);
		vTaskDelete(NULL); 
        return ;
    }
    ezlog_i(TAG_APP, "Socket created");

	if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		perror("setsockopet error\n");
		vTaskDelete(NULL); 
		return ;
	}


    int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
    if (err != 0) {
        ezlog_i(TAG_APP, "Socket unable to bind: errno %d", errno);
		vTaskDelete(NULL); 
        return ;
    }
    ezlog_i(TAG_APP, "Socket binded");

	
	
    err = listen(listen_sock, 1);
    if (err != 0) {
        ezlog_i(TAG_APP, "Error occured during listen: errno %d", errno);
		vTaskDelete(NULL); 
        return ;
    }
    ezlog_i(TAG_APP, "Socket listening");

#ifdef CONFIG_EXAMPLE_IPV6
    struct sockaddr_in6 sourceAddr; // Large enough for both IPv4 or IPv6
#else
    struct sockaddr_in sourceAddr;
#endif
    uint addrLen = sizeof(sourceAddr);
    while (1) 
    {
		ezlog_i(TAG_APP, "waiting for a Socket connect");
        sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);
        if (sock < 0) 
        {
	        ezlog_i(TAG_APP, "Unable to accept connection: errno %d", errno);
	        continue;
    	}
        ezlog_i(TAG_APP, "Socket accepted");

		/* 音乐律动连接进入，关闭场景线程*/
		//exit_light_scene();

		/* lwtcp 协议栈不是很准确，测试结果如下，暂且用1,1,1组合，10s检测机制，
		电脑客户端在切换网络在切回来时，无法再连接
		60,60,3 =87s
		120,60,5 =105s
		2,2,1=12s
		1,1,1=10s
		1,10,3=40s
		60,10,3=40s
		*/
		iSoketOptTmp = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &iSoketOptTmp, sizeof(iSoketOptTmp)) == -1)
		{
			ezlog_i(TAG_APP, "Unable to set keepalive: errno %d", errno);
	        break;
		}

		iSoketOptTmp = 1;
		if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &iSoketOptTmp, sizeof(iSoketOptTmp)) < 0) {
			ezlog_i(TAG_APP, "Unable to set keepalive retry time: errno %d", errno);
	        break;
		}
		
		iSoketOptTmp = 1;
		if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &iSoketOptTmp, sizeof(iSoketOptTmp)) < 0) {
			ezlog_i(TAG_APP, "Unable to set keepalive retry time: errno %d", errno);
	        break;
		}

        while (!g_exit_music)
        {			     

			ezos_memset(&struHeader,0,sizeof(struHeader));
			ezos_memset(tx_buffer,0,sizeof(tx_buffer));

			/* APP 在音乐律动开启的情况下，在首页可以关灯，但app 无法主动关掉音乐律动，
			   这里设备做一个限制，让APP 被动关掉音乐律动*/
			if (false == get_light_switch())
			{
				ezlog_i(TAG_APP, "bulb is turned off ,music is not support");
				break;
			}
			ezlog_v(TAG_APP, "first need receive struheader=%d",sizeof(struHeader));
            len = user_recvn_witherr(sock, &struHeader, sizeof(struHeader), 200, &iSocket_Status);
            // Error occured during receiving
            if (len < 0) {
                ezlog_i(TAG_APP, "recv failed: errno %d", errno);
                break;
            }
            
            else if (len == 0) 
            {
				if(HPR_SOCKET_STATUS_REMOTE_CLOSED == iSocket_Status)
				{
                	ezlog_w(TAG_APP, "Connection closed");
                	break;
				}
				else if(HPR_SOCKET_STATUS_OVERTIME == iSocket_Status)
				{
					ezlog_v(TAG_APP, "recv date timeout"); 
					continue;
				}
				else
				{
					ezlog_e(TAG_APP, "recv date error %d",iSocket_Status); 
					break;
				}
				
                
            }
            // Data received
            else 
            {
                inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
	
				flag = struHeader.flag;

				dataLen = ntohs(struHeader.len);

				if ('$' != flag)
			    {
			        ezlog_i(TAG_APP,"Invalid protocol. flag:%c", flag);
			        break;
			    }
			    
			    if(dataLen > 127)
			    {
			    	ezlog_i(TAG_APP," cmdtype %d. datalenght=%d is too long ", struHeader.type,dataLen);
			        break;
            	}
				if(0 == dataLen)  //没有klv 的情况下，数据长度为0，包含心跳以及不带信息的register
				{
					ezlog_v(TAG_APP," cmdtype %d. datalenght=%d is zero", struHeader.type,dataLen);
			    	if(LAN_TYPE_REGISTER_REQUEST == struHeader.type)
					{
						
						iSendLen = build_register_response(tx_buffer,struHeader,LAN_KEY_RESULT_FAIL);
				
						iSendLen = send(sock, tx_buffer, iSendLen, 0);
						if(iSendLen < 0)
						{
							ezlog_e(TAG_APP," Error occured during sending: errno %d", errno);
							break;
						}							
					}
					else if(LAN_TYPE_HEARBEAT == struHeader.type)
					{
						iSendLen = send(sock, &struHeader, len, 0);
						if(iSendLen < 0)
						{
							ezlog_e(TAG_APP," Error occured during sending: errno %d", errno);
							break;
						}						
					}
					else
					{
						ezlog_e(TAG_APP," cmdtype %d. datalenght=%d is zero,something wrong",struHeader.type,dataLen);
						break;

					}
				}
				else
				{            	
	            	ezos_memset(rx_buffer,0,sizeof(rx_buffer));
					len = user_recvn_witherr(sock, &rx_buffer, dataLen, 200, &iSocket_Status);
					ezlog_v(TAG_APP," cmdtype %d. datalenght=%d,%d", struHeader.type,dataLen,struHeader.len);
					if (len < 0) 
					{
					   ezlog_i(TAG_APP, "recv failed: errno %d", errno);
					   break;
				   	}
					// Connection closed
					else if (len == 0) 
					{
					    if(HPR_SOCKET_STATUS_REMOTE_CLOSED == iSocket_Status)
						{
		                	ezlog_w(TAG_APP, "Connection closed");
						}
						else if(HPR_SOCKET_STATUS_OVERTIME == iSocket_Status)
						{
							ezlog_w(TAG_APP, "recv dateLen=%d timeout",dataLen);
						}
					    break;
					}
					// Data received
					else 
					{

						rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
						ezlog_v(TAG_APP, "Received %d bytes from %s:", len, addr_str);
						
						//first KLV 
						ezlog_v(TAG_APP, "%d,%d,%d,%d", rx_buffer[0],rx_buffer[1],rx_buffer[2],rx_buffer[3]);   
						
						if(LAN_TYPE_REGISTER_REQUEST == struHeader.type)
						{
														
							iSendLen = build_register_response(tx_buffer,struHeader,LAN_KEY_RESULT_SUC);
							iSendLen = send(sock, tx_buffer, iSendLen, 0);
							if(iSendLen < 0)
							{
								ezlog_e(TAG_APP," Error occured during sending: errno %d", errno);
								break;
							}	
						}
						else if(LAN_TYPE_DATA_TRANSFER == struHeader.type)
						{

							if(MUSIC_LED_CTRL == rx_buffer[3])	
							{
								if(sizeof(struLedCtrl) != (len-3))
								{
									ezlog_e(TAG_APP, "invalid param!!! receive len is not equal with ledctrlenght");
								}
							}
							
							ezos_memcpy(&struLedCtrl,&rx_buffer[3],sizeof(struLedCtrl));					

							ledctrl_netparam_convert(&struLedCtrl);

							led_ctrl_do_async(&struLedCtrl);
							
		        		}
					}
				}
            }
        }

        if (sock != -1) {
        	if (true == get_light_switch())
        	{				
        		//regular_power_up();
            }
			ezlog_i(TAG_APP, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            closesocket(sock);
            vTaskDelay(100);//停留1s
        }
    }
    vTaskDelete(NULL); 
    g_pthreadTaskTcpServer = NULL;
    return;
}

void music_server_start(void)
{
	g_exit_music = false;

	if(NULL == g_pthreadTaskTcpServer)
	{
        xTaskCreate(tcp_server_task, "tcp_server", 2560, NULL, 9, &g_pthreadTaskTcpServer);
	}
	else
	{
        ezlog_w(TAG_APP, "tcp_server is already create\n");
	}

	return;
}


void music_server_stop(void)
{
	ezlog_i(TAG_APP, "enter %s", __func__);
	
	if (NULL != g_pthreadTaskTcpServer)
	{
		ezlog_i(TAG_APP, "delete g_pthreadTaskTcpServer\n");
		g_exit_music = true;
		vTaskDelay(300/portTICK_PERIOD_MS);	

		vTaskDelete(g_pthreadTaskTcpServer);
		g_pthreadTaskTcpServer = NULL;
      
	}

	if (listen_sock != -1) 
	{
		ezlog_i(TAG_APP, "Shutting down listen_socket \n");
		shutdown(listen_sock, 0);
		/* 这里调用close ，系统会卡住，暂时不知道原因，先屏蔽*/
		//closesocket(listen_sock);
		listen_sock = -1;

	}
	return;
}


