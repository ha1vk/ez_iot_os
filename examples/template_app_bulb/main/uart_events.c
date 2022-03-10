/* UART Events Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "ezlog.h"
#include "ezos_time.h" //延迟需要

#include "bulb_led_ctrl.h"

//#include "esp_system.h"
// #include "lwip/arch.h"
// #include "lwip/opt.h"
// #include "arch/perf.h"
static const char *TAG = "uart_events";
#define EX_UART_NUM UART_NUM_0
#define PATTERN_CHR_NUM    (3)         /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/
typedef struct {
	unsigned char head[2];//包头
	unsigned short usLength;//长度
	unsigned int cmd;//命令包
}COMM_DATA_HEAD_INFO;
#define BUF_SIZE (100)
#define RD_BUF_SIZE (BUF_SIZE)
static QueueHandle_t uart0_queue;
static unsigned char uart_buf[BUF_SIZE];
static int g_reply_length = 0;
//static COMM_DATA_HEAD_INFO g_header;
//char dstMac[6] = {0};
// /**
//  * This example shows how to use the UART driver to handle special UART events.
//  *
//  * It also reads data from UART0 directly, and echoes it to console.
//  *
//  * - Port: UART0
//  * - Receive (Rx) buffer: on
//  * - Transmit (Tx) buffer: off
//  * - Flow control: off
//  * - Event queue: on
//  * - Pin assignment: TxD (default), RxD (default)
//  */
// static const unsigned char auchCRCHi[] = 
// 	{
// 		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
// 		0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
// 		0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
// 		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
// 		0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
// 		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
// 		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
// 		0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
// 		0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
// 		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
// 		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
// 		0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
// 		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
// 		0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
// 		0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
// 		0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40
// 	} ;
// // -----------------------------------------------------------------------------
// // DESCRIPTION: RTU CRC 地位列表
// // -----------------------------------------------------------------------------
// static const unsigned char auchCRCLo[] = 
// 	{
// 		0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,0x07,0xC7,0x05,0xC5,0xC4,0x04,
// 		0xCC,0x0C,0x0D,0xCD,0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,0x08,0xC8,
// 		0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,
// 		0x14,0xD4,0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,0x11,0xD1,0xD0,0x10,
// 		0xF0,0x30,0x31,0xF1,0x33,0xF3,0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
// 		0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,0x3B,0xFB,0x39,0xF9,0xF8,0x38,
// 		0x28,0xE8,0xE9,0x29,0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,0xEC,0x2C,
// 		0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,
// 		0xA0,0x60,0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,0xA5,0x65,0x64,0xA4,
// 		0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
// 		0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,
// 		0xB4,0x74,0x75,0xB5,0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,0x70,0xB0,
// 		0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,
// 		0x9C,0x5C,0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,0x99,0x59,0x58,0x98,
// 		0x88,0x48,0x49,0x89,0x4B,0x8B,0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
// 		0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,0x43,0x83,0x41,0x81,0x80,0x40
// 	};



// #define HEAD_MAGIC_LENGTH 2         //ĺčŽŽé­ć°ĺ 2ä¸Şĺ­č 0xFD 0xFC
// #define DATA_LENGTH 2               //ć°ćŽéżĺşŚĺ 2ä¸Şĺ­č
// #define CMD_LENGTH 4                //ĺ˝äť¤ĺ 4ä¸Şĺ­č
// #define CRC_LENGTH 2                //CRCĺ 2ä¸Şĺ­č
// #define HEAD_MAGIC_FIRST_BYTE 0xFD  //é­ć°çŹŹä¸ä¸Şĺ­č
// #define HEAD_MAGIC_SECOND_BYTE 0xFC //é­ć°çŹŹäşä¸Şĺ­č
// #define CMD_CODE_BEGIN 1000
// #define CMD_CODE_END 1043
// #define CMD_CODE_GPIO_LIGHTS 1040 //wifić¨ĄçťçgpioçŻ
// #define CMD_CODE_WIFI_CONNECT 1041
// #define CMD_CODE_WIFI_CALL_FLOW 1042 //wifiććľ
// #define CMD_CODE_WIFI_RESET 1025     //ć˘ĺ¤ĺşĺ
// #define CMD_CODE_INFO_CHECK 1045     //äżĄćŻć ĄéŞďźć ĄéŞĺşĺĺˇăPTIDăčŽžĺ¤ĺĺˇăč˝ŻäťśçćŹ4ä¸ŞäżĄćŻ
// #define CMD_CODE_REBOOT 1048         //čŽžĺ¤éĺŻ
// #ifdef htons
// #undef htons
// #endif /* htons */
// #ifdef htonl
// #undef htonl
// #endif /* htonl */
// #ifdef ntohs
// #undef ntohs
// #endif /* ntohs */
// #ifdef ntohl
// #undef ntohl
// #endif /* ntohl */
// #define htons(x) lwip_htons(x)
// #define ntohs(x) lwip_ntohs(x)
// #define htonl(x) lwip_htonl(x)
// #define ntohl(x) lwip_ntohl(x)
// #define lwip_htons(x) LWIP_PLATFORM_HTONS(x)
// #define lwip_ntohs(x) LWIP_PLATFORM_HTONS(x)
// #define lwip_htonl(x) LWIP_PLATFORM_HTONL(x)
// #define lwip_ntohl(x) LWIP_PLATFORM_HTONL(x)
// static void do_reboot()
// {
//     ezlog_i(TAG_APP, "do_reboot!!!");
//     ezos_delay_ms(3000); 
//     //vTaskDelay(1000 / portTICK_RATE_MS);
//     esp_restart();
// }

