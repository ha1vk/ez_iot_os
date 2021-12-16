/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-05-05     Bernard      the first version
 * 2013-06-10     Bernard      fix the slow speed issue when download file.
 * 2015-11-14     aozima       add content_length_remainder.
 * 2017-12-23     aozima       update gethostbyname to getaddrinfo.
 * 2018-01-04     aozima       add ipv6 address support.
 * 2018-07-26     chenyong     modify log information
 * 2018-08-07     chenyong     modify header processing
 * 2021-11-26     zoujinwei    modify function definition
 */

#include <ctype.h>

#include "webclient.h"
#include <ezlog.h>


#define TAG_WEB    "web"
#define DBG_ENABLE
#define DBG_SECTION_NAME "web"
#ifdef WEBCLIENT_DEBUG
#define DBG_LEVEL DBG_LOG
#else
#define DBG_LEVEL DBG_INFO
#endif /* WEBCLIENT_DEBUG */
#define DBG_COLOR
// #include <rtdbg.h>

/* default receive or send timeout */
#define WEBCLIENT_DEFAULT_TIMEO 6

extern long int strtol(const char *nptr, char **endptr, int base);

static int webclient_strncasecmp(const char *a, const char *b, size_t n)
{
    ez_uint8_t c1, c2;
    if (n <= 0)
        return 0;
    do
    {
        c1 = tolower(*a++);
        c2 = tolower(*b++);
    } while (--n && c1 && c1 == c2);
    return c1 - c2;
}

static const char *webclient_strstri(const char *str, const char *subStr)
{
    int len = ezos_strlen(subStr);

    if (len == 0)
    {
        return NULL;
    }

    while (*str)
    {
        if (webclient_strncasecmp(str, subStr, len) == 0)
        {
            return str;
        }
        ++str;
    }
    return NULL;
}

static int webclient_send(struct webclient_session *session, const void *buffer, size_t len, int flag)
{
#ifdef WEBCLIENT_USING_MBED_TLS
    if (session->tls_session)
    {
        return mbedtls_client_write(session->tls_session, buffer, len);
    }
#endif

    return ezos_send(session->socket, buffer, len, flag);
}

static int webclient_recv(struct webclient_session *session, void *buffer, size_t len, int flag)
{
#ifdef WEBCLIENT_USING_MBED_TLS
    if (session->tls_session)
    {
        return mbedtls_client_read(session->tls_session, buffer, len);
    }
#endif

    return ezos_recv(session->socket, buffer, len, flag);
}

static int webclient_read_line(struct webclient_session *session, char *buffer, int size)
{
    int rc, count = 0;
    char ch = 0, last_ch = 0;

    EZ_ASSERT(session);
    EZ_ASSERT(buffer);

    /* Keep reading until we fill the buffer. */
    while (count < size)
    {
        rc = webclient_recv(session, (unsigned char *)&ch, 1, 0);
#if defined(WEBCLIENT_USING_MBED_TLS) || defined(WEBCLIENT_USING_SAL_TLS)
        if (session->is_tls && (rc == MBEDTLS_ERR_SSL_WANT_READ || rc == MBEDTLS_ERR_SSL_WANT_WRITE))
        {
            continue;
        }
#endif
        if (rc <= 0)
            return rc;

        if (ch == '\n' && last_ch == '\r')
            break;

        buffer[count++] = ch;

        last_ch = ch;
    }

    if (count > size)
    {
        ezlog_e(TAG_WEB,"read line failed. The line data length is out of buffer size(%d)!\n", count);
        return -WEBCLIENT_ERROR;
    }

    return count;
}

/**
 * resolve server address
 *
 * @param session http session
 * @param res the server address information
 * @param url the input server URI address
 * @param request the pointer to point the request url, for example, /index.html
 *
 * @return 0 on resolve server address OK, others failed
 *
 * URL example:
 * http://www.rt-thread.org
 * http://www.rt-thread.org:80
 * https://www.rt-thread.org/
 * http://192.168.1.1:80/index.htm
 * http://[fe80::1]
 * http://[fe80::1]/
 * http://[fe80::1]/index.html
 * http://[fe80::1]:80/index.html
 */
