#ifndef H_EZDEVSDK_CONFIG_H_
#define H_EZDEVSDK_CONFIG_H_

/** 
 *  \brief		devinfo配置文件的读取
 *  \method		get_devinfo_fromconfig
 *  \param[in]	path				路径
 *  \param[out]	devinfo_context		数据地址
 *  \param[in]	devinfo_context_len	数据长度
 *  \returns    成功返回0,失败返回-1
 */
int get_devinfo_fromconfig(const char* path, char* devinfo_context, int devinfo_context_len);

/** 
 *  \brief		数据保存至文件，目前用于devid和masterkey
 *  \method		set_file_value
 *  \param[in]	path				路径
 *  \param[in]	devinfo_context		数据地址
 *  \param[in]	devinfo_context_len	数据长度
 *  \returns    成功返回0,失败返回-1
 */
int set_file_value(const char* path, unsigned char* keyvalue, int keyvalue_size);

/** 
 *  \brief		从文件读取，目前用于devid和masterkey，读取长度小于64字节
 *  \method		get_file_value
 *  \param[in]	path				路径
 *  \param[out]	devinfo_context		数据地址
 *  \param[in]	devinfo_context_len	数据长度
 *  \returns    成功返回0,失败返回-1
 */
int get_file_value(const char* path, unsigned char* keyvalue, int keyvalue_maxsize);

#endif