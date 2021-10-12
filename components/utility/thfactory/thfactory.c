/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *******************************************************************************/
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include "osal_thread.h"
#include "osal_time.h"
#include "osal_mem.h"

#include <string.h>
#include "thfactory.h"
#include "tool/sem.h"

#ifdef THPOOL_DEBUG
#define THPOOL_DEBUG 1
#else
#define THPOOL_DEBUG 0
#endif

#if !defined(DISABLE_PRINT) || defined(THPOOL_DEBUG)
#define err(str) fprintf(stderr, str)
#else
#define err(str)
#endif

/* ========================== STRUCTURES ============================ */

/* Job */
typedef struct job
{
    struct job *prev;                /* pointer to previous job   */
    int (*function)(void *arg);      /* function pointer          */
    void *arg;                       /* function's argument       */
    void (*arg_free_func)(void *arg);  /* function's pointer to free argument       */
} job;

/* Job queue */
typedef struct jobqueue
{
    ez_mutex_t rwmutex; /* used for queue r/w access */
    job *front;              /* pointer to front of queue */
    job *rear;               /* pointer to rear  of queue */
    void *has_jobs;          /* flag as binary semaphore  */
    int len;                 /* number of jobs in queue   */
    int max_num;             /* max number of jobs in queue   */
} jobqueue;

/* Thread */
typedef struct thread
{
    int id;                   /* friendly id               */
    ez_thread_t pthread;        /* pointer to actual thread  */
    thfactory_t *thfactory_p; /* access to thpool          */
} thread;

/* =====================public member func=========================== */
static int thfactory_init(thfactory_t *thiz, int num_threads, int flags, int task_num);
static int thfactory_add_task(thfactory_t *thiz, int (*func_ptr)(void *), void *arg_ptr, void (*arg_free_func)(void *));
static int thfactory_wait4(thfactory_t *thiz);
static int thfactory_pause(thfactory_t *thiz);
static int thfactory_resume(thfactory_t *thiz);
static int thfactory_num_threads_working(thfactory_t *thiz);
static void thfactory_final(thfactory_t *thiz);

/* ========================private func============================== */
static int thread_init(thfactory_t *thfactory_p, struct thread **thread_p, int id);
static void *thread_do(struct thread *thread_p);
// static void thread_hold(int sig_id);
static void thread_destroy(struct thread *thread_p);

static int jobqueue_init(jobqueue *jobqueue_p, int max_job_num);
static void jobqueue_clear(jobqueue *jobqueue_p);
static int jobqueue_push(jobqueue *jobqueue_p, struct job *newjob_p);
static struct job *jobqueue_pull(jobqueue *jobqueue_p);
static void jobqueue_destroy(jobqueue *jobqueue_p);

thfactory_t *thfactory_new(void)
{
    thfactory_t *thfactory_new = (thfactory_t *)ez_malloc(sizeof(thfactory_t));
    if (NULL == thfactory_new)
    {
        return NULL;
    }

    memset((void *)thfactory_new, 0, sizeof(thfactory_t));

    thfactory_new->init = thfactory_init;
    thfactory_new->add_task = thfactory_add_task;
    thfactory_new->pause = thfactory_pause;
    thfactory_new->resume = thfactory_resume;
    thfactory_new->final = thfactory_final;
    thfactory_new->num_threads_working = thfactory_num_threads_working;

    return thfactory_new;
}

void thfactory_del(thfactory_t *thiz)
{
    thiz->final(thiz);
    ez_free(thiz);
}

/* ========================== THREADPOOL ============================ */

