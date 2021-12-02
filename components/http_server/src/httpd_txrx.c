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

#include <sys/errno.h>

#include <http_server.h>
#include "httpd_priv.h"

#include "ezlog.h"
#include "ezos_libc.h"
#include "ezos_mem.h"

static const char *TAG = "httpd_txrx";

ez_err_t httpd_sess_set_send_override(httpd_handle_t hd, ez_int32_t sockfd, httpd_send_func_t send_func)
{
    struct sock_db *sess = httpd_sess_get(hd, sockfd);
    if (!sess)
    {
        return EZHTTPD_ERRNO_FAIL;
    }
    sess->send_fn = send_func;
    return EZHTTPD_ERRNO_SUCC;
}

ez_err_t httpd_sess_set_recv_override(httpd_handle_t hd, ez_int32_t sockfd, httpd_recv_func_t recv_func)
{
    struct sock_db *sess = httpd_sess_get(hd, sockfd);
    if (!sess)
    {
        return EZHTTPD_ERRNO_FAIL;
    }
    sess->recv_fn = recv_func;
    return EZHTTPD_ERRNO_SUCC;
}

ez_err_t httpd_sess_set_pending_override(httpd_handle_t hd, ez_int32_t sockfd, httpd_pending_func_t pending_func)
{
    struct sock_db *sess = httpd_sess_get(hd, sockfd);
    if (!sess)
    {
        return EZHTTPD_ERRNO_FAIL;
    }
    sess->pending_fn = pending_func;
    return EZHTTPD_ERRNO_SUCC;
}

ez_int32_t httpd_send(httpd_req_t *r, const ez_int8_t *buf, ez_size_t buf_len)
{
    if (r == NULL || buf == NULL)
    {
        return HTTPD_SOCK_ERR_INVALID;
    }

    if (!httpd_valid_req(r))
    {
        return HTTPD_SOCK_ERR_INVALID;
    }

    struct httpd_req_aux *ra = r->aux;
    ez_int32_t ret = ra->sd->send_fn(ra->sd->handle, ra->sd->fd, buf, buf_len, 0);
    if (ret < 0)
    {
        ezlog_v(TAG, LOG_FMT("error in send_fn"));
        return ret;
    }
    return ret;
}

static ez_err_t httpd_send_all(httpd_req_t *r, const ez_int8_t *buf, ez_size_t buf_len)
{
    struct httpd_req_aux *ra = r->aux;
    ez_int32_t ret;

    while (buf_len > 0)
    {
        ret = ra->sd->send_fn(ra->sd->handle, ra->sd->fd, buf, buf_len, 0);
        if (ret < 0)
        {
            ezlog_v(TAG, LOG_FMT("error in send_fn"));
            return EZHTTPD_ERRNO_FAIL;
        }
        ezlog_v(TAG, LOG_FMT("sent = %d"), ret);
        buf += ret;
        buf_len -= ret;
    }
    return EZHTTPD_ERRNO_SUCC;
}

static ez_size_t httpd_recv_pending(httpd_req_t *r, ez_int8_t *buf, ez_size_t buf_len)
{
    struct httpd_req_aux *ra = r->aux;
    size_t offset = sizeof(ra->sd->pending_data) - ra->sd->pending_len;

    /* buf_len must not be greater than remaining_len */
    buf_len = MIN(ra->sd->pending_len, buf_len);
    ezos_memcpy(buf, ra->sd->pending_data + offset, buf_len);

    ra->sd->pending_len -= buf_len;
    return buf_len;
}