static int webclient_resolve_address(struct webclient_session *session, ez_sockaddr_in_t **dst_addr,
                                     const char *url, const char **request)
{
    int rc = WEBCLIENT_OK;
    char *ptr;
    char port_str[6] = "80"; /* default port of 80(http) */
    unsigned short port = 80;
    const char *port_ptr;
    const char *path_ptr;
    ez_hostent_t *host;
    char ipstr[32];

    const char *host_addr = 0;
    int url_len, host_addr_len = 0;

    EZ_ASSERT(dst_addr);
    EZ_ASSERT(request);

    url_len = ezos_strlen(url);

    /* strip protocol(http or https) */
    if (ezos_strncmp(url, "http://", 7) == 0)
    {
        host_addr = url + 7;
    }
    else if (ezos_strncmp(url, "https://", 8) == 0)
    {
        ezos_strncpy(port_str, "443", 4);
        host_addr = url + 8;
    }
    else
    {
        rc = -WEBCLIENT_ERROR;
        goto __exit;
    }

    /* ipv6 address */
    if (host_addr[0] == '[')
    {
        host_addr += 1;
        ptr = ezos_strstr(host_addr, "]");
        if (!ptr)
        {
            rc = -WEBCLIENT_ERROR;
            goto __exit;
        }
        host_addr_len = ptr - host_addr;
    }

    path_ptr = ezos_strstr(host_addr, "/");
    *request = path_ptr ? path_ptr : "/";

    /* resolve port */
    port_ptr = ezos_strstr(host_addr + host_addr_len, ":");
    if (port_ptr && path_ptr && (port_ptr < path_ptr))
    {
        int port_len = path_ptr - port_ptr - 1;

        ezos_strncpy(port_str, port_ptr + 1, port_len);
        port_str[port_len] = '\0';
    }

    if (port_ptr && (!path_ptr))
    {
        ezos_strcpy(port_str, port_ptr + 1);
    }

    /* ipv4 or domain. */
    if (!host_addr_len)
    {
        if (port_ptr)
        {
            host_addr_len = port_ptr - host_addr;
        }
        else if (path_ptr)
        {
            host_addr_len = path_ptr - host_addr;
        }
        else
        {
            host_addr_len = ezos_strlen(host_addr);
        }
    }

    if ((host_addr_len < 1) || (host_addr_len > url_len))
    {
        rc = -WEBCLIENT_ERROR;
        goto __exit;
    }

    /* get host address ok. */
    {
        char *host_addr_new = web_malloc(host_addr_len + 1);

        if (!host_addr_new)
        {
            rc = -WEBCLIENT_ERROR;
            goto __exit;
        }

        ezos_memcpy(host_addr_new, host_addr, host_addr_len);
        host_addr_new[host_addr_len] = '\0';
        session->host = host_addr_new;
    }

    ezlog_d(TAG_WEB,"host address: %s , port: %s\n", session->host, port_str);

#ifdef WEBCLIENT_USING_MBED_TLS
    if (session->tls_session)
    {
        session->tls_session->port = web_strdup(port_str);
        session->tls_session->host = web_strdup(session->host);
        if (session->tls_session->port == NULL || session->tls_session->host == NULL)
        {
            return -WEBCLIENT_NOMEM;
        }

        return rc;
    }
#endif

    /* resolve the host name. */
    {
        

        
        host = ezos_gethostbyname(session->host);
        if (host == NULL)
        {
            ezlog_e(TAG_WEB,"getaddrinfo err: '%s'\n", session->host);
            rc = -WEBCLIENT_ERROR;
            goto __exit;
        }
        *dst_addr = ezos_malloc(sizeof(ez_sockaddr_in_t));
        if(*dst_addr == NULL){
            rc = -WEBCLIENT_ERROR;
            goto __exit;
        }
        (*dst_addr)->sin_family = EZ_AF_INET;
        port = ezos_atoi(port_str);
        (*dst_addr)->sin_port = ezos_htons(port);
        ezos_memcpy(&((*dst_addr)->sin_addr), host->h_addr_list[0], sizeof(ez_in_addr_t));
        ezos_memset(ipstr, 0, 32);
        ezos_inet_ntop(EZ_AF_INET, (void *)host->h_addr_list[0], ipstr, 32);
        ezlog_d(TAG_WEB,"ezos_gethostbyname:%s\n", ipstr);
    }

__exit:
    if (rc != WEBCLIENT_OK)
    {
        if (session->host)
        {
            web_free(session->host);
            session->host = NULL;
        }
    }

    return rc;
}

#ifdef WEBCLIENT_USING_MBED_TLS
/**
 * create and initialize https session.
 *
 * @param session webclient session
 * @param URI input server URI address
 *
 * @return <0: create failed, no memory or other errors
 *         =0: success
 */
static int webclient_open_tls(struct webclient_session *session, const char *URI)
{
    int tls_ret = 0;
    const char *pers = "webclient";

    EZ_ASSERT(session);

    session->tls_session = (MbedTLSSession *)web_calloc(1, sizeof(MbedTLSSession));
    if (session->tls_session == NULL)
    {
        return -WEBCLIENT_NOMEM;
    }

    session->tls_session->buffer_len = WEBCLIENT_RESPONSE_BUFSZ;
    session->tls_session->buffer = web_malloc(session->tls_session->buffer_len);
    if (session->tls_session->buffer == NULL)
    {
        ezlog_e(TAG_WEB,"no memory for tls_session buffer!\n");
        return -WEBCLIENT_ERROR;
    }

    if ((tls_ret = mbedtls_client_init(session->tls_session, (void *)pers, strlen(pers))) < 0)
    {
        ezlog_e(TAG_WEB,"initialize https client failed return: -0x%x.\n", -tls_ret);
        return -WEBCLIENT_ERROR;
    }

    return WEBCLIENT_OK;
}
#endif

/**
 * connect to http server.
 *
 * @param session webclient session
 * @param URI the input server URI address
 *
 * @return <0: connect failed or other error
 *         =0: connect success
 */