static int thfactory_init(thfactory_t *thiz, int num_threads, int flags, int task_num)
{
    int ret = -1;
    int n;

    if (0 != thiz->m_binited)
        return -1;

    do
    {
        thiz->m_threads_keepalive = flags & THREAD_KEEPALIVE;
        thiz->m_max_threads = (num_threads < 1) ? 1 : num_threads;
        thiz->m_threads_on_pause = 0;
        thiz->m_num_threads_alive = 0;
        thiz->m_num_threads_working = 0;

        ///< malloc任务队列
        if (NULL == (thiz->m_jobqueue = (jobqueue *)ez_malloc(sizeof(jobqueue))))
        {
            err("thfactory_init(): Could not allocate memory for job queue1\n");
            break;
        }

        ///< 初始化任务队列
        if (-1 == jobqueue_init((jobqueue *)thiz->m_jobqueue, task_num))
        {
            err("thfactory_init(): Could not allocate memory for job queue2\n");
            jobqueue_destroy(thiz->m_jobqueue);
            ez_free(thiz->m_jobqueue);
            thiz->m_jobqueue = NULL;
            break;
        }

        ///< malloc构造线程池
        if (NULL == (thiz->m_threads = (void **)ez_malloc(thiz->m_max_threads * sizeof(struct thread *))))
        {
            err("thpool_init(): Could not allocate memory for threads\n");
            jobqueue_destroy(thiz->m_jobqueue);
            ez_free(thiz->m_jobqueue);
            thiz->m_jobqueue = NULL;
            break;
        }

        memset(thiz->m_threads, 0, thiz->m_max_threads * sizeof(struct thread *));

        ///< 如果工作线程是长活线程，则初始化和启动所有线程
        if (thiz->m_threads_keepalive)
        {
            for (n = 0; n < thiz->m_max_threads; n++)
            {
                thread_init(thiz, (struct thread **)&thiz->m_threads[n], n);
            }

            /* 等到所有线程全部启动 */
            while (thiz->m_num_threads_alive != thiz->m_max_threads)
            {
                ez_delay_ms(1000);
            }
        }

        thiz->m_thcount_lock = ez_mutex_create();
        if(thiz->m_thcount_lock == NULL){
            
        }
        thiz->m_binited = 1;
        ret = 0;
    } while (0);

    return ret;
}

static int thfactory_add_task(thfactory_t *thiz, int (*func_ptr)(void *), void *arg_ptr, void (*arg_free_func)(void *))
{
    if (0 == thiz->m_binited)
        return -1;

    int ret = -1;
    job *newjob;
    int n;
    do
    {
        if (NULL != arg_ptr && NULL == arg_free_func)
        {
            err("thfactory_add_task(): params invalid\n");
            break;
        }

        if (NULL == (newjob = (struct job *)ez_malloc(sizeof(struct job))))
        {
            err("thfactory_add_task(): Could not allocate memory for new job\n");
            break;
        }

        /* 添加函数加参数 */
        newjob->function = func_ptr;
        newjob->arg = arg_ptr;
        newjob->arg_free_func = arg_free_func;

        int jobcount = jobqueue_push((jobqueue *)thiz->m_jobqueue, newjob);
        if( jobcount < 0)
        {
            err("job queue is full,release the rest job,please retry \n");
            ez_free(newjob);
            break;
        }
        ret = 0;

        if (thiz->m_threads_keepalive)
            break;

        if (jobcount > thiz->m_num_threads_alive && thiz->m_num_threads_alive < thiz->m_max_threads)
        {
            ez_mutex_lock(thiz->m_thcount_lock);
            for (n = 0; n < thiz->m_max_threads; n++)
            {
                if (NULL == thiz->m_threads[n])
                {
                    thread_init(thiz, (struct thread **)&thiz->m_threads[n], n);
                    break;
                }
            }
            ez_mutex_unlock(thiz->m_thcount_lock);
        }
    } while (0);

    return ret;
}


static int thfactory_pause(thfactory_t *thiz)
{
    // int n;
    // for (n = 0; n < thpool_p->num_threads_alive; n++)
    // {
    //     pthread_kill(thpool_p->threads[n]->pthread, SIGUSR1);
    // }
    return 0;
}
static int thfactory_resume(thfactory_t *thiz)
{
    // // resuming a single threadpool hasn't been
    // // implemented yet, meanwhile this supresses
    // // the warnings
    // (void)thpool_p;

    // threads_on_hold = 0;
    return 0;
}

