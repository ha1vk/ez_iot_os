

#ifndef H_EZOS_THREAD_H_
#define H_EZOS_THREAD_H_

#include <ezos_def.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void *ez_thread_t;
    typedef void *ez_mutex_t;
    typedef void (*ez_thread_func_t)(void *param);

    /**
     * @brief This function will create a thread object
     * 
     * @param handle if non-null, return the thread handle, if null, marks the thread as detached, the resources whill released back to the system automatically
     * @param name the name of thread, which shall be unique
     * @param thread_fun the function of thread
     * @param param thread parameter of thread function
     * @param stack_size the size of thread stack
     * @param priority the size of thread stack
     * @return 0 for success, -1 for failure 
     */
    EZOS_API ez_err_t EZOS_CALL ezos_thread_create(ez_thread_t *const handle, const ez_char_t *name, ez_thread_func_t thread_fun,
                                                   const ez_void_t *param, ez_uint32_t stack_size, ez_uint32_t priority);

    /**
     * @brief The function waits for the thread specified by handle to terminate.  If that thread has
     *        already terminated, returns immediately. 
     * 
     * @param handle thread handle
     * @return 0 for success, -1 for failure 
     */
    EZOS_API ez_err_t EZOS_CALL ezos_thread_destroy(ez_thread_t handle);

    /** 
     * @brief  Get the unique ID of the calling thread
     * 
     * @return returning the calling thread's ID
     */
    EZOS_API ez_int32_t EZOS_CALL ezos_thread_self(ez_void_t);

    /**
     * @brief 
     * 
     * @return non-null for success, null for failure
     */
    EZOS_API ez_mutex_t EZOS_CALL ezos_mutex_create(void);

    /**
     * @brief 
     * 
     * @param mutex 
     * @return 0 for success, -1 for failure
     */
    EZOS_API ez_err_t EZOS_CALL ezos_mutex_destory(ez_mutex_t mutex);

    /**
     * @brief 
     * 
     * @param mutex 
     * @return 0 for success, -1 for failure
     */
    EZOS_API ez_err_t EZOS_CALL ezos_mutex_lock(ez_mutex_t mutex);

    /**
     * @brief 
     * 
     * @param mutex 
     * @return 0 for success, -1 for failure
     */
    EZOS_API ez_err_t EZOS_CALL ezos_mutex_unlock(ez_mutex_t mutex);

#ifdef __cplusplus
}
#endif

#endif //H_EZOS_THREAD_H_