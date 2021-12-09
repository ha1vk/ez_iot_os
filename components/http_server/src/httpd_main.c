// Copyright 2018 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <sys/param.h>
#include <sys/errno.h>
#include <assert.h>

#include "http_server.h"
#include "httpd_priv.h"
#include "ctrl_sock.h"
#include "ezlog.h"
#include "ezos.h"
#include "ezos_time.h"

static const char *TAG = "httpd";

static ez_err_t httpd_accept_conn(struct httpd_data *hd, ez_int32_t listen_fd)
{
    /* If no space is available for new session, ezos_closesocket the least recently used one */
    if (hd->config.lru_purge_enable == true)
    {
        if (!httpd_is_sess_available(hd))
        {
            /* Queue asynchronous closure of the least recently used session */
            return httpd_sess_close_lru(hd);
            /* Returning from this allowes the main server thread to process
             * the queued asynchronous control message for closing LRU session.
             * Since connection request hasn't been addressed yet using accept()
             * therefore httpd_accept_conn() will be called again, but this time
             * with space available for one session
             */
        }
    }

    struct ez_sockaddr_in addr_from;
    ez_socklen_t addr_from_len = sizeof(addr_from);
    int new_fd = ezos_accept(listen_fd, (struct ez_sockaddr *)&addr_from, &addr_from_len);
    if (new_fd < 0)
    {
        ezlog_w(TAG, LOG_FMT("error in accept (%d)"), errno);
        return EZHTTPD_ERRNO_FAIL;
    }
    ezlog_v(TAG, LOG_FMT("newfd = %d"), new_fd);

    int rv = EZHTTPD_ERRNO_SUCC;
    ezos_timeval_t tv;
    /* Set recv timeout of this fd as per config */
    tv.tv_sec = hd->config.recv_wait_timeout;
    tv.tv_usec = 0;
    rv |= ezos_setsockopt(new_fd, EZ_SOL_SOCKET, EZ_SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    /* Set send timeout of this fd as per config */
    tv.tv_sec = hd->config.send_wait_timeout;
    tv.tv_usec = 0;
    rv |= ezos_setsockopt(new_fd, EZ_SOL_SOCKET, EZ_SO_SNDTIMEO, (const char *)&tv, sizeof(tv));
    
    if (EZHTTPD_ERRNO_SUCC != rv)
    {
        ezlog_w(TAG, "set socket option failed.");
        ezos_closesocket(new_fd);
        return EZHTTPD_ERRNO_FAIL;
    }

    if (EZHTTPD_ERRNO_SUCC != httpd_sess_new(hd, new_fd))
    {
        ezlog_w(TAG, LOG_FMT("session creation failed"));
        ezos_closesocket(new_fd);
        return EZHTTPD_ERRNO_FAIL;
    }
    ezlog_v(TAG, LOG_FMT("complete"));
    return EZHTTPD_ERRNO_SUCC;
}

struct httpd_ctrl_data
{
    enum httpd_ctrl_msg
    {
        HTTPD_CTRL_SHUTDOWN,
        HTTPD_CTRL_WORK,
    } hc_msg;
    httpd_work_fn_t hc_work;
    void *hc_work_arg;
};

ez_err_t httpd_queue_work(httpd_handle_t handle, httpd_work_fn_t work, void *arg)
{
    if (handle == NULL || work == NULL)
    {
        return EZHTTPD_ERRNO_FAIL;
    }

    struct httpd_data *hd = (struct httpd_data *)handle;
    struct httpd_ctrl_data msg = {
        .hc_msg = HTTPD_CTRL_WORK,
        .hc_work = work,
        .hc_work_arg = arg,
    };

    int ret = cs_send_to_ctrl_sock(hd->msg_fd, hd->config.ctrl_port, &msg, sizeof(msg));
    if (ret < 0)
    {
        ezlog_w(TAG, LOG_FMT("failed to queue work"));
        return EZHTTPD_ERRNO_FAIL;
    }

    return EZHTTPD_ERRNO_SUCC;
}

void *httpd_get_global_user_ctx(httpd_handle_t handle)
{
    return ((struct httpd_data *)handle)->config.global_user_ctx;
}

void *httpd_get_global_transport_ctx(httpd_handle_t handle)
{
    return ((struct httpd_data *)handle)->config.global_transport_ctx;
}

static void httpd_close_all_sessions(struct httpd_data *hd)
{
    int fd = -1;
    while ((fd = httpd_sess_iterate(hd, fd)) != -1)
    {
        ezlog_v(TAG, LOG_FMT("cleaning up ezos_socket %d"), fd);
        httpd_sess_delete(hd, fd);
        ezos_closesocket(fd);
    }
}

static void httpd_process_ctrl_msg(struct httpd_data *hd)
{
    struct httpd_ctrl_data msg;
    int ret = ezos_recv(hd->ctrl_fd, &msg, sizeof(msg), 0);
    if (ret <= 0)
    {
        ezlog_w(TAG, LOG_FMT("error in recv (%d)"), errno);
        return;
    }
    if (ret != sizeof(msg))
    {
        ezlog_w(TAG, LOG_FMT("incomplete msg"));
        return;
    }

    switch (msg.hc_msg)
    {
    case HTTPD_CTRL_WORK:
        if (msg.hc_work)
        {
            ezlog_v(TAG, LOG_FMT("work"));
            (*msg.hc_work)(msg.hc_work_arg);
        }
        break;
    case HTTPD_CTRL_SHUTDOWN:
        ezlog_v(TAG, LOG_FMT("shutdown"));
        hd->hd_td.status = THREAD_STOPPING;
        break;
    default:
        break;
    }
}

/* Manage in-coming connection or data requests */
static ez_err_t httpd_server(struct httpd_data *hd)
{
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(hd->listen_fd, &read_set);
    FD_SET(hd->ctrl_fd, &read_set);

    int tmp_max_fd;
    httpd_sess_set_descriptors(hd, &read_set, &tmp_max_fd);
    int maxfd = MAX(hd->listen_fd, tmp_max_fd);
    tmp_max_fd = maxfd;
    maxfd = MAX(hd->ctrl_fd, tmp_max_fd);

    ezlog_v(TAG, LOG_FMT("doing select maxfd+1 = %d"), maxfd + 1);
    int active_cnt = select(maxfd + 1, &read_set, NULL, NULL, NULL);
    if (active_cnt < 0)
    {
        ezlog_e(TAG, LOG_FMT("error in select (%d)"), errno);
        httpd_sess_delete_invalid(hd);
        return EZHTTPD_ERRNO_SUCC;
    }

    /* Case0: Do we have a control message? */
    if (FD_ISSET(hd->ctrl_fd, &read_set))
    {
        ezlog_v(TAG, LOG_FMT("processing ctrl message"));
        httpd_process_ctrl_msg(hd);
        if (hd->hd_td.status == THREAD_STOPPING)
        {
            ezlog_v(TAG, LOG_FMT("stopping thread"));
            return EZHTTPD_ERRNO_FAIL;
        }
    }

    /* Case1: Do we have any activity on the current data
     * sessions? */
    int fd = -1;
    while ((fd = httpd_sess_iterate(hd, fd)) != -1)
    {
        if (FD_ISSET(fd, &read_set) || (httpd_sess_pending(hd, fd)))
        {
            ezlog_v(TAG, LOG_FMT("processing ezos_socket %d"), fd);
            if (httpd_sess_process(hd, fd) != EZHTTPD_ERRNO_SUCC)
            {
                ezlog_v(TAG, LOG_FMT("closing ezos_socket %d"), fd);
                ezos_closesocket(fd);
                /* Delete session and update fd to that
                 * preceding the one being deleted */
                fd = httpd_sess_delete(hd, fd);
            }
        }
    }

    /* Case2: Do we have any incoming connection requests to
     * process? */
    if (FD_ISSET(hd->listen_fd, &read_set))
    {
        ezlog_v(TAG, LOG_FMT("processing listen ezos_socket %d"), hd->listen_fd);
        if (httpd_accept_conn(hd, hd->listen_fd) != EZHTTPD_ERRNO_SUCC)
        {
            ezlog_w(TAG, LOG_FMT("error accepting new connection"));
        }
    }
    return EZHTTPD_ERRNO_SUCC;
}

/* The main HTTPD thread */
static void httpd_thread(void *arg)
{
    int ret;
    struct httpd_data *hd = (struct httpd_data *)arg;
    hd->hd_td.status = THREAD_RUNNING;

    ezlog_v(TAG, LOG_FMT("web server started"));
    while (1)
    {
        ret = httpd_server(hd);
        if (ret != EZHTTPD_ERRNO_SUCC)
        {
            break;
        }
    }

    ezlog_v(TAG, LOG_FMT("web server exiting"));
    ezos_closesocket(hd->msg_fd);
    cs_free_ctrl_sock(hd->ctrl_fd);
    httpd_close_all_sessions(hd);
    ezos_closesocket(hd->listen_fd);
    hd->hd_td.status = THREAD_STOPPED;
    httpd_os_thread_delete(hd->hd_td.handle);
}

static ez_err_t httpd_server_init(struct httpd_data *hd)
{
#ifdef CONFIG_LWIP_IPV6
    int fd = ezos_socket(PF_INET6, SOCK_STREAM, 0);
#else
    int fd = ezos_socket(EZ_PF_INET, EZ_SOCK_STREAM, 0);
#endif /* CONFIG_LWIP_IPV6 */
    if (fd < 0)
    {
        ezlog_e(TAG, LOG_FMT("error in ezos_socket (%d)"), errno);
        return EZHTTPD_ERRNO_FAIL;
    }

#ifdef CONFIG_LWIP_IPV6
    struct in6_addr inaddr_any = IN6ADDR_ANY_INIT;
    struct sockaddr_in6 serv_addr = {
        .sin6_family = PF_INET6,
        .sin6_addr = inaddr_any,
        .sin6_port = htons(hd->config.server_port)};
#else
    struct ez_sockaddr_in serv_addr = {
        .sin_family = EZ_AF_INET,
        .sin_addr = {
            .s_addr = ezos_htonl(EZ_INADDR_ANY)},
        .sin_port = ezos_htons(hd->config.server_port)};
#endif /* CONFIG_LWIP_IPV6 */

    int ret = ezos_bind(fd, (struct ez_sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        ezlog_e(TAG, LOG_FMT("error in bind (%d)"), errno);
        ezos_closesocket(fd);
        return EZHTTPD_ERRNO_FAIL;
    }

    ret = ezos_listen(fd, hd->config.backlog_conn);
    if (ret < 0)
    {
        ezlog_e(TAG, LOG_FMT("error in listen (%d)"), errno);
        ezos_closesocket(fd);
        return EZHTTPD_ERRNO_FAIL;
    }

    int ctrl_fd = cs_create_ctrl_sock(hd->config.ctrl_port);
    if (ctrl_fd < 0)
    {
        ezlog_e(TAG, LOG_FMT("error in creating ctrl ezos_socket (%d)"), errno);
        ezos_closesocket(fd);
        return EZHTTPD_ERRNO_FAIL;
    }

    int msg_fd = ezos_socket(EZ_AF_INET, EZ_SOCK_DGRAM, EZ_IPPROTO_UDP);
    if (msg_fd < 0)
    {
        ezlog_e(TAG, LOG_FMT("error in creating msg ezos_socket (%d)"), errno);
        ezos_closesocket(fd);
        ezos_closesocket(ctrl_fd);
        return EZHTTPD_ERRNO_FAIL;
    }

    hd->listen_fd = fd;
    hd->ctrl_fd = ctrl_fd;
    hd->msg_fd = msg_fd;
    return EZHTTPD_ERRNO_SUCC;
}

static struct httpd_data *httpd_create(const httpd_config_t *config)
{
    /* Allocate memory for httpd instance data */
    struct httpd_data *hd = ezos_calloc(1, sizeof(struct httpd_data));
    if (hd != NULL)
    {   
        hd->hd_calls = ezos_calloc(config->max_uri_handlers, sizeof(httpd_uri_t *));
        if (hd->hd_calls == NULL)
        {
            ezos_free(hd);
            return NULL;
        }
        hd->hd_sd = ezos_calloc(config->max_open_sockets, sizeof(struct sock_db));
        if (hd->hd_sd == NULL)
        {
            ezos_free(hd->hd_calls);
            ezos_free(hd);
            return NULL;
        }
        struct httpd_req_aux *ra = &hd->hd_req_aux;
        ra->resp_hdrs = ezos_calloc(config->max_resp_headers, sizeof(struct resp_hdr));
        if (ra->resp_hdrs == NULL)
        {
            ezos_free(hd->hd_sd);
            ezos_free(hd->hd_calls);
            ezos_free(hd);
            return NULL;
        }
        /* Save the configuration for this instance */
        hd->config = *config;
    }
    else
    {
        ezlog_e(TAG, "mem alloc failed");
    }
    return hd;
}

static void httpd_delete(struct httpd_data *hd)
{
    struct httpd_req_aux *ra = &hd->hd_req_aux;
    /* ezos_free memory of httpd instance data */
    ezos_free(ra->resp_hdrs);
    ezos_free(hd->hd_sd);

    /* ezos_free registered URI handlers */
    httpd_unregister_all_uri_handlers(hd);
    ezos_free(hd->hd_calls);
    ezos_free(hd);
}

ez_err_t httpd_start(httpd_handle_t *handle, const httpd_config_t *config)
{
    if (handle == NULL || config == NULL)
    {
        return EZHTTPD_ERRNO_FAIL;
    }

    struct httpd_data *hd = httpd_create(config);
    if (hd == NULL)
    {
        /* Failed to allocate memory */
        
        return EZHTTPD_ERRNO_ALLOC_MEM;
    }

    if (httpd_server_init(hd) != EZHTTPD_ERRNO_SUCC)
    {
        httpd_delete(hd);
        return EZHTTPD_ERRNO_FAIL;
    }

    httpd_sess_init(hd);
    if (httpd_os_thread_create(&hd->hd_td.handle, "httpd",
                               hd->config.stack_size,
                               hd->config.task_priority,
                               httpd_thread, hd) != EZHTTPD_ERRNO_SUCC)
    {
        /* Failed to launch task */
        httpd_delete(hd);
       
        return EZHTTPD_ERRNO_TASK;
    }

    *handle = (httpd_handle_t *)hd;
    return EZHTTPD_ERRNO_SUCC;
}

ez_err_t httpd_stop(httpd_handle_t handle)
{
    struct httpd_data *hd = (struct httpd_data *)handle;
    if (hd == NULL)
    {
        return EZHTTPD_ERRNO_FAIL;
    }

    struct httpd_ctrl_data msg;
    ezos_memset(&msg, 0, sizeof(msg));
    msg.hc_msg = HTTPD_CTRL_SHUTDOWN;
    cs_send_to_ctrl_sock(hd->msg_fd, hd->config.ctrl_port, &msg, sizeof(msg));

    ezlog_v(TAG, LOG_FMT("sent control msg to stop server"));
    while (hd->hd_td.status != THREAD_STOPPED)
    {
        httpd_os_thread_sleep(100);
    }

    /* Release global user context, if not NULL */
    if (hd->config.global_user_ctx)
    {
        if (hd->config.global_user_ctx_free_fn)
        {
            hd->config.global_user_ctx_free_fn(hd->config.global_user_ctx);
        }
        else
        {
            ezos_free(hd->config.global_user_ctx);
        }
        hd->config.global_user_ctx = NULL;
    }

    /* Release global transport context, if not NULL */
    if (hd->config.global_transport_ctx)
    {
        if (hd->config.global_transport_ctx_free_fn)
        {
            hd->config.global_transport_ctx_free_fn(hd->config.global_transport_ctx);
        }
        else
        {
            ezos_free(hd->config.global_transport_ctx);
        }
        hd->config.global_transport_ctx = NULL;
    }

    ezlog_v(TAG, LOG_FMT("server stopped"));
    httpd_delete(hd);
    return EZHTTPD_ERRNO_SUCC;
}