ez_int32_t httpd_recv_with_opt(httpd_req_t *r, ez_int8_t *buf, ez_size_t buf_len, ez_bool_t halt_after_pending)
{
    ezlog_v(TAG, LOG_FMT("requested length = %d"), buf_len);

    ez_size_t pending_len = 0;
    struct httpd_req_aux *ra = r->aux;

    /* First fetch pending data from local buffer */
    if (ra->sd->pending_len > 0)
    {
        ezlog_v(TAG, LOG_FMT("pending length = %d"), ra->sd->pending_len);
        pending_len = httpd_recv_pending(r, buf, buf_len);
        buf += pending_len;
        buf_len -= pending_len;

        /* If buffer filled then no need to recv.
         * If asked to halt after receiving pending data then
         * return with received length */
        if (!buf_len || halt_after_pending)
        {
            return pending_len;
        }
    }

    /* Receive data of remaining length */
    ez_int32_t ret = ra->sd->recv_fn(ra->sd->handle, ra->sd->fd, buf, buf_len, 0);
    if (ret < 0)
    {
        ezlog_v(TAG, LOG_FMT("error in recv_fn"));
        if ((ret == HTTPD_SOCK_ERR_TIMEOUT) && (pending_len != 0))
        {
            /* If recv() timeout occurred, but pending data is
             * present, return length of pending data.
             * This behavior is similar to that of socket recv()
             * function, which, in case has only partially read the
             * requested length, due to timeout, returns with read
             * length, rather than error */
            return pending_len;
        }
        return ret;
    }

    ezlog_v(TAG, LOG_FMT("received length = %d"), ret + pending_len);
    return ret + pending_len;
}

ez_int32_t httpd_recv(httpd_req_t *r, ez_int8_t *buf, ez_size_t buf_len)
{
    return httpd_recv_with_opt(r, buf, buf_len, false);
}

ez_size_t httpd_unrecv(struct httpd_req *r, const ez_int8_t *buf, ez_size_t buf_len)
{
    struct httpd_req_aux *ra = r->aux;
    /* Truncate if external buf_len is greater than pending_data buffer size */
    ra->sd->pending_len = MIN(sizeof(ra->sd->pending_data), buf_len);

    /* Copy data into internal pending_data buffer */
    ez_size_t offset = sizeof(ra->sd->pending_data) - ra->sd->pending_len;
    ezos_memcpy(ra->sd->pending_data + offset, buf, buf_len);
    ezlog_v(TAG, LOG_FMT("length = %d"), ra->sd->pending_len);
    return ra->sd->pending_len;
}

/**
 * This API appends an additional header field-value pair in the HTTP response.
 * But the header isn't sent out until any of the send APIs is executed.
 */
ez_err_t httpd_resp_set_hdr(httpd_req_t *r, const ez_int8_t *field, const ez_int8_t *value)
{
    if (r == NULL || field == NULL || value == NULL)
    {
        return EZHTTPD_ERRNO_FAIL;
    }

    if (!httpd_valid_req(r))
    {
        return EZHTTPD_ERRNO_INVALID_REQ;
    }

    struct httpd_req_aux *ra = r->aux;
    struct httpd_data *hd = (struct httpd_data *)r->handle;

    /* Number of additional headers is limited */
    if (ra->resp_hdrs_count >= hd->config.max_resp_headers)
    {
        return EZHTTPD_ERRNO_RESP_HDR;
    }

    /* Assign header field-value pair */
    ra->resp_hdrs[ra->resp_hdrs_count].field = field;
    ra->resp_hdrs[ra->resp_hdrs_count].value = value;
    ra->resp_hdrs_count++;

    ezlog_v(TAG, LOG_FMT("new header = %s: %s"), field, value);
    return EZHTTPD_ERRNO_SUCC;
}

/**
 * This API sets the status of the HTTP response to the value specified.
 * But the status isn't sent out until any of the send APIs is executed.
 */
ez_err_t httpd_resp_set_status(httpd_req_t *r, const ez_int8_t *status)
{
    if (r == NULL || status == NULL)
    {
        return EZHTTPD_ERRNO_FAIL;
    }

    if (!httpd_valid_req(r))
    {
        return EZHTTPD_ERRNO_INVALID_REQ;
    }

    struct httpd_req_aux *ra = r->aux;
    ra->status = (char *)status;
    return EZHTTPD_ERRNO_SUCC;
}

/**
 * This API sets the method/type of the HTTP response to the value specified.
 * But the method isn't sent out until any of the send APIs is executed.
 */
ez_err_t httpd_resp_set_type(httpd_req_t *r, const ez_int8_t *type)
{
    if (r == NULL || type == NULL)
    {
        return EZHTTPD_ERRNO_FAIL;
    }

    if (!httpd_valid_req(r))
    {
        return EZHTTPD_ERRNO_INVALID_REQ;
    }

    struct httpd_req_aux *ra = r->aux;
    ra->content_type = (char *)type;
    return EZHTTPD_ERRNO_SUCC;
}