// int mac_str2hex(const char *buf, char* pValue)
// {
//     int digit;
//     int i = 0;
//     char c;
//     pValue[0] = 0;
//     while((c = *buf++))
//     {
//         //printf("c is %c\n",c);
//         if(i>5)
//         {
//             return false;
//         }
//         if((':' == c) || ('-' == c) || (' ' == c))
//         {
//             i++;
//             pValue[i] = 0;
//         }
//             continue;
//         if(48<=c && c<=57)
//         {
//             digit = c - 48;
//         }
//         else if (65 <= c && c <=70)
//         {
//             digit = c - 'A' + 10;
//         }
//         else if (97 <= c && c <=102)
//         {
//             digit = c - 'a' + 10;
//         }
//         else
//         {
//             return false;
//         }
//         if((digit<0) || (digit>16))
//         {
//             return false;
//         }
//         pValue[i] = 16*(pValue[i]) + digit;
//     }

//     return true;
// }

int hextoi(char *s)
{
	int i=0;
	int t=0;             //t记录临时加的数 
	long sum =0;	
	for(i=0;s[i] !=' ';i++)
	{
		if(s[i]>='0'&&s[i]<='9')
		t=s[i]-'0';       //当字符是0~9时保持原数不变
		if(s[i]>='a'&&s[i]<='z')
		t=s[i]-'a'+10;
		if(s[i]>='A'&&s[i]<='Z')
		t=s[i]-'A'+10;
		sum=sum*16+t;
	}
	return sum;
 } 


//  unsigned short RTU_CRC( unsigned char * puchMsg,unsigned short usDataLen,unsigned short nAccum )
// {
// 		unsigned char uchCRCHi; // high byte of CRC initialized
// 		unsigned char uchCRCLo; // low byte of CRC initialized
// 		unsigned uIndex; // will index into CRC lookup table

// 		uchCRCHi = nAccum>>8;
// 		uchCRCLo = (nAccum&0xFF);
// 		if (usDataLen==0)return 0;
// 		while ( usDataLen-- )
// 		{
// 			// calculate the CRC
// 			uIndex = uchCRCHi ^ (unsigned char)( *puchMsg++ );
// 			uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
// 			uchCRCLo = auchCRCLo[uIndex];
// 		}
// 		 return ( uchCRCHi << 8 | uchCRCLo );
		 
// }


// int check_is_valid(unsigned char *data, int len)
// {
//     ezlog_hexdump(TAG_APP, 16, data, len);

//     if (NULL == data || len < 8)
//     {
//         ezlog_e(TAG_APP, "protocol_handle head is NULL or len[%d] is error!\n", len);
//         return -1;
//     }

//     COMM_DATA_HEAD_INFO *header = (COMM_DATA_HEAD_INFO *)data;

//     memset(&g_header, 0, sizeof(COMM_DATA_HEAD_INFO));
//     g_header.head[0] = data[0];
//     ezlog_w(TAG_APP, "data[0]:%d",data[0]);
//     g_header.head[1] = data[1];
//     ezlog_w(TAG_APP, "data[1]:%d",data[1]);
//     g_header.usLength = ntohs(header->usLength);
//     g_header.cmd = ntohl(header->cmd);
//     ezlog_w(TAG_APP, "header->cmd:%d",header->cmd);
//     ezlog_w(TAG_APP, "g_header.cmd:%d",g_header.cmd);
//     // if (g_header.head[0] != HEAD_MAGIC_FIRST_BYTE || g_header.head[1] != HEAD_MAGIC_SECOND_BYTE)
//     // {
//     //     return -1;
//     // }

//     // unsigned char *pdata = data + HEAD_MAGIC_LENGTH;

