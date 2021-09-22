

#ifndef H_THREAD_INTERFACE_H_
#define H_THREAD_INTERFACE_H_

#if (defined(_WIN32) || defined(_WIN64))
#  if defined(EZ_OS_API_EXPORTS)
#    define EZ_OS_API_EXTERN __declspec(dllexport)
#  else
#    define EZ_OS_API_EXTERN __declspec(dllimport)
#  endif
#  define EZ_OS_API_CALL __stdcall
#elif defined(__linux__)
#  define EZ_OS_API_EXTERN
#  define EZ_OS_API_CALL
#else
#  define EZ_OS_API_EXTERN
#  define EZ_OS_API_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define THREAD_STACKSIZE_MAX		8196*1024
#define THREAD_STACKSIZE_MIN		1024

typedef void * ez_thread_t;
typedef void * ez_mutex_t;

typedef struct
{
	void (EZ_OS_API_CALL * task_fun)(void *task_arg);   // 线程入口函数 
	void *task_arg;						                // 入口函数的传入参数  
	unsigned char priority;				                // 线程的优先级 
	unsigned int stackSize;				                // 程堆栈的大小 
	char task_name[64];					                // 线程名称 
	unsigned int tick;					                // 线程的时间片大小 
}ez_task_init_parm;

/** 
 *  \brief		创建线程
 *  \method		ez_thread_create
 *  \param[in] 	taskParam 线程参数
 *  \return 	成功返回线程句柄 失败返回NULL
 */
EZ_OS_API_EXTERN ez_thread_t EZ_OS_API_CALL ez_thread_create(ez_task_init_parm *taskParam);

/** 
 *  \brief		线程分离
 *  \method		ez_thread_detach
 *  \param[in] 	handle 线程句柄
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_thread_detach(ez_thread_t handle);
/** 
 *  \brief		线程销毁
 *  \method		ez_thread_destroy
 *  \param[in] 	handle 线程句柄
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_thread_destroy(ez_thread_t handle);

/** 
 *  \brief		获取线程id
 *  \method		ez_thread_self
 *  \return 	返回线程id
 */
 EZ_OS_API_EXTERN unsigned int EZ_OS_API_EXTERN ez_thread_self();

/** 
 *  \brief		创建互斥锁
 *  \method		ez_mutex_create
 *  \return 	成功返回互斥锁句柄 失败返回NULL
 */
EZ_OS_API_EXTERN  ez_mutex_t EZ_OS_API_CALL ez_mutex_create(void);

/** 
 *  \brief		删除互斥锁
 *  \method		ez_mutex_destory
 *  \param[in] 	mutex 互斥锁句柄
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_mutex_destory(ez_mutex_t mutex);

/** 
 *  \brief		互斥锁上锁
 *  \method		ez_mutex_lock
 *  \param[in] 	mutex 互斥锁句柄
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_mutex_lock(ez_mutex_t mutex);

/** 
 *  \brief		释放互斥锁
 *  \method		ez_mutex_unlock
 *  \param[in] 	mutex 互斥锁句柄
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_mutex_unlock(ez_mutex_t mutex);

#ifdef __cplusplus
}
#endif

#endif