static int webclient_connect(struct webclient_session *session, const char *URI)
{
    int rc = WEBCLIENT_OK;
    int socket_handle;
    ezos_timeval_t timeout;
    ez_sockaddr_in_t *dst_addr=NULL;
    const char *req_url;

    EZ_ASSERT(session);
    EZ_ASSERT(URI);

    timeout.tv_sec = WEBCLIENT_DEFAULT_TIMEO;
    timeout.tv_usec = 0;

    if (ezos_strncmp(URI, "https://", 8) == 0)
    {
#if defined(WEBCLIENT_USING_SAL_TLS)
        session->is_tls = RT_TRUE;
#elif defined(WEBCLIENT_USING_MBED_TLS)
        if (webclient_open_tls(session, URI) < 0)
        {
            ezlog_e(TAG_WEB,"connect failed, https client open URI(%s) failed!\n", URI);
            return -WEBCLIENT_ERROR;
        }
        session->is_tls = RT_TRUE;
#else
        ezlog_e(TAG_WEB,"not support https connect, please enable webclient https configure!\n");
        rc = -WEBCLIENT_ERROR;
        goto __exit;
#endif
    }

    /* Check valid IP address and URL */
    rc = webclient_resolve_address(session, &dst_addr, URI, &req_url);
    if (rc != WEBCLIENT_OK)
    {
        ezlog_e(TAG_WEB,"connect failed, resolve address error(%d).\n", rc);
        goto __exit;
    }
    
    /* Not use 'getaddrinfo()' for https connection */
    if (session->is_tls == ez_false && dst_addr == NULL)
    {
        ezlog_e(TAG_WEB,"is_tls\n");//zoujinwei
        rc = -WEBCLIENT_ERROR;
        goto __exit;
    }

    /* copy host address */
    if (req_url)
    {
        session->req_url = web_strdup(req_url);
    }
    else
    {
        ezlog_e(TAG_WEB,"connect failed, resolve request address error.\n");
        rc = -WEBCLIENT_ERROR;
        goto __exit;
    }

#ifdef WEBCLIENT_USING_MBED_TLS
    if (session->tls_session)
    {
        int tls_ret = 0;

        if ((tls_ret = mbedtls_client_context(session->tls_session)) < 0)
        {
            ezlog_e(TAG_WEB,"connect failed, https client context return: -0x%x\n", -tls_ret);
            return -WEBCLIENT_ERROR;
        }

        if ((tls_ret = mbedtls_client_connect(session->tls_session)) < 0)
        {
            ezlog_e(TAG_WEB,"connect failed, https client connect return: -0x%x\n", -tls_ret);
            return -WEBCLIENT_CONNECT_FAILED;
        }

        socket_handle = session->tls_session->server_fd.fd;

        /* set recv timeout option */
        ezos_setsockopt(socket_handle, EZ_SOL_SOCKET, EZ_SO_RCVTIMEO, (void *)&timeout,
                   sizeof(timeout));
        ezos_setsockopt(socket_handle, EZ_SOL_SOCKET, EZ_SO_SNDTIMEO, (void *)&timeout,
                   sizeof(timeout));

        session->socket = socket_handle;

        return WEBCLIENT_OK;
    }
#endif

    {
#ifdef WEBCLIENT_USING_SAL_TLS
        if (session->is_tls)
        {
            socket_handle = ezos_socket(EZ_AF_INET, EZ_SOCK_STREAM, EZ_PROTOCOL_TLS);
        }
        else
        {
            socket_handle = ezos_socket(EZ_AF_INET, EZ_SOCK_STREAM, EZ_IPPROTO_TCP);
        }
#else
        socket_handle = ezos_socket(EZ_AF_INET, EZ_SOCK_STREAM, EZ_IPPROTO_TCP);
#endif

        if (socket_handle < 0)
        {
            ezlog_e(TAG_WEB,"connect failed, create socket(%d) error.\n", socket_handle);
            rc = -WEBCLIENT_NOSOCKET;
            goto __exit;
        }

        /* set receive and send timeout option */
        if(ezos_setsockopt(socket_handle, EZ_SOL_SOCKET, EZ_SO_RCVTIMEO, (void *)&timeout, sizeof(timeout)) != 0)
        {
            ezlog_e(TAG_WEB,"ezos_setsockopt EZ_SO_RCVTIMEO error\n");
        }
        if(ezos_setsockopt(socket_handle, EZ_SOL_SOCKET, EZ_SO_SNDTIMEO, (void *)&timeout, sizeof(timeout)))
        {
            ezlog_e(TAG_WEB,"ezos_setsockopt EZ_SO_SNDTIMEO error\n");
        }

        if (ezos_connect(socket_handle, (const ez_sockaddr_t *)(dst_addr), sizeof(ez_sockaddr_t)) != 0)
        {
            /* connect failed, close socket */
            ezlog_e(TAG_WEB,"connect failed, connect socket(%d) error:%d\n", socket_handle, ezos_getlasterror());
            ezos_closesocket(socket_handle);
            rc = -WEBCLIENT_CONNECT_FAILED;
            goto __exit;
        }

        session->socket = socket_handle;
    }

__exit:
    if (dst_addr)
    {
        ezos_free(dst_addr);
    }

    return rc;
}

/**
 * add fields data to request header data.
 *
 * @param session webclient session
 * @param fmt fields format
 *
 * @return >0: data length of successfully added
 *         <0: not enough header buffer size
 */
int webclient_header_fields_add(struct webclient_session *session, const char *fmt, ...)
{
    ez_int32_t length;
    va_list args;

    EZ_ASSERT(session);
    EZ_ASSERT(session->header->buffer);

    va_start(args, fmt);
    length = ezos_vsnprintf(session->header->buffer + session->header->length,
                          session->header->size - session->header->length, fmt, args);
    if (length < 0)
    {
        ezlog_e(TAG_WEB,"add fields header data failed, return length(%d) error.\n", length);
        return -WEBCLIENT_ERROR;
    }
    va_end(args);

    session->header->length += length;

    /* check header size */
    if (session->header->length >= session->header->size)
    {
        ezlog_e(TAG_WEB,"not enough header buffer size(%ld)!\n", session->header->size);
        return -WEBCLIENT_ERROR;
    }

    return length;
}

/**
 * get fields information from request/response header data.
 *
 * @param session webclient session
 * @param fields fields keyword
 *
 * @return = NULL: get fields data failed
 *        != NULL: success get fields data
 */
const char *webclient_header_fields_get(struct webclient_session *session, const char *fields)
{
    char *resp_buf = NULL;
    size_t resp_buf_len = 0;

    EZ_ASSERT(session);
    EZ_ASSERT(session->header->buffer);

    resp_buf = session->header->buffer;
    while (resp_buf_len < session->header->length)
    {
        if (webclient_strstri(resp_buf, fields) == resp_buf)
        {
            char *mime_ptr = NULL;

            /* jump space */
            mime_ptr = ezos_strstr(resp_buf, ":");
            if (mime_ptr != NULL)
            {
                mime_ptr += 1;

                while (*mime_ptr && (*mime_ptr == ' ' || *mime_ptr == '\t'))
                    mime_ptr++;

                return mime_ptr;
            }
        }

        if (*resp_buf == '\0')
            break;

        resp_buf += ezos_strlen(resp_buf) + 1;
        resp_buf_len += ezos_strlen(resp_buf) + 1;
    }

    return NULL;
}

