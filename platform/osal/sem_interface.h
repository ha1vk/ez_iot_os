#ifndef H_SEM_INTERFACE_H_
#define H_SEM_INTERFACE_H_

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

typedef void * ez_sem_t;


/** 
 *  \brief		创建信号量
 *  \method		ez_sem_create
 *  \return 	成功返回信号量句柄 失败返回NULL
 */
EZ_OS_API_EXTERN ez_sem_t EZ_OS_API_CALL ez_sem_create(void);

/** 
 *  \brief		删除信号量
 *  \method		ez_mutex_unlock
 *  \param[in] 	sem 信号量句柄
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_sem_destory(ez_sem_t sem);

/** 
 *  \brief		等待信号量
 *  \method		ez_sem_wait
 *  \param[in] 	sem 信号量句柄
 *  \param[in]  timewait_ms 等待超时事件ms,-1表示一直等待
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_sem_wait(ez_sem_t sem, int timewait_ms);

/** 
 *  \brief		发送信号量
 *  \method		ez_sem_post
 *  \param[in] 	sem 信号量句柄
 *  \return 	成功返回0 失败返回-1
 */
EZ_OS_API_EXTERN int EZ_OS_API_CALL ez_sem_post(ez_sem_t sem);

#ifdef __cplusplus
}
#endif

#endif