ez_err_t httpd_resp_send(httpd_req_t *r, const ez_int8_t *buf, ez_ssize_t buf_len)
{
    if (r == NULL)
    {
        return EZHTTPD_ERRNO_FAIL;
    }

    if (!httpd_valid_req(r))
    {
        return EZHTTPD_ERRNO_INVALID_REQ;
    }

    struct httpd_req_aux *ra = r->aux;
    const char *httpd_hdr_str = "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n";
    const char *colon_separator = ": ";
    const char *cr_lf_seperator = "\r\n";

    if (buf_len == -1)
        buf_len = ezos_strlen(buf);

    /* Request headers are no longer available */
    ra->req_hdrs_count = 0;

    /* Size of essential headers is limited by scratch buffer size */
    if (ezos_snprintf(ra->scratch, sizeof(ra->scratch), httpd_hdr_str,
                 ra->status, ra->content_type, buf_len) >= sizeof(ra->scratch))
    {
        return EZHTTPD_ERRNO_RESP_HDR;
    }

    /* Sending essential headers */
    if (httpd_send_all(r, ra->scratch, ezos_strlen(ra->scratch)) != EZHTTPD_ERRNO_SUCC)
    {
        return EZHTTPD_ERRNO_RESP_SEND;
    }

    /* Sending additional headers based on set_header */
    for (unsigned i = 0; i < ra->resp_hdrs_count; i++)
    {
        /* Send header field */
        if (httpd_send_all(r, ra->resp_hdrs[i].field, ezos_strlen(ra->resp_hdrs[i].field)) != EZHTTPD_ERRNO_SUCC)
        {
            return EZHTTPD_ERRNO_RESP_SEND;
        }
        /* Send ': ' */
        if (httpd_send_all(r, colon_separator, ezos_strlen(colon_separator)) != EZHTTPD_ERRNO_SUCC)
        {
            return EZHTTPD_ERRNO_RESP_SEND;
        }
        /* Send header value */
        if (httpd_send_all(r, ra->resp_hdrs[i].value, ezos_strlen(ra->resp_hdrs[i].value)) != EZHTTPD_ERRNO_SUCC)
        {
            return EZHTTPD_ERRNO_RESP_SEND;
        }
        /* Send CR + LF */
        if (httpd_send_all(r, cr_lf_seperator, ezos_strlen(cr_lf_seperator)) != EZHTTPD_ERRNO_SUCC)
        {
            return EZHTTPD_ERRNO_RESP_SEND;
        }
    }

    /* End header section */
    if (httpd_send_all(r, cr_lf_seperator, ezos_strlen(cr_lf_seperator)) != EZHTTPD_ERRNO_SUCC)
    {
        return EZHTTPD_ERRNO_RESP_SEND;
    }

    /* Sending content */
    if (buf && buf_len)
    {
        if (httpd_send_all(r, buf, buf_len) != EZHTTPD_ERRNO_SUCC)
        {
            return EZHTTPD_ERRNO_RESP_SEND;
        }
    }
    return EZHTTPD_ERRNO_SUCC;
}