/**
 * get http response status code.
 *
 * @param session webclient session
 *
 * @return response status code
 */
int webclient_resp_status_get(struct webclient_session *session)
{
    EZ_ASSERT(session);

    return session->resp_status;
}

/**
 * get http response data content length.
 *
 * @param session webclient session
 *
 * @return response content length
 */
int webclient_content_length_get(struct webclient_session *session)
{
    EZ_ASSERT(session);

    return session->content_length;
}

static int webclient_send_header(struct webclient_session *session, int method)
{
    int rc = WEBCLIENT_OK;
    char *header = NULL;

    EZ_ASSERT(session);

    header = session->header->buffer;

    if (session->header->length == 0)
    {
        /* use default header data */
        if (webclient_header_fields_add(session, "GET %s HTTP/1.1\r\n", session->req_url) < 0)
            return -WEBCLIENT_NOMEM;
        if (webclient_header_fields_add(session, "Host: %s\r\n", session->host) < 0)
            return -WEBCLIENT_NOMEM;
        if (webclient_header_fields_add(session, "User-Agent: Ezviz\r\n\r\n") < 0)
            return -WEBCLIENT_NOMEM;

        webclient_write(session, (unsigned char *)session->header->buffer, session->header->length);
    }
    else
    {
        if (method != WEBCLIENT_USER_METHOD)
        {
            /* check and add fields header data */
            if (ezos_memcmp(header, "HTTP/1.", ezos_strlen("HTTP/1.")))
            {
                char *header_buffer = NULL;
                int length = 0;

                header_buffer = web_strdup(session->header->buffer);
                if (header_buffer == NULL)
                {
                    ezlog_e(TAG_WEB,"no memory for header buffer!\n");
                    rc = -WEBCLIENT_NOMEM;
                    goto __exit;
                }

                /* splice http request header data */
                if (method == WEBCLIENT_GET)
                    length = ezos_snprintf(session->header->buffer, session->header->size, "GET %s HTTP/1.1\r\n%s",
                                         session->req_url ? session->req_url : "/", header_buffer);
                else if (method == WEBCLIENT_POST)
                    length = ezos_snprintf(session->header->buffer, session->header->size, "POST %s HTTP/1.1\r\n%s",
                                         session->req_url ? session->req_url : "/", header_buffer);
                session->header->length = length;

                web_free(header_buffer);
            }

            if (ezos_strstr(header, "Host:") == NULL)
            {
                if (webclient_header_fields_add(session, "Host: %s\r\n", session->host) < 0)
                    return -WEBCLIENT_NOMEM;
            }

            if (ezos_strstr(header, "User-Agent:") == NULL)
            {
                if (webclient_header_fields_add(session, "User-Agent: Ezviz\r\n") < 0)
                    return -WEBCLIENT_NOMEM;
            }

            if (ezos_strstr(header, "Accept:") == NULL)
            {
                if (webclient_header_fields_add(session, "Accept: */*\r\n") < 0)
                    return -WEBCLIENT_NOMEM;
            }

            /* header data end */
            ezos_snprintf(session->header->buffer + session->header->length, session->header->size - session->header->length, "\r\n");
            session->header->length += 2;

            /* check header size */
            if (session->header->length > session->header->size)
            {
                ezlog_e(TAG_WEB,"send header failed, not enough header buffer size(%ld)!\n", session->header->size);
                rc = -WEBCLIENT_NOBUFFER;
                goto __exit;
            }

            webclient_write(session, (unsigned char *)session->header->buffer, session->header->length);
        }
        else
        {
            webclient_write(session, (unsigned char *)session->header->buffer, session->header->length);
        }
    }

    /* get and echo request header data */
    {
        char *header_str, *header_ptr;
        int header_line_len;
        ezlog_d(TAG_WEB,"request header:\n");

        for (header_str = session->header->buffer; (header_ptr = ezos_strstr(header_str, "\r\n")) != NULL;)
        {
            header_line_len = header_ptr - header_str;

            if (header_line_len > 0)
            {
                ezlog_d(TAG_WEB,"%.*s\n", header_line_len, header_str);
            }
            header_str = header_ptr + ezos_strlen("\r\n");
        }
#ifdef WEBCLIENT_DEBUG
        LOG_RAW("\n");
#endif
    }

__exit:
    return rc;
}

/**
 * resolve server response data.
 *
 * @param session webclient session
 *
 * @return <0: resolve response data failed
 *         =0: success
 */