//     // unsigned short crc_data = RTU_CRC(pdata, DATA_LENGTH + g_header.usLength, 0xFFFF);

//     // int offset = HEAD_MAGIC_LENGTH + DATA_LENGTH + g_header.usLength;
//     // unsigned short _crc_data_source = 0;
//     // memcpy(&_crc_data_source, data + offset, CRC_LENGTH);
//     // unsigned short crc_data_source = ntohs(_crc_data_source);

//     // if (crc_data != crc_data_source)
//     // {
//     //     ezlog_e(TAG_APP, "protocol_handle crc check error crc_data[%d] crc_data_source[%d]!\n", crc_data, crc_data_source);
//     //     return -1;
//     // }

//     return 0;
// }

// int protocol_handle(unsigned char *data, int len)
// {
//     if (0 != check_is_valid(data, len))
//     {
//         return -2;
//     }

//     int ret = 0;
//     ezlog_w(TAG_APP, "get factory cmd");
//     ezlog_w(TAG_APP, "get factory cmd:%d",g_header.cmd);
//     switch (g_header.cmd)
//     {
//     case CMD_CODE_BEGIN:
//         ret =0; //test_begin();
//         break;

//     case CMD_CODE_END:
//         ret =0;// test_end();
//         break;

//     case CMD_CODE_GPIO_LIGHTS:
//         ret = 0;//light_test();
//         break;

//     case CMD_CODE_WIFI_CONNECT:
//         ret = 0;//wifi_connect_test(data, len);
//         break;

//     case CMD_CODE_WIFI_CALL_FLOW:
//         ret = 0;//rssi_connect_test(data, len);
//         break;

//     case CMD_CODE_WIFI_RESET:
//         ret =0;// dev_reset_test();
//         break;

//     case CMD_CODE_INFO_CHECK:
//         ret = 0;//info_check_test();
//         break;

//     case 1234567890:
//         do_reboot();
//         break;

//     default:
//         break;
//     }

//     return ret;
// }