ez_err_t httpd_resp_send_chunk(httpd_req_t *r, const ez_int8_t *buf, ez_ssize_t buf_len)
{
    if (r == NULL)
    {
        return EZHTTPD_ERRNO_FAIL;
    }

    if (!httpd_valid_req(r))
    {
        return EZHTTPD_ERRNO_INVALID_REQ;
    }

    if (buf_len == -1)
        buf_len = ezos_strlen(buf);

    struct httpd_req_aux *ra = r->aux;
    const char *httpd_chunked_hdr_str = "HTTP/1.1 %s\r\nContent-Type: %s\r\nTransfer-Encoding: chunked\r\n";
    const char *colon_separator = ": ";
    const char *cr_lf_seperator = "\r\n";

    /* Request headers are no longer available */
    ra->req_hdrs_count = 0;

    if (!ra->first_chunk_sent)
    {
        /* Size of essential headers is limited by scratch buffer size */
        if (ezos_snprintf(ra->scratch, sizeof(ra->scratch), httpd_chunked_hdr_str,
                     ra->status, ra->content_type) >= sizeof(ra->scratch))
        {
            return EZHTTPD_ERRNO_RESP_HDR;
        }

        /* Sending essential headers */
        if (httpd_send_all(r, ra->scratch, ezos_strlen(ra->scratch)) != EZHTTPD_ERRNO_SUCC)
        {
            return EZHTTPD_ERRNO_RESP_SEND;
        }

        /* Sending additional headers based on set_header */
        for (unsigned i = 0; i < ra->resp_hdrs_count; i++)
        {
            /* Send header field */
            if (httpd_send_all(r, ra->resp_hdrs[i].field, ezos_strlen(ra->resp_hdrs[i].field)) != EZHTTPD_ERRNO_SUCC)
            {
                return EZHTTPD_ERRNO_RESP_SEND;
            }
            /* Send ': ' */
            if (httpd_send_all(r, colon_separator, ezos_strlen(colon_separator)) != EZHTTPD_ERRNO_SUCC)
            {
                return EZHTTPD_ERRNO_RESP_SEND;
            }
            /* Send header value */
            if (httpd_send_all(r, ra->resp_hdrs[i].value, ezos_strlen(ra->resp_hdrs[i].value)) != EZHTTPD_ERRNO_SUCC)
            {
                return EZHTTPD_ERRNO_RESP_SEND;
            }
            /* Send CR + LF */
            if (httpd_send_all(r, cr_lf_seperator, ezos_strlen(cr_lf_seperator)) != EZHTTPD_ERRNO_SUCC)
            {
                return EZHTTPD_ERRNO_RESP_SEND;
            }
        }

        /* End header section */
        if (httpd_send_all(r, cr_lf_seperator, ezos_strlen(cr_lf_seperator)) != EZHTTPD_ERRNO_SUCC)
        {
            return EZHTTPD_ERRNO_RESP_SEND;
        }
        ra->first_chunk_sent = true;
    }

    /* Sending chunked content */
    char len_str[10];
    ezos_snprintf(len_str, sizeof(len_str), "%x\r\n", buf_len);
    if (httpd_send_all(r, len_str, ezos_strlen(len_str)) != EZHTTPD_ERRNO_SUCC)
    {
        return EZHTTPD_ERRNO_RESP_SEND;
    }

    if (buf)
    {
        if (httpd_send_all(r, buf, (size_t)buf_len) != EZHTTPD_ERRNO_SUCC)
        {
            return EZHTTPD_ERRNO_RESP_SEND;
        }
    }

    /* Indicate end of chunk */
    if (httpd_send_all(r, "\r\n", ezos_strlen("\r\n")) != EZHTTPD_ERRNO_SUCC)
    {
        return EZHTTPD_ERRNO_RESP_SEND;
    }
    return EZHTTPD_ERRNO_SUCC;
}

ez_err_t httpd_resp_send_404(httpd_req_t *r)
{
    return httpd_resp_send_err(r, HTTPD_404_NOT_FOUND);
}

ez_err_t httpd_resp_send_408(httpd_req_t *r)
{
    return httpd_resp_send_err(r, HTTPD_408_REQ_TIMEOUT);
}

ez_err_t httpd_resp_send_500(httpd_req_t *r)
{
    return httpd_resp_send_err(r, HTTPD_500_SERVER_ERROR);
}

ez_err_t httpd_resp_send_err(httpd_req_t *req, httpd_err_resp_t error)
{
    const char *msg;
    const char *status;
    switch (error)
    {
    case HTTPD_501_METHOD_NOT_IMPLEMENTED:
        status = "501 Method Not Implemented";
        msg = "Request method is not supported by server";
        break;
    case HTTPD_505_VERSION_NOT_SUPPORTED:
        status = "505 Version Not Supported";
        msg = "HTTP version not supported by server";
        break;
    case HTTPD_400_BAD_REQUEST:
        status = "400 Bad Request";
        msg = "Server unable to understand request due to invalid syntax";
        break;
    case HTTPD_404_NOT_FOUND:
        status = "404 Not Found";
        msg = "This URI doesn't exist";
        break;
    case HTTPD_405_METHOD_NOT_ALLOWED:
        status = "405 Method Not Allowed";
        msg = "Request method for this URI is not handled by server";
        break;
    case HTTPD_408_REQ_TIMEOUT:
        status = "408 Request Timeout";
        msg = "Server closed this connection";
        break;
    case HTTPD_414_URI_TOO_LONG:
        status = "414 URI Too Long";
        msg = "URI is too long for server to interpret";
        break;
    case HTTPD_411_LENGTH_REQUIRED:
        status = "411 Length Required";
        msg = "Chunked encoding not supported by server";
        break;
    case HTTPD_431_REQ_HDR_FIELDS_TOO_LARGE:
        status = "431 Request Header Fields Too Large";
        msg = "Header fields are too long for server to interpret";
        break;
    case HTTPD_XXX_UPGRADE_NOT_SUPPORTED:
        /* If the server does not support upgrade, or is unable to upgrade
         * it responds with a standard HTTP/1.1 response */
        status = "200 OK";
        msg = "Upgrade not supported by server";
        break;
    case HTTPD_500_SERVER_ERROR:
    default:
        status = "500 Server Error";
        msg = "Server has encountered an unexpected error";
    }
    ezlog_w(TAG, LOG_FMT("%s - %s"), status, msg);

    httpd_resp_set_status(req, status);
    httpd_resp_set_type(req, HTTPD_TYPE_TEXT);
    return httpd_resp_send(req, msg, ezos_strlen(msg));
}