int webclient_handle_response(struct webclient_session *session)
{
    int rc = WEBCLIENT_OK;
    char *mime_buffer = NULL;
    char *mime_ptr = NULL;
    const char *transfer_encoding;
    int i;

    EZ_ASSERT(session);

    /* clean header buffer and size */
    ezos_memset(session->header->buffer, 0x00, session->header->size);
    session->header->length = 0;

    ezlog_d(TAG_WEB,"response header:\n");
    /* We now need to read the header information */
    while (1)
    {
        mime_buffer = session->header->buffer + session->header->length;

        /* read a line from the header information. */
        rc = webclient_read_line(session, mime_buffer, session->header->size - session->header->length);
        if (rc < 0)
            break;

        /* End of headers is a blank line.  exit. */
        if (rc == 0)
            break;
        if ((rc == 1) && (mime_buffer[0] == '\r'))
        {
            mime_buffer[0] = '\0';
            break;
        }

        /* set terminal charater */
        mime_buffer[rc - 1] = '\0';

        /* echo response header data */
        ezlog_d(TAG_WEB,"%s\n", mime_buffer);

        session->header->length += rc;

        if (session->header->length >= session->header->size)
        {
            ezlog_e(TAG_WEB,"not enough header buffer size(%ld)!\n", session->header->size);
            return -WEBCLIENT_NOMEM;
        }
    }

    /* get HTTP status code */
    mime_ptr = web_strdup(session->header->buffer);
    if (mime_ptr == NULL)
    {
        ezlog_e(TAG_WEB,"no memory for get http status code buffer!\n");
        return -WEBCLIENT_NOMEM;
    }

    if (ezos_strstr(mime_ptr, "HTTP/1."))
    {
        char *ptr = mime_ptr;

        ptr += ezos_strlen("HTTP/1.x");

        while (*ptr && (*ptr == ' ' || *ptr == '\t'))
            ptr++;

        /* Terminate string after status code */
        for (i = 0; ((ptr[i] != ' ') && (ptr[i] != '\t')); i++)
            ;
        ptr[i] = '\0';

        session->resp_status = (int)strtol(ptr, NULL, 10);
    }

    /* get content length */
    if (webclient_header_fields_get(session, "Content-Length") != NULL)
    {
        session->content_length = ezos_atoi(webclient_header_fields_get(session, "Content-Length"));
    }
    session->content_remainder = session->content_length ? (size_t)session->content_length : 0xFFFFFFFF;

    transfer_encoding = webclient_header_fields_get(session, "Transfer-Encoding");
    if (transfer_encoding && ezos_strcmp(transfer_encoding, "chunked") == 0)
    {
        char line[16];

        /* chunk mode, we should get the first chunk size */
        webclient_read_line(session, line, session->header->size);
        session->chunk_sz = strtol(line, NULL, 16);
        session->chunk_offset = 0;
    }

    if (mime_ptr)
    {
        web_free(mime_ptr);
    }

    if (rc < 0)
    {
        return rc;
    }

    return session->resp_status;
}

/**
 * create webclient session, set maximum header and response size
 *
 * @param header_sz maximum send header size
 * @param resp_sz maximum response data size
 *
 * @return  webclient session structure
 */
struct webclient_session *webclient_session_create(size_t header_sz)
{
    struct webclient_session *session;

    /* create session */
    session = (struct webclient_session *)web_calloc(1, sizeof(struct webclient_session));
    if (session == NULL)
    {
        ezlog_e(TAG_WEB,"webclient create failed, no memory for webclient session!\n");
        return NULL;
    }
    /* initialize the socket of session */
    session->socket = -1;
    session->content_length = -1;

    session->header = (struct webclient_header *)web_calloc(1, sizeof(struct webclient_header));
    if (session->header == NULL)
    {
        ezlog_e(TAG_WEB,"webclient create failed, no memory for session header!\n");
        web_free(session);
        session = NULL;
        return NULL;
    }
    session->header->size = header_sz;
    session->header->buffer = (char *)web_calloc(1, header_sz);
    if (session->header->buffer == NULL)
    {
        ezlog_e(TAG_WEB,"webclient create failed, no memory for session header buffer!\n");
        web_free(session->header);
        web_free(session);
        session = NULL;
        return NULL;
    }
    return session;
}

static int webclient_clean(struct webclient_session *session);

/**
 *  send GET request to http server and get response header.
 *
 * @param session webclient session
 * @param URI input server URI address
 * @param header GET request header
 *             = NULL: use default header data
 *            != NULL: use custom header data
 *
 * @return <0: send GET request failed
 *         >0: response http status code
 */
int webclient_get(struct webclient_session *session, const char *URI)
{
    int rc = WEBCLIENT_OK;
    int resp_status = 0;

    EZ_ASSERT(session);
    EZ_ASSERT(URI);

    rc = webclient_connect(session, URI);
    if (rc != WEBCLIENT_OK)
    {
        /* connect to webclient server failed. */
        return rc;
    }

    rc = webclient_send_header(session, WEBCLIENT_GET);
    if (rc != WEBCLIENT_OK)
    {
        /* send header to webclient server failed. */
        return rc;
    }

    /* handle the response header of webclient server */
    resp_status = webclient_handle_response(session);
    ezlog_d(TAG_WEB,"get position handle response(%d).\n", resp_status);
    if (resp_status > 0)
    {
        const char *location = webclient_header_fields_get(session, "Location");

        /* relocation */
        if ((resp_status == 302 || resp_status == 301) && location)
        {
            char *new_url;

            new_url = web_strdup(location);
            if (new_url == NULL)
            {
                return -WEBCLIENT_NOMEM;
            }

            /* clean webclient session */
            webclient_clean(session);
            /* clean webclient session header */
            session->header->length = 0;
            ezos_memset(session->header->buffer, 0, session->header->size);

            rc = webclient_get(session, new_url);

            web_free(new_url);
            return rc;
        }
    }

    return resp_status;
}

/**
 *  http breakpoint resume.
 *
 * @param session webclient session
 * @param URI input server URI address
 * @param position last downloaded position
 *
 * @return <0: send GET request failed
 *         >0: response http status code
 */
int webclient_get_position(struct webclient_session *session, const char *URI, int position)
{
    int rc = WEBCLIENT_OK;
    int resp_status = 0;

    EZ_ASSERT(session);
    EZ_ASSERT(URI);
    rc = webclient_connect(session, URI);
    if (rc != WEBCLIENT_OK)
    {
        ezlog_e(TAG_WEB,"webclient_connect error(%d).\n", rc);
        return rc;
    }
    /* splice header*/
    if (webclient_header_fields_add(session, "Range: bytes=%d-\r\n", position) <= 0)
    {
        rc = -WEBCLIENT_ERROR;
        return rc;
    }
    rc = webclient_send_header(session, WEBCLIENT_GET);
    if (rc != WEBCLIENT_OK)
    {
        ezlog_e(TAG_WEB,"webclient_send_header fail\n");
        return rc;
    }
    /* handle the response header of webclient server */
    resp_status = webclient_handle_response(session);

    ezlog_d(TAG_WEB,"get position handle response(%d).\n", resp_status);

    if (resp_status > 0)
    {
        const char *location = webclient_header_fields_get(session, "Location");

        /* relocation */
        if ((resp_status == 302 || resp_status == 301) && location)
        {
            char *new_url;

            new_url = web_strdup(location);
            if (new_url == NULL)
            {
                return -WEBCLIENT_NOMEM;
            }

            /* clean webclient session */
            webclient_clean(session);
            /* clean webclient session header */
            session->header->length = 0;
            ezos_memset(session->header->buffer, 0, session->header->size);

            rc = webclient_get_position(session, new_url, position);

            web_free(new_url);
            return rc;
        }
    }

    return resp_status;
}