static void pasreATCMD(unsigned char *data, int len)
{
    const char *prestr = "AT+CMD=";
    const char *atcmd_loglvl = "setloglvl";

    int loglvl = 0;

    char *p = NULL;

    char *p1srcLm = NULL;
    char *p2dstLm = NULL;
    char *p3srcRgb = NULL;
    char *p4dstRgb= NULL;
    char *p5mSleep = NULL;
    char *p6chgLoops = NULL;
    
    int srcLm=0;
    int dstLm=0;
    int srcRgb=0;
    int dstRgb=0;
    int mSleep =0;
    int chgLoops = 0;
    int index = 0;
	led_ctrl_t led_ctrl_cmd = {0};
	
    ezlog_w(TAG_APP, "get at cmd:%s", data);
    ezlog_w(TAG_APP, "len:%d", len);
    if (strlen(prestr) >= len && 0 == strncmp((const char *)data, prestr, strlen(prestr)))
    {
        ezlog_w(TAG_APP, "get at cmd:%s", data);
        return;
    }

    if (NULL == (p = strstr((const char *)data, atcmd_loglvl)))
    {
        ezlog_w(TAG_APP, "get at cmd:%s", data);
        return;
    }
    ezlog_w(TAG_APP, "get at cmd:%s", data);
    if (strlen(p) > strlen(atcmd_loglvl) + 1 && 0 == strncmp(p, atcmd_loglvl, strlen(atcmd_loglvl)))
    {
        ezlog_w(TAG_APP, "get at cmd:%s", data);
        if (NULL == (p = strstr((const char *)p, " ")))
        {
            return;
        }

        loglvl = atoi(p+1);
		if(loglvl<6)
		{
            ezlog_w(TAG_APP, "the level is:%d", loglvl);
            ezlog_filter_lvl(loglvl);
		}
// 		else if(loglvl == 6)
// 		{
// 			ezlog_i(TAG_APP,"line (%d) and function (%s)): wifiinfo=%s\n "
//  				,__LINE__, __func__,g_wifi_data);
// 		}
// 		else if(loglvl == 7)
// 		{
// 			esp_print_tasks();
// 			heap_caps_print_heap_info(MALLOC_CAP_32BIT);
// 		}
// 		else if(loglvl == 8)
// 		{
// 			if (NULL == (pMacStr = strstr((const char *)p + 1, " ")))
// 			{
// 			   return;
// 			}
// 			pMacStr++;
// 			mac_str2hex(pMacStr,dstMac);			
// 		}
// 		else if(loglvl == 9)
// 		{
			
// 			if (NULL == (pSceneDurationStr = strstr((const char *)p + 1, " ")))
// 			{
// 			   return;
// 			}
			
// 			if (NULL == (pSpeedtimeStr = strstr((const char *)pSceneDurationStr + 1, " ")))
// 			{
// 			   return;
// 			}

// 			if (NULL == (psceneName = strstr((const char *)pSpeedtimeStr + 1, " ")))
// 			{
// 			   return;
// 			}
			
	
// 			pSceneDurationStr++;
// 			pSpeedtimeStr++;
// 			psceneName++;
					
// 			iSceneDuration = atoi(pSceneDurationStr);
// 			iSpeedtime = atoi(pSpeedtimeStr);
			
// 			g_light_scene_cfg.duration = iSceneDuration;
// 			g_light_scene_cfg.speed = iSpeedtime;

// 		}
        else
        {
            if (NULL == (p1srcLm = strstr((const char *)p + 1, " ")))
            {
            return;
            }

            if (NULL == (p2dstLm= strstr((const char *)p1srcLm + 1, " ")))
            {
            return;
            }

            if (NULL == (p3srcRgb = strstr((const char *)p2dstLm + 1, " ")))
            {
            return;
            }

            if (NULL == (p4dstRgb = strstr((const char *)p3srcRgb + 1, " ")))
            {
            return;
            }

            if (NULL == (p5mSleep = strstr((const char *)p4dstRgb + 1, " ")))
            {
            return;
            }

            if (NULL == (p6chgLoops = strstr((const char *)p5mSleep + 1, " ")))
            {
            return;
            }
            
            p1srcLm++;
            p2dstLm++;
            p3srcRgb++;
            p4dstRgb++;
            p5mSleep++;
            p6chgLoops++;

//AT+CMD=setloglvl 2700 1 20 100 100 20 50

            srcLm = atoi(p1srcLm);
            dstLm= atoi(p2dstLm);

            srcRgb = hextoi(p3srcRgb);
            dstRgb = hextoi(p4dstRgb);
            mSleep =  atoi(p5mSleep);
            chgLoops = atoi(p6chgLoops);

            ezlog_i(TAG_APP,"line (%d) and function (%s)): cct=%d,srclm=%d,destlm=%d,srcrgb=0x%x,destrgb=0x%x,msleep=%d,loops=%d\n "
                ,__LINE__, __func__,loglvl,srcLm,dstLm,srcRgb,dstRgb,mSleep,chgLoops);
            if(loglvl > 6500)
            {			
             
                led_ctrl_cmd.iRgbValue = dstRgb;
                led_ctrl_cmd.nbrightness = dstLm;
                led_ctrl_cmd.nUpDuration = mSleep * chgLoops;
                led_ctrl_do_async(&led_ctrl_cmd);
            }
            else
            {
                for(index=0;index <2;index++)
                {
                    led_ctrl_cmd.iRgbValue = 0;
                    led_ctrl_cmd.nCctValue = loglvl;
                    led_ctrl_cmd.nbrightness = dstLm;
                    led_ctrl_cmd.nUpDuration = mSleep * chgLoops;
                    led_ctrl_do_async(&led_ctrl_cmd);

                }
            }

		}

     }
}

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    //uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(uart0_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            // bzero(dtmp, RD_BUF_SIZE);
            // ESP_LOGI(TAG, "uart[%d] event:", EX_UART_NUM);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    //ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                    uart_read_bytes(EX_UART_NUM, (uint8_t *)uart_buf, event.size, portMAX_DELAY);
                    //ez_log_hexdump(TAG_APP, 32, uart_buf, event.size);
                   // ESP_LOGI(TAG, "[DATA EVT]:");
                    if (1)//-2 == protocol_handle(uart_buf, event.size))
                    {
                        pasreATCMD(uart_buf, event.size);
                        break;
                    }
                    uart_write_bytes(EX_UART_NUM, (const char *)uart_buf, g_reply_length);
                    //ez_log_hexdump(TAG_APP, 32, uart_buf, g_reply_length);
                    g_reply_length = 0;
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(uart0_queue);
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    vTaskDelete(NULL);
}

int uart_init(void)
{
    //Create a task to handler UART event from ISR
        uart_config_t uart_config = {
        .baud_rate = 460800,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    esp_err_t ret = ESP_OK;

    ret = uart_param_config(EX_UART_NUM, &uart_config);
    if (ESP_OK != ret)
    {
        ezlog_e(TAG_APP, "uart_init uart_param_config error!");
        return ret;
    }

    ret = uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 100, &uart0_queue);
    if (ESP_OK != ret)
    {
        ezlog_e(TAG_APP, "uart_init uart_driver_install error!");
        return ret;
    }
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 5, NULL);
    return ret;
}
