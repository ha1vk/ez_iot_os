

#ifndef H_EZOS_THREAD_H_
#define H_EZOS_THREAD_H_

#include <ezos_def.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THREAD_STACKSIZE_MAX		8196*1024
#define THREAD_STACKSIZE_MIN		1024

typedef void * ez_thread_t;
typedef void * ez_mutex_t;

typedef struct
{
	void (EZOS_CALL * task_fun)(void *task_arg);   // 线程入口函数 
	void *task_arg;						                // 入口函数的传入参数  
	unsigned char priority;				                // 线程的优先级 
	unsigned int stackSize;				                // 程堆栈的大小 
	char task_name[64];					                // 线程名称 
	unsigned int tick;					                // 线程的时间片大小 
}ez_task_init_parm;

/** 
 *  \brief		创建线程
 *  \method		ezos_thread_create
 *  \param[in] 	taskParam 线程参数
 *  \return 	成功返回线程句柄 失败返回NULL
 */
EZOS_API ez_thread_t EZOS_CALL ezos_thread_create(ez_task_init_parm *taskParam);

/** 
 *  \brief		线程分离
 *  \method		ezos_thread_detach
 *  \param[in] 	handle 线程句柄
 *  \return 	成功返回0 失败返回-1
 */
EZOS_API int EZOS_CALL ezos_thread_detach(ez_thread_t handle);

/** 
 *  \brief		线程销毁
 *  \method		ezos_thread_destroy
 *  \param[in] 	handle 线程句柄
 *  \return 	成功返回0 失败返回-1
 */
EZOS_API int EZOS_CALL ezos_thread_destroy(ez_thread_t handle);

/** 
 *  \brief		获取线程id
 *  \method		ezos_thread_self
 *  \return 	返回线程id
 */
 EZOS_API unsigned int EZOS_CALL ezos_thread_self();

/** 
 *  \brief		创建互斥锁
 *  \method		ezos_mutex_create
 *  \return 	成功返回互斥锁句柄 失败返回NULL
 */
EZOS_API  ez_mutex_t EZOS_CALL ezos_mutex_create(void);

/** 
 *  \brief		删除互斥锁
 *  \method		ezos_mutex_destory
 *  \param[in] 	mutex 互斥锁句柄
 *  \return 	成功返回0 失败返回-1
 */
EZOS_API int EZOS_CALL ezos_mutex_destory(ez_mutex_t mutex);

/** 
 *  \brief		互斥锁上锁
 *  \method		ezos_mutex_lock
 *  \param[in] 	mutex 互斥锁句柄
 *  \return 	成功返回0 失败返回-1
 */
EZOS_API int EZOS_CALL ezos_mutex_lock(ez_mutex_t mutex);

/** 
 *  \brief		释放互斥锁
 *  \method		ezos_mutex_unlock
 *  \param[in] 	mutex 互斥锁句柄
 *  \return 	成功返回0 失败返回-1
 */
EZOS_API int EZOS_CALL ezos_mutex_unlock(ez_mutex_t mutex);

#ifdef __cplusplus
}
#endif

#endif//H_EZOS_THREAD_H_