/**
 * send POST request to server and get response header data.
 *
 * @param session webclient session
 * @param URI input server URI address
 * @param header POST request header, can't be empty
 * @param post_data data send to the server
 *                = NULL: just connect server and send header
 *               != NULL: send header and body data, resolve response data
 * @param data_len the length of send data
 *
 * @return <0: send POST request failed
 *         =0: send POST header success
 *         >0: response http status code
 */
int webclient_post(struct webclient_session *session, const char *URI, const void *post_data, size_t data_len)
{
    int rc = WEBCLIENT_OK;
    int resp_status = 0;

    EZ_ASSERT(session);
    EZ_ASSERT(URI);

    if ((post_data != NULL) && (data_len == 0))
    {
        ezlog_e(TAG_WEB,"input post data length failed\n");
        return -WEBCLIENT_ERROR;
    }

    rc = webclient_connect(session, URI);
    if (rc != WEBCLIENT_OK)
    {
        /* connect to webclient server failed. */
        return rc;
    }

    rc = webclient_send_header(session, WEBCLIENT_POST);
    if (rc != WEBCLIENT_OK)
    {
        /* send header to webclient server failed. */
        return rc;
    }

    if (post_data && (data_len > 0))
    {
        webclient_write(session, post_data, data_len);

        /* resolve response data, get http status code */
        resp_status = webclient_handle_response(session);
        ezlog_d(TAG_WEB,"post handle response(%d).\n", resp_status);
    }

    return resp_status;
}

/**
 * set receive and send data timeout.
 *
 * @param session http session
 * @param millisecond timeout millisecond
 *
 * @return 0: set timeout success
 */
int webclient_set_timeout(struct webclient_session *session, int millisecond)
{
    ezos_timeval_t timeout;
    int second = millisecond / 1000;

    EZ_ASSERT(session);

    timeout.tv_sec = second;
    timeout.tv_usec = 0;

    /* set recv timeout option */
    if(ezos_setsockopt(session->socket, EZ_SOL_SOCKET, EZ_SO_RCVTIMEO, (void *)&timeout, sizeof(timeout)) != 0)
    {
        ezlog_e(TAG_WEB,"ezos_setsockopt EZ_SO_RCVTIMEO error\n");
    }
    if(ezos_setsockopt(session->socket, EZ_SOL_SOCKET, EZ_SO_SNDTIMEO, (void *)&timeout, sizeof(timeout)) != 0)
    {
        ezlog_e(TAG_WEB,"ezos_setsockopt EZ_SO_SNDTIMEO error\n");
    }

    return 0;
}

static int webclient_next_chunk(struct webclient_session *session)
{
    char line[64];
    int length;

    EZ_ASSERT(session);

    ezos_memset(line, 0x00, sizeof(line));
    length = webclient_read_line(session, line, sizeof(line));
    if (length > 0)
    {
        if (ezos_strcmp(line, "\r") == 0)
        {
            length = webclient_read_line(session, line, sizeof(line));
            if (length <= 0)
            {
                ezos_closesocket(session->socket);
                session->socket = -1;
                return length;
            }
        }
    }
    else
    {
        ezos_closesocket(session->socket);
        session->socket = -1;

        return length;
    }

    session->chunk_sz = strtol(line, NULL, 16);
    session->chunk_offset = 0;

    if (session->chunk_sz == 0)
    {
        /* end of chunks */
        ezos_closesocket(session->socket);
        session->socket = -1;
        session->chunk_sz = -1;
    }

    return session->chunk_sz;
}

/**
 *  read data from http server.
 *
 * @param session http session
 * @param buffer read buffer
 * @param length the maximum of read buffer size
 *
 * @return <0: read data error
 *         =0: http server disconnect
 *         >0: successfully read data length
 */
int webclient_read(struct webclient_session *session, void *buffer, size_t length)
{
    int bytes_read = 0;
    int total_read = 0;
    int left;

    EZ_ASSERT(session);

    /* get next chunk size is zero, client is already closed, return zero */
    if (session->chunk_sz < 0)
    {
        return 0;
    }

    if (session->socket < 0)
    {
        return -WEBCLIENT_DISCONNECT;
    }

    if (length == 0)
    {
        return 0;
    }

    /* which is transfered as chunk mode */
    if (session->chunk_sz)
    {
        if ((int)length > (session->chunk_sz - session->chunk_offset))
        {
            length = session->chunk_sz - session->chunk_offset;
        }

        bytes_read = webclient_recv(session, buffer, length, 0);
        if (bytes_read <= 0)
        {
            if (ezos_getlasterror() == EZ_EWOULDBLOCK || ezos_getlasterror() == EZ_EAGAIN)
            {
                /* recv timeout */
                return -WEBCLIENT_TIMEOUT;
            }
            else
            {
                ezos_closesocket(session->socket);
                session->socket = -1;
                return 0;
            }
        }

        session->chunk_offset += bytes_read;
        if (session->chunk_offset >= session->chunk_sz)
        {
            webclient_next_chunk(session);
        }

        return bytes_read;
    }

    if (session->content_length > 0)
    {
        if (length > session->content_remainder)
        {
            length = session->content_remainder;
        }

        if (length == 0)
        {
            return 0;
        }
    }

    /*
     * Read until: there is an error, we've read "size" bytes or the remote
     * side has closed the connection.
     */
    left = length;
    do
    {
        bytes_read = webclient_recv(session, (void *)((char *)buffer + total_read), left, 0);
        if (bytes_read <= 0)
        {
#if defined(WEBCLIENT_USING_SAL_TLS) || defined(WEBCLIENT_USING_MBED_TLS)
            if (session->is_tls &&
                (bytes_read == MBEDTLS_ERR_SSL_WANT_READ || bytes_read == MBEDTLS_ERR_SSL_WANT_WRITE))
            {
                continue;
            }

#endif
            ezlog_d(TAG_WEB,"receive data error(%d).\n", bytes_read);

            if (total_read)
            {
                break;
            }
            else
            {
                if (ezos_getlasterror() == EZ_EWOULDBLOCK || ezos_getlasterror() == EZ_EAGAIN)
                {
                    /* recv timeout */
                    ezlog_e(TAG_WEB,"receive data timeout.\n");
                    return -WEBCLIENT_TIMEOUT;
                }
                else
                {
                    ezos_closesocket(session->socket);
                    session->socket = -1;
                    return 0;
                }
            }
        }

        left -= bytes_read;
        total_read += bytes_read;
    } while (left);

    if (session->content_length > 0)
    {
        session->content_remainder -= total_read;
    }

    return total_read;
}

