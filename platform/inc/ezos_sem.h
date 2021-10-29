#ifndef H_EZOS_SEM_H_
#define H_EZOS_SEM_H_

#if (defined(_WIN32) || defined(_WIN64))
#  if defined(EZOS_API_EXPORTS)
#    define EZOS_API __declspec(dllexport)
#  else
#    define EZOS_API __declspec(dllimport)
#  endif
#  define EZOS_CALL __stdcall
#elif defined(__linux__)
#  define EZOS_API
#  define EZOS_CALL
#else
#  define EZOS_API
#  define EZOS_CALL
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
EZOS_API ez_sem_t EZOS_CALL ez_sem_create(void);

/** 
 *  \brief		删除信号量
 *  \method		ezos_mutex_unlock
 *  \param[in] 	sem 信号量句柄
 *  \return 	成功返回0 失败返回-1
 */
EZOS_API int EZOS_CALL ez_sem_destory(ez_sem_t sem);

/** 
 *  \brief		等待信号量
 *  \method		ez_sem_wait
 *  \param[in] 	sem 信号量句柄
 *  \param[in]  timewait_ms 等待超时事件ms,-1表示一直等待
 *  \return 	成功返回0 失败返回-1
 */
EZOS_API int EZOS_CALL ez_sem_wait(ez_sem_t sem, int timewait_ms);

/** 
 *  \brief		发送信号量
 *  \method		ez_sem_post
 *  \param[in] 	sem 信号量句柄
 *  \return 	成功返回0 失败返回-1
 */
EZOS_API int EZOS_CALL ez_sem_post(ez_sem_t sem);

#ifdef __cplusplus
}
#endif

#endif//H_EZOS_SEM_H_