/* 销毁线程池 */
static void thfactory_final(thfactory_t *thiz)
{
    if (0 == thiz->m_binited)
        return;

    /* 结束所有线程一级循环条件 */
    if (thiz->m_threads_keepalive)
    {
        thiz->m_threads_keepalive = 0;
        ezsem_post_all(((jobqueue *)thiz->m_jobqueue)->has_jobs);
    }

    /* 清空任务队列，结束所有线程二级循环条件 */
    jobqueue_clear(thiz->m_jobqueue);

    /* 等待所有线程退出 */
    while (thiz->m_num_threads_alive)
    {
        ez_delay_ms(1000);
    }

    /* 销毁任务队列 */
    jobqueue_destroy((jobqueue *)thiz->m_jobqueue);
    ez_free(thiz->m_jobqueue);

    ez_free(thiz->m_threads);
    thiz->m_threads = NULL;
    thiz->m_jobqueue = NULL;
    thiz->m_binited = 0;
}

static int thfactory_num_threads_working(thfactory_t *thiz)
{
    return thiz->m_num_threads_working;
}

/* ============================ THREAD ============================== */

/* Initialize a thread in the thread pool
 *
 * @param thread        address to the pointer of the thread to be created
 * @param id            id to be given to the thread
 * @return 0 on success, -1 otherwise.
 */
static int thread_init(thfactory_t *thfactory_p, struct thread **thread_p, int id)
{
    ez_task_init_parm task_para;

    if (thread_p == NULL)
    {
        err("thread_init(): thread_p is NULL\n");
        return -1;
    }

    *thread_p = (struct thread *)ez_malloc(sizeof(struct thread));

    if (*thread_p == NULL)
    {
        err("thread_init(): Could not allocate memory for thread\n");
        return -1;
    }

    (*thread_p)->thfactory_p = thfactory_p;
    (*thread_p)->id = id;
    
    memset(&task_para, 0, sizeof(ez_task_init_parm));
    task_para.task_fun = thread_do;

    sprintf(task_para.task_name, "ezthpool-tid-%d", id);
    task_para.task_arg = (*thread_p);
    (*thread_p)->pthread = ez_thread_create(&task_para);
    if ((*thread_p)->pthread == NULL){
        ez_log_e(TAG_APP,"Failed to create NetCheckTask thread.\n");
        return -1;
    }

    return 0;
}

/* What each thread is doing
*
* In principle this is an endless loop. The only time this loop gets interuppted is once
* thpool_destroy() is invoked or the program exits.
*
* @param  thread        thread that will run this function
* @return nothing
*/
static void thread_do(struct thread *thread_p)
{
    thfactory_t *thfactory_p = thread_p->thfactory_p;
    int thread_index = thread_p->id;

    /* 更新线程存活计数器 */
    ez_mutex_lock(thfactory_p->m_thcount_lock);
    thfactory_p->m_num_threads_alive += 1;
    ez_mutex_unlock(thfactory_p->m_thcount_lock);

    do
    {
        /* 如果是常驻线程，等待任务队列信号被触发 */
        if (thfactory_p->m_threads_keepalive)
        {
            ezsem_wait(((jobqueue *)thfactory_p->m_jobqueue)->has_jobs);
        }

        ez_mutex_lock(thfactory_p->m_thcount_lock);
        thfactory_p->m_num_threads_working++;
        ez_mutex_unlock(thfactory_p->m_thcount_lock);

        /* 从任务队列中获取一个任务 */
        int (*func_buff)(void *);
        void *arg_buff;
        job *job_p;
        while (NULL != (job_p = jobqueue_pull((jobqueue *)thfactory_p->m_jobqueue)))
        {
            func_buff = job_p->function;
            arg_buff = job_p->arg;
            func_buff(arg_buff);
            ez_free(job_p);
        }

        ez_mutex_lock(thfactory_p->m_thcount_lock);
        if (0 == --thfactory_p->m_num_threads_working)
        {
            
        }
        ez_mutex_unlock(thfactory_p->m_thcount_lock);
    } while (thfactory_p->m_threads_keepalive);

    /* 更新线程存活计数器，销毁线程上下文 */
    ez_mutex_lock(thfactory_p->m_thcount_lock);
    thfactory_p->m_num_threads_alive--;
    thread_destroy(thread_p);
    thfactory_p->m_threads[thread_index] = NULL;
    ez_mutex_unlock(thfactory_p->m_thcount_lock);

    return NULL;
}