/**
 *  write data to http server.
 *
 * @param session http session
 * @param buffer write buffer
 * @param length write buffer size
 *
 * @return <0: write data error
 *         =0: http server disconnect
 *         >0: successfully write data length
 */
int webclient_write(struct webclient_session *session, const void *buffer, size_t length)
{
    int bytes_write = 0;
    int total_write = 0;
    int left = length;

    EZ_ASSERT(session);

    if (session->socket < 0)
    {
        return -WEBCLIENT_DISCONNECT;
    }

    if (length == 0)
    {
        return 0;
    }

    /* send all of data on the buffer. */
    do
    {
        bytes_write = webclient_send(session, (void *)((char *)buffer + total_write), left, 0);
        if (bytes_write <= 0)
        {
#if defined(WEBCLIENT_USING_SAL_TLS) || defined(WEBCLIENT_USING_MBED_TLS)
            if (session->is_tls &&
                (bytes_write == MBEDTLS_ERR_SSL_WANT_READ || bytes_write == MBEDTLS_ERR_SSL_WANT_WRITE))
            {
                continue;
            }
#endif
            if (ezos_getlasterror() == EZ_EWOULDBLOCK || ezos_getlasterror() == EZ_EAGAIN)
            {
                /* send timeout */
                if (total_write)
                {
                    return total_write;
                }
                continue;
                /* TODO: whether return the TIMEOUT
                 * return -WEBCLIENT_TIMEOUT; */
            }
            else
            {
                ezos_closesocket(session->socket);
                session->socket = -1;

                if (total_write == 0)
                {
                    return -WEBCLIENT_DISCONNECT;
                }
                break;
            }
        }

        left -= bytes_write;
        total_write += bytes_write;
    } while (left);

    return total_write;
}

/* close session socket, free host and request url */
static int webclient_clean(struct webclient_session *session)
{
#ifdef WEBCLIENT_USING_MBED_TLS
    if (session->tls_session)
    {
        mbedtls_client_close(session->tls_session);
    }
    else
    {
        if (session->socket >= 0)
        {
            ezos_closesocket(session->socket);
            session->socket = -1;
        }
    }
#else
    if (session->socket >= 0)
    {
        ezos_closesocket(session->socket);
        session->socket = -1;
    }
#endif

    if (session->host)
    {
        web_free(session->host);
        session->host = NULL;
    }

    if (session->req_url)
    {
        web_free(session->req_url);
        session->req_url = NULL;
    }

    session->content_length = -1;

    return 0;
}

/**
 * close a webclient client session.
 *
 * @param session http client session
 *
 * @return 0: close success
 */
int webclient_close(struct webclient_session *session)
{
    if(session == NULL){
        return -1;
    }

    webclient_clean(session);

    if (session->header && session->header->buffer)
    {
        web_free(session->header->buffer);
    }

    if (session->header)
    {
        web_free(session->header);
    }

    web_free(session);
    session = NULL;

    return 0;
}

/**
 * get wenclient request response data.
 *
 * @param session wenclient session
 * @param response response buffer address
 * @param resp_len response buffer length
 *
 * @return response data size
 */
int webclient_response(struct webclient_session *session, void **response, size_t *resp_len)
{
    unsigned char *buf_ptr;
    unsigned char *response_buf = 0;
    int length, total_read = 0;

    EZ_ASSERT(session);
    EZ_ASSERT(response);

    /* initialize response */
    *response = NULL;

    /* not content length field kind */
    if (session->content_length < 0)
    {
        size_t result_sz;

        total_read = 0;
        while (1)
        {
            unsigned char *new_resp = NULL;

            result_sz = total_read + WEBCLIENT_RESPONSE_BUFSZ;
            new_resp = web_realloc(response_buf, result_sz + 1);
            if (new_resp == NULL)
            {
                ezlog_e(TAG_WEB,"no memory for realloc new response buffer!\n");
                break;
            }

            response_buf = new_resp;
            buf_ptr = (unsigned char *)response_buf + total_read;

            /* read result */
            length = webclient_read(session, buf_ptr, result_sz - total_read);
            if (length <= 0)
                break;

            total_read += length;
        }
    }
    else
    {
        int result_sz;

        result_sz = session->content_length;
        response_buf = web_calloc(1, result_sz + 1);
        if (response_buf == NULL)
        {
            return -WEBCLIENT_NOMEM;
        }

        buf_ptr = (unsigned char *)response_buf;
        for (total_read = 0; total_read < result_sz;)
        {
            length = webclient_read(session, buf_ptr, result_sz - total_read);
            if (length <= 0)
                break;

            buf_ptr += length;
            total_read += length;
        }
    }

    if ((total_read == 0) && (response_buf != 0))
    {
        web_free(response_buf);
        response_buf = NULL;
    }

    if (response_buf)
    {
        *response = (void *)response_buf;
        *(response_buf + total_read) = '\0';
        *resp_len = total_read;
    }

    return total_read;
}