ez_int32_t httpd_req_recv(httpd_req_t *r, ez_int8_t *buf, ez_size_t buf_len)
{
    if (r == NULL || buf == NULL)
    {
        return HTTPD_SOCK_ERR_INVALID;
    }

    if (!httpd_valid_req(r))
    {
        ezlog_w(TAG, LOG_FMT("invalid request"));
        return HTTPD_SOCK_ERR_INVALID;
    }

    struct httpd_req_aux *ra = r->aux;
    ezlog_v(TAG, LOG_FMT("remaining length = %d"), ra->remaining_len);

    if (buf_len > ra->remaining_len)
    {
        buf_len = ra->remaining_len;
    }
    if (buf_len == 0)
    {
        return buf_len;
    }

    ez_int32_t ret = httpd_recv(r, buf, buf_len);
    if (ret < 0)
    {
        ezlog_v(TAG, LOG_FMT("error in httpd_recv"));
        return ret;
    }
    ra->remaining_len -= ret;
    ezlog_v(TAG, LOG_FMT("received length = %d"), ret);
    return ret;
}

ez_int32_t httpd_req_to_sockfd(httpd_req_t *r)
{
    if (r == NULL)
    {
        return -1;
    }

    if (!httpd_valid_req(r))
    {
        ezlog_w(TAG, LOG_FMT("invalid request"));
        return -1;
    }

    struct httpd_req_aux *ra = r->aux;
    return ra->sd->fd;
}

static int httpd_sock_err(const char *ctx, int sockfd)
{
    int errval;
    int sock_err;
    size_t sock_err_len = sizeof(sock_err);

    if (ezos_getsockopt(sockfd, EZ_SOL_SOCKET, EZ_SO_ERROR, &sock_err, &sock_err_len) < 0)
    {
        ezlog_e(TAG, LOG_FMT("error calling ezos_getsockopt : %d"), errno);
        return HTTPD_SOCK_ERR_FAIL;
    }
    ezlog_w(TAG, LOG_FMT("error in %s : %d"), ctx, sock_err);

    switch (sock_err)
    {
    case EAGAIN:
    case EINTR:
        errval = HTTPD_SOCK_ERR_TIMEOUT;
        break;
    case EINVAL:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        errval = HTTPD_SOCK_ERR_INVALID;
        break;
    default:
        errval = HTTPD_SOCK_ERR_FAIL;
    }
    return errval;
}

ez_int32_t httpd_default_send(httpd_handle_t hd, ez_int32_t sockfd, const ez_int8_t *buf, ez_size_t buf_len, ez_int32_t flags)
{
    (void)hd;
    if (buf == NULL)
    {
        return HTTPD_SOCK_ERR_INVALID;
    }

    ez_int32_t ret = ezos_send(sockfd, buf, buf_len, flags);
    if (ret < 0)
    {
        return httpd_sock_err("send", sockfd);
    }
    return ret;
}

ez_int32_t httpd_default_recv(httpd_handle_t hd, ez_int32_t sockfd, ez_int8_t *buf, ez_size_t buf_len, ez_int32_t flags)
{
    (void)hd;
    if (buf == NULL)
    {
        return HTTPD_SOCK_ERR_INVALID;
    }

    ez_int32_t ret = ezos_recv(sockfd, buf, buf_len, flags);
    if (ret < 0)
    {
        return httpd_sock_err("recv", sockfd);
    }
    return ret;
}