/* Frees a thread  */
static void thread_destroy(thread *thread_p)
{
    ez_free(thread_p);
}

/* ============================ JOB QUEUE =========================== */

/* Initialize queue */
static int jobqueue_init(jobqueue *jobqueue_p, int max_job_num)
{
    jobqueue_p->len = 0;
    jobqueue_p->front = NULL;
    jobqueue_p->rear = NULL;
    jobqueue_p->max_num = max_job_num;

    jobqueue_p->has_jobs = ezsem_init();
    if (jobqueue_p->has_jobs == NULL)
    {
        return -1;
    }

    jobqueue_p->rwmutex = ez_mutex_create();
    if(jobqueue_p->rwmutex == NULL){
        
    }

    return 0;
}

/* Clear the queue */
static void jobqueue_clear(jobqueue *jobqueue_p)
{
    void (*arg_free_func)(void *);
    void *arg_p = NULL;
    job *job_p = NULL;

    while (NULL != (job_p = jobqueue_pull(jobqueue_p)))
    {
        arg_free_func = job_p->arg_free_func;
        arg_p = job_p->arg;

        if (NULL != arg_free_func)
            arg_free_func(arg_p);

        ez_free(job_p);
    }
}

/* Add (allocated) job to queue
 */
static int jobqueue_push(jobqueue *jobqueue_p, struct job *newjob)
{
    int count = 0;
    ez_mutex_lock(jobqueue_p->rwmutex);
    newjob->prev = NULL;

    if(jobqueue_p->len > jobqueue_p->max_num)
    {
        //队列已满吧,不再继续添加任务
        ez_mutex_unlock(jobqueue_p->rwmutex);
        return -1;
    }
    switch (jobqueue_p->len)
    {

    case 0: /* if no jobs in queue */
        jobqueue_p->front = newjob;
        jobqueue_p->rear = newjob;
        break;

    default: /* if jobs in queue */
        jobqueue_p->rear->prev = newjob;
        jobqueue_p->rear = newjob;
    }
    count = ++jobqueue_p->len;

    ezsem_post(jobqueue_p->has_jobs);
    ez_mutex_unlock(jobqueue_p->rwmutex);

    return count;
}

/* Get first job from queue(removes it from queue)
 *
 * Notice: Caller MUST hold a mutex
 */
static struct job *jobqueue_pull(jobqueue *jobqueue_p)
{
    ez_mutex_lock(jobqueue_p->rwmutex);
    job *job_p = jobqueue_p->front;

    switch (jobqueue_p->len)
    {

    case 0: /* if no jobs in queue */
        break;

    case 1: /* if one job in queue */
        jobqueue_p->front = NULL;
        jobqueue_p->rear = NULL;
        jobqueue_p->len = 0;
        break;

    default: /* if >1 jobs in queue */
        jobqueue_p->front = job_p->prev;
        jobqueue_p->len--;
        /* more than one job in queue -> post it */
        ezsem_post(jobqueue_p->has_jobs);
    }

    ez_mutex_unlock(jobqueue_p->rwmutex);
    return job_p;
}

/* Free all queue resources back to the system */
static void jobqueue_destroy(jobqueue *jobqueue_p)
{
    jobqueue_clear(jobqueue_p);

    jobqueue_p->front = NULL;
    jobqueue_p->rear = NULL;
    ezsem_final(jobqueue_p->has_jobs);
    jobqueue_p->has_jobs = NULL;
    jobqueue_p->len = 0;
}