/**
 * add request(GET/POST) header data.
 *
 * @param request_header add request buffer address
 * @param fmt fields format
 *
 * @return <=0: add header failed
 *          >0: add header data size
 */

int webclient_request_header_add(char **request_header, const char *fmt, ...)
{
    ez_int32_t length, header_length;
    char *header;
    va_list args;

    EZ_ASSERT(request_header);

    if (*request_header == NULL)
    {
        header = ezos_calloc(1, WEBCLIENT_HEADER_BUFSZ);
        if (header == NULL)
        {
            ezlog_e(TAG_WEB,"No memory for webclient request header add.\n");
            return -1;
        }
        *request_header = header;
    }
    else
    {
        header = *request_header;
    }

    va_start(args, fmt);
    header_length = ezos_strlen(header);
    length = ezos_vsnprintf(header + header_length, WEBCLIENT_HEADER_BUFSZ - header_length, fmt, args);
    if (length < 0)
    {
        ezlog_e(TAG_WEB,"add request header data failed, return length(%d) error.\n", length);
        return -WEBCLIENT_ERROR;
    }
    va_end(args);

    /* check header size */
    if (ezos_strlen(header) >= WEBCLIENT_HEADER_BUFSZ)
    {
        ezlog_e(TAG_WEB,"not enough request header data size(%d)!\n", WEBCLIENT_HEADER_BUFSZ);
        return -WEBCLIENT_ERROR;
    }

    return length;
}

/**
 *  send request(GET/POST) to server and get response data.
 *
 * @param URI input server address
 * @param header send header data
 *             = NULL: use default header data, must be GET request
 *            != NULL: user custom header data, GET or POST request
 * @param post_data data sent to the server
 *             = NULL: it is GET request
 *            != NULL: it is POST request
 * @param data_len send data length
 * @param response response buffer address
 * @param resp_len response buffer length
 *
 * @return <0: request failed
 *        >=0: response buffer size
 */
int webclient_request(const char *URI, const char *header, const void *post_data, size_t data_len, void **response, size_t *resp_len)
{
    struct webclient_session *session = NULL;
    int rc = WEBCLIENT_OK;
    int totle_length = 0;

    EZ_ASSERT(URI);

    if (post_data == NULL && response == NULL)
    {
        ezlog_e(TAG_WEB,"request get failed, get response data cannot be empty.\n");
        return -WEBCLIENT_ERROR;
    }

    if ((post_data != NULL) && (data_len == 0))
    {
        ezlog_e(TAG_WEB,"input post data length failed\n");
        return -WEBCLIENT_ERROR;
    }

    if ((response != NULL && resp_len == NULL) ||
        (response == NULL && resp_len != NULL))
    {
        ezlog_e(TAG_WEB,"input response data or length failed\n");
        return -WEBCLIENT_ERROR;
    }

    if (post_data == NULL)
    {
        /* send get request */
        session = webclient_session_create(WEBCLIENT_HEADER_BUFSZ);
        if (session == NULL)
        {
            rc = -WEBCLIENT_NOMEM;
            goto __exit;
        }

        if (header != NULL)
        {
            char *header_str, *header_ptr;
            int header_line_length;

            for (header_str = (char *)header; (header_ptr = ezos_strstr(header_str, "\r\n")) != NULL;)
            {
                header_line_length = header_ptr + ezos_strlen("\r\n") - header_str;
                webclient_header_fields_add(session, "%.*s", header_line_length, header_str);
                header_str += header_line_length;
            }
        }

        if (webclient_get(session, URI) != 200)
        {
            rc = -WEBCLIENT_ERROR;
            goto __exit;
        }

        totle_length = webclient_response(session, response, resp_len);
        if (totle_length <= 0)
        {
            rc = -WEBCLIENT_ERROR;
            goto __exit;
        }
    }
    else
    {
        /* send post request */
        session = webclient_session_create(WEBCLIENT_HEADER_BUFSZ);
        if (session == NULL)
        {
            rc = -WEBCLIENT_NOMEM;
            goto __exit;
        }

        if (header != NULL)
        {
            char *header_str, *header_ptr;
            int header_line_length;

            for (header_str = (char *)header; (header_ptr = ezos_strstr(header_str, "\r\n")) != NULL;)
            {
                header_line_length = header_ptr + ezos_strlen("\r\n") - header_str;
                webclient_header_fields_add(session, "%.*s", header_line_length, header_str);
                header_str += header_line_length;
            }
        }

        if (ezos_strstr(session->header->buffer, "Content-Length") == NULL)
        {
            webclient_header_fields_add(session, "Content-Length: %ld\r\n", ezos_strlen(post_data));
        }

        if (ezos_strstr(session->header->buffer, "Content-Type") == NULL)
        {
            webclient_header_fields_add(session, "Content-Type: application/octet-stream\r\n");
        }

        if (webclient_post(session, URI, post_data, data_len) != 200)
        {
            rc = -WEBCLIENT_ERROR;
            goto __exit;
        }

        totle_length = webclient_response(session, response, resp_len);
        if (totle_length <= 0)
        {
            rc = -WEBCLIENT_ERROR;
            goto __exit;
        }
    }

__exit:
    if (session)
    {
        webclient_close(session);
        session = NULL;
    }

    if (rc < 0)
    {
        return rc;
    }

    return totle_length;
}
