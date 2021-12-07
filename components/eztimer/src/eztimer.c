#include "eztimer.h"
#include "ezos_thread.h"
#include "ezos_time.h"
#include "ezos_libc.h"
#include "ezlist.h"
#include "ezos_gconfig.h"

typedef struct
{
    ez_int8_t   name[32];       // 可以重复
    ez_int32_t  id;             // 输出,唯一标示这个timer_struct ,不可以重复
    ez_bool_t    reload;         // 可重复触发
    ez_int32_t  time_out;       // 超时时间ms
    ez_int32_t  count_down;     // 超时时间/100
    ez_int32_t  (*fun)(void);   // 超时后的回调
} eztimer_struct;

typedef struct timer_list
{
    list_t    *list;             //eztimer_struct
    ez_int32_t  msec;              //轮询时间，默认100ms
    ez_bool_t   invalid;
    void        *thread;
} timer_list;

static void *g_status_mutex = NULL;

static timer_list g_list;

//引用计数
static ez_int32_t g_instance_count = 0;

static void timer_routine(void *param)
{
    while (g_list.invalid)
    {
        ezos_mutex_lock(g_status_mutex);
        ez_int32_t list_size = ezlist_size(g_list.list);
        ez_size_t node_size = sizeof(eztimer_struct);
        for (ez_size_t i = 0; i < list_size; i++)
        {
            eztimer_struct *node = ezlist_getat(g_list.list, i, &node_size, ez_false);
            if (node != NULL)
            {
                if (0 == node->count_down)
                {
                    if (node->fun != NULL)
                    {
                        node->fun();
                    }

                    if (!node->reload)
                    {
                        ezlist_removeat(g_list.list, i);
                        break;
                    }
                    else
                    {
                        node->count_down = node->time_out / 100;
                    }

                }
                if (0 <= node->count_down)
                {
                    node->count_down--;
                }
            }
        }
        ezos_mutex_unlock(g_status_mutex);

        ezos_delay_ms(g_list.msec);
    }
}

ez_int32_t ez_timer_init(void)
{
    g_status_mutex = ezos_mutex_create();
    if (NULL == g_status_mutex)
    {
        return -1;
    }

    ezos_mutex_lock(g_status_mutex);
    if (g_instance_count++ > 0)
    {
        ezos_mutex_unlock(g_status_mutex);
        return 0;
    }

    ezos_memset(&g_list, 0, sizeof(g_list));
    ezos_thread_create(&g_list.thread, (ez_int8_t *)"ez_timer", timer_routine, (void *)&g_list, CONFIG_EZIOT_CONPONENT_TIMER_TASK_STACK_SIZE, 13);
    if (NULL == g_list.thread)
    {
        ezos_mutex_unlock(g_status_mutex);
        return -1;
    }
    g_list.invalid = ez_true;
    g_list.msec = 100;
    g_list.list = ezlist(ezlist_THREADSAFE);
    ezos_mutex_unlock(g_status_mutex);
    return 0;
}

ez_int32_t ez_timer_fini(void)
{
    ezos_mutex_lock(g_status_mutex);
    if (--g_instance_count > 0)
    {
        ezos_mutex_unlock(g_status_mutex);
        return 0;
    }

    if (g_list.invalid)
    {
        ez_int32_t list_size = ezlist_size(g_list.list);
        if (0 == list_size)
        {
            g_list.invalid = ez_false;
            ezos_thread_destroy(g_list.thread);
            g_list.thread = NULL;

            ezlist_free(g_list.list);
        }
    }

    ezos_mutex_unlock(g_status_mutex);

    return 0;
}

void *ez_timer_create(ez_int8_t *name, ez_int32_t time_out, ez_bool_t reload, ez_int32_t (*fun)(void))
{
    static ez_size_t id = 0;

    if (ez_false == g_list.invalid)
    {
        if (0 != ez_timer_init())
        {
            return NULL;
        }
    }

    eztimer_struct timer = {0};
    ezos_strncpy(timer.name, name, sizeof(timer.name) - 1);
    timer.id = ++id;
    timer.fun = fun;
    timer.time_out = time_out;
    timer.reload = reload;
    timer.count_down = time_out / g_list.msec;

    ezos_mutex_lock(g_status_mutex);
    ezlist_addlast(g_list.list, (void *)&timer, sizeof(eztimer_struct));
    ezos_mutex_unlock(g_status_mutex);

    return (void *)timer.id;
}

ez_int32_t ez_timer_delete(void *handle)
{
    if (ez_false == g_list.invalid)
    {
        return -1;
    }

    if (NULL == handle)
    {
        return -1;
    }

    ez_size_t id = (ez_size_t)handle;

    ezos_mutex_lock(g_status_mutex);
    ez_size_t list_size = ezlist_size(g_list.list);
    ez_size_t node_size = sizeof(eztimer_struct);
    for (ez_size_t i = 0; i < list_size; i++)
    {
        eztimer_struct *timer = ezlist_getat(g_list.list, i, &node_size, ez_false);
        if (id == timer->id)
        {
            ezlist_removeat(g_list.list, i);
            break;
        }
    }
    ezos_mutex_unlock(g_status_mutex);

    if (1 == list_size)
    {
        ez_timer_fini();
    }
    return 0;
}

ez_int32_t ez_timer_reset(void *handle)
{
    if (ez_false == g_list.invalid)
    {
        return -1;
    }

    if (NULL == handle)
    {
        return -1;
    }

    ez_size_t id = (ez_size_t)handle;

    ezos_mutex_lock(g_status_mutex);
    ez_size_t list_size = ezlist_size(g_list.list);
    ez_size_t node_size = sizeof(eztimer_struct);
    for (ez_size_t i = 0; i < list_size; i++)
    {
        eztimer_struct *timer = ezlist_getat(g_list.list, i, &node_size, ez_false);
        if (id == timer->id)
        {
            timer->count_down = timer->time_out / g_list.msec;
            break;
        }
    }
    ezos_mutex_unlock(g_status_mutex);
    return 0;
}

ez_int32_t ez_timer_change_period(void *handle, ez_int32_t new_time_out)
{
    if (ez_false == g_list.invalid)
    {
        return -1;
    }

    if (NULL == handle)
    {
        return -1;
    }

    ez_size_t id = (ez_size_t)handle;

    ezos_mutex_lock(g_status_mutex);
    ez_size_t list_size = ezlist_size(g_list.list);
    ez_size_t node_size = sizeof(eztimer_struct);
    for (ez_size_t i = 0; i < list_size; i++)
    {
        eztimer_struct *timer = ezlist_getat(g_list.list, i, &node_size, ez_false);
        if (id == timer->id)
        {
            timer->time_out = new_time_out;
            if (timer->count_down >= new_time_out / g_list.msec)
            {
                timer->count_down = new_time_out / g_list.msec;
            }
            else
            {
                timer->count_down += (new_time_out - timer->time_out) / g_list.msec;
            }
            break;
        }
    }
    ezos_mutex_unlock(g_status_mutex);
    return 0;
}