#ifndef EZ_BULB_DEV_INFO_H
#define EZ_BULB_DEV_INFO_H

#define MAGIC_NUMBER 0x53574B48
#define MAGIC_NUMBER_INVERT 0x484B5753
#define RAND_LEN 6
#define MAX_ETHERNET 2
#define MACADDR_LEN 6
#define PRODDATELEN 8
#define PRODNUMLEN 9
#define MODEL_LEN 64
#define AUTH_MODE_SAP 0
#define AUTH_MODE_LIC 1

typedef struct
{
    /* 0  */ unsigned int magicNumber;                         //幻数
    /* 4  */ unsigned int paraChecksum;                        //检查和
    /* 8  */ unsigned int paraLength;                          //结构长度  检查和、长度从'encryptVer'开始计算
    /* 12 */ unsigned int encryptVer;                          //加密版本:用于控制此结构的更改
                                                               /*
                                                                   以下4项用户升级控制:必须与升级文件包中的内容一致
                                                               */
    /* 16 */ unsigned int language;                            //支持语言
    /* 20 */ unsigned int deviceClass;                         //产品类型，1 -- DS9000 DVR, ...
    /* 24 */ unsigned int oemCode;                             // oem代码：1---代表自己
    /* 28 */ unsigned short devType_high;                      //设备类型高字节，高2个字节，低2个字节在devType
    /* 30 */ unsigned char rand_code[RAND_LEN];                // 6位验证码，英文字符，不区分大小写  verifyCode
    /* 36 */ unsigned char res1[8];                            //保留
    /* 44 */ unsigned short encodeChans;                       //编码路数
    /* 46 */ unsigned short decodeChans;                       //解码路数
    /* 48 */ unsigned short ipcChans;                          // IPC通道数
    /* 50 */ unsigned short ivsChans;                          //智能通道数
    /* 52 */ unsigned char alarminNum;                         //报警输入个数0,1,2,3,4,5,6,7,8
    /* 53 */ unsigned char macAddr[MAX_ETHERNET][MACADDR_LEN]; //设备MAC地址
    /* 65 */ unsigned char prodDate[PRODDATELEN];              //设备生产日期,ASCII
    /* 73 */ unsigned char prodNo[PRODNUMLEN];                 //设备序列号
    /* 82 */ unsigned char videoStandard;                      //视频制式 0=PAL|1=NTSC
    /* 83 */ unsigned char cpuFreq;                            // CPU频率 1=229M|2=300M|3=337M|4=364M|5=450M|6=500M
    /* 84 */ unsigned char dspFreq;                            // DSP频率 1=459M|2=600M|3=675M|4=729M|5=900M|6=1G
    /* 85 */ unsigned char zone;                               //销售地区
    /* 86 */ unsigned char audioInSupport;                     //支持音频输入
    /* 87 */ unsigned char sensorType;                         //支持sensor类型
    /* 88 */ unsigned char usbNums;                            // USB个数
    /* 89 */ unsigned char resetSupport;                       //支持reset
    /* 90 */ unsigned char alarmoutNums;                       //报警输出: 0,1,2,3,4
    /* 91 */ unsigned char sensorVersion;                      // sensor板版本
    /* 92 */ unsigned char audiooutSupport;                    //支持音频输出：0 -- 无，1--支持
    /* 93 */ unsigned char videooutType;                       //视频输出类型：0 -- 无，1 -- CVBS，2 -- HDMI
    /* 94 */ unsigned char videoinType;                        //视频输入类型 0 -- 9d131, 1-- 9d136,2 --656,
    /* 95 */ unsigned char hardwareType;                       //主板类型
    /* 96 */ unsigned char hardwareVersion;                    //主板版本
    /* 97 */ unsigned char IRSupport;                          //支持红外功能 0不支持；1支持
    /* 98 */ unsigned char wifiSupport;                        //支持wifi功能 0不支持 1支持
    /* 99 */ unsigned char autoIrisType;                       //电动镜头类型，0不支持；1老款设备；2-。一体机中表示镜头类型
    /*100 */ unsigned short devType;                           //设备类型低字节，2个字节
    /*102 */ unsigned char ICRSupport;                         //支持ICR
    /*103 */ unsigned char deviceTypeSecond;                   // device
    /*104 */ unsigned int ubootAdrs;                           // uboot存放flash地址
    /*108 */ unsigned int ubootSize;                           // uboot大小
    /*112 */ unsigned int ubootCheckSum;                       // uboot校验值
    /*116 */ unsigned int tinyKernelAdrs;                      // tinyKernel存放flash地址
    /*120 */ unsigned int tinyKernelSize;                      // tinyKernel大小
    /*124 */ unsigned int tinyKernelCheckSum;                  // tinyKernel校验值
    /*128 */ unsigned char devModel[MODEL_LEN];                //产品型号:考虑国标型号，扩充到64字节
    /*192 */ unsigned char pirSupport;                         //支持PIR
    /*193 */ unsigned char rfSupport;                          //支持RF
    /*194 */ unsigned char res2[6];                            //预留字段
    /*200 */ unsigned int crypto;                              //是否加密过 1：是，0：否
    /*204 */ unsigned char abfType;                            // ABF类型，0-不支持；1-类型1
    /*205 */ unsigned char lensType;                           //镜头类型，0-2mm;1-2.8mm;2-4mm;3-6mm;4-8mm
    /*206 */ unsigned char customNo[50];
} BOOT_PARAMS, *PBOOT_PARAMS;

typedef struct
{
    BOOT_PARAMS boot_params; // boot参数
    unsigned int year;       //当前时间：年、月、日、时、分、秒
    unsigned int month;
    unsigned int day;
    unsigned int hour;
    unsigned int minute;
    unsigned int second;
    unsigned int dayofweek;
    unsigned long dogId;      //设置人员的ID(加密狗的流水号)
    unsigned int formatFlash; // 0 --- 不格式化，1 --- 格式化
    unsigned int irewrite;    // 0 --- 不重写，  1 --- 重写
} PRODUCT_INFO, *PPRODUCT_INFO;

typedef struct
{
    char dev_productKey[33];    //产品PID
    char dev_deviceName[13];    //产品序列号
    char dev_deviceLicense[48]; //产品验证码
    char dev_auth_mode;             // 1 for license, 0 for sap
} product_dev_info_t;

char *get_dev_productKey();
char *get_dev_deviceName();
char *get_dev_License();
char get_dev_auth_mode();

int parse_dev_config(char *buf, int buf_size);
#endif