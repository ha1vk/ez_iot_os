
#include "utest.h"
#include <ezos.h>

#define MYPORT 6000

static void ut_ezos_socket(void);
static void ut_ezos_sem(void);
static void ut_ezos_thread(void);
static void ut_ezos_time(void);
static void ut_ezos_mem(void);
static void ut_ezos_libc(void);

UTEST_TC_EXPORT(ut_ezos_libc, NULL, NULL, 10);
UTEST_TC_EXPORT(ut_ezos_mem, NULL, NULL, 10);
UTEST_TC_EXPORT(ut_ezos_time, NULL, NULL, 10);
UTEST_TC_EXPORT(ut_ezos_thread, NULL, NULL, 10);
UTEST_TC_EXPORT(ut_ezos_sem, NULL, NULL, 10);
UTEST_TC_EXPORT(ut_ezos_socket, NULL, NULL, 10);




ez_thread_t gthread = NULL;
ez_mutex_t glock = NULL;
ez_sem_t gsem = NULL;



int server_create()
{
	int socket_fd;
	int ret = 0;
	ez_sockaddr_in_t server_addr;
    
	socket_fd = ezos_socket(EZ_AF_INET, EZ_SOCK_STREAM, EZ_IPPROTO_TCP);
	if (socket_fd == -1){
		ezos_printf("test fail:\tezos_socket\n");
		return -1;
	}

	ezos_bzero(&server_addr, sizeof(server_addr));

	server_addr.sin_family = EZ_AF_INET;
	server_addr.sin_port = ezos_htons(MYPORT);
	server_addr.sin_addr.s_addr = ezos_htonl(EZ_INADDR_ANY);

	//bind  
	if (ezos_bind(socket_fd, (ez_sockaddr_t*)&server_addr, sizeof(server_addr)) < 0){
		ezos_printf("test fail:\tezos_bind\n");
		return -1;
	}

	//listen  
	if (ezos_listen(socket_fd, 5) < 0){
		ezos_printf("test fail:\tezos_listen\n");
		return -1;
	}

	return socket_fd;
}


static void net_server_task(void* user_data)
{
	int serverfd;
	int clientfd;
	char recvbuf[50];
	int recvlen;
	ez_sockaddr_t client_addr;
	ez_socklen_t len;

	serverfd = server_create();
	len = sizeof(client_addr);
	while (1)
	{
		clientfd = ezos_accept(serverfd, &client_addr, &len);
		if (clientfd == -1) {
			ezos_printf("test fail:\tezos_accept\n");
			return;
		}

		recvlen = ezos_recv(clientfd, recvbuf, 50, 0);
		if (recvlen <= 0) {
			ezos_printf("test fail:\tezos_recv\n");
			ezos_closesocket(clientfd);
			return;
		}
		ezos_closesocket(clientfd);
		break;
	}
	ezos_delay_ms(1000);
	ezos_closesocket(serverfd);
}

int client_create()
{
	int socket_fd;
	const int opt = 1400;
	int ret = 0;
	struct hostent *hpr;

	socket_fd = ezos_socket(EZ_AF_INET, EZ_SOCK_STREAM, EZ_IPPROTO_TCP);
	if (socket_fd == -1)
	{
		return -1;
	}

	ret = ezos_setsockopt(socket_fd, EZ_IPPROTO_TCP, EZ_TCP_MAXSEG, (char *)&opt, sizeof(opt));
	if (ret < 0)
	{
		ezos_printf("set socket opt, EZ_TCP_MAXSEG error\n");
	}

	return socket_fd;
}

int client_connect(int socket_fd)
{
	ez_sockaddr_in_t dst_addr;
	ez_sockaddr_t src_addr;
	int len;
	int return_value = 0;
	int lasterror;
	int socket_err = 0;
	ez_socklen_t socklen = sizeof(socket_err);
	ez_hostent_t *host;
	char ip[32];

	ezos_setnonblock(socket_fd);

	host = ezos_gethostbyname("www.baidu.com");
	if (host == NULL){
		ezos_printf("test fail:\tezos_gethostbyname\n");
		return -1;
	}

	ezos_inet_ntop(EZ_AF_INET, (void *)host->h_addr_list[0], ip, 32);

	dst_addr.sin_family = EZ_AF_INET;
	dst_addr.sin_addr.s_addr = ezos_inet_addr("127.0.0.1");
	dst_addr.sin_port = ezos_htons(MYPORT);

	if (ezos_connect(socket_fd, (ez_sockaddr_t *)(&dst_addr), sizeof(dst_addr)) == -1)
	{
		lasterror = ezos_getlasterror();
		if ((lasterror != EZ_EINPROGRESS) && (lasterror != EZ_EWOULDBLOCK) && (lasterror != 0))
		{
			ezos_printf("test fail:\tezos_connect\n");
			return -1;
		}
	}

	return_value = ezos_poll(socket_fd, POLL_CONNECT, 10000);
	if (return_value <= 0) {
		ezos_printf("test fail:\tezos_socket_poll\n");
		return -1;
	}

	if (ezos_getsockopt(socket_fd, EZ_SOL_SOCKET, EZ_SO_ERROR, (char *)&socket_err, (ez_socklen_t*)&socklen) == -1) {
		return -1;
	}

	if (socket_err != 0) {
		return -1;
	}

	return 0;
}

int client_read(int socket_fd, unsigned char* read_buf, int read_buf_maxsize, int timeout_ms)
{
	int rev_size = 0;
	int rev_total_size = 0;
	int return_value = 0;

	do
	{
		return_value = ezos_poll(socket_fd, POLL_RECV, timeout_ms);
		if (return_value < 0)
			return -1;
		else if (return_value == 0)
			return 0;

		rev_size = ezos_recv(socket_fd, read_buf + rev_total_size, read_buf_maxsize - rev_total_size, 0);
		if (rev_size < 0)
		{
			return -1;
		}
		else if (rev_size == 0)
		{
			return -1;
		}
		rev_total_size += rev_size;

	} while (rev_total_size < read_buf_maxsize);

	return rev_total_size;
}

int client_write(int socket_fd, char* write_buf, int write_buf_size, int timeout_ms)
{
	int send_size = 0;
	int send_total_size = 0;

	int return_value = 0;

	do
	{
		return_value = ezos_poll(socket_fd, POLL_SEND, timeout_ms);
		if (return_value < 0)
			return -1;
		else if (return_value == 0)
			return -2;

		send_size = ezos_send(socket_fd, write_buf + send_total_size, write_buf_size - send_total_size, 0);
		if (send_size == -1)
		{
			ezos_printf("test fail:\tezos_send\n");
			return -1;
		}

	} while (0);

	return send_size;
}

int net_client_task()
{
	int clientfd;
	int ret;
	clientfd = client_create();
	if (clientfd <= 0) {
		return -1;
	}
	ret = client_connect(clientfd);
	if (ret) {
		return -1;
	}
	client_write(clientfd, "testnet", 7, 10000);
	ezos_closesocket(clientfd);
	
    ezos_delay_ms(2000);
	return 0;
}

static void ut_ezos_socket(void)
{
	int ret;
#if defined (_WIN32) || defined(_WIN64)
	WORD wVersionRequested;
	WSADATA wsaData;
	//WinSock初始化 
	wVersionRequested = MAKEWORD(2, 2);            //希望使用的WinSock DLL的版本
	ret = WSAStartup(wVersionRequested, &wsaData); //加载套接字库
	if (ret != 0) {
		return -1;
	}
#endif

	ezos_thread_create(NULL, "test_net",net_server_task, NULL, 16*1024, 10);

	ezos_delay_ms(100);
	net_client_task();
}


static void test_sem_task(void* user_data)
{
	int ret;
	ezos_time_t* starttime = (ezos_time_t*)user_data;
	ezos_time_t endtime;

	if (starttime)
	{
		if ((ret = ezos_sem_wait(gsem, 1000)) == 0) {
			ezos_printf("test fail:\tezos_sem_post\n");
			ezos_printf("test fail:\tezos_sem_wait\n");
		}

		if ((ret = ezos_sem_wait(gsem, -1)) != 0) {
			ezos_printf("test fail:\tezos_sem_post\n");
			ezos_printf("test fail:\tezos_sem_wait\n");
		}

		endtime = ezos_time(NULL);
		if ((endtime - *starttime) == 2) {
			
		}
		else {
			ezos_printf("test fail:\tezos_sem_post\n");
			ezos_printf("test fail:\tezos_sem_wait\n");
		}
	}

}

static void ut_ezos_sem(void)
{
	int ret;
	ezos_time_t *starttime;
	gsem = ezos_sem_create(0, 1);
	if (gsem == NULL)
	{
		ezos_printf("test fail:\tezos_sem_create\n");
		return;
	}

	starttime = ezos_malloc(sizeof(ezos_time_t));
	*starttime = ezos_time(NULL);
	ezos_mutex_lock(glock);
	ezos_thread_create(NULL, "test_sem",test_sem_task, starttime, 16*1024, 10);
	ezos_delay_ms(2000);
	if ((ret = ezos_sem_post(gsem)) != 0) {
		ezos_printf("test fail:\tezos_sem_post\n");
		return;
	}

	ezos_sem_destroy(gsem);
	gsem = NULL;
	ezos_free(starttime);
}

static void test_thread_task(void* user_data)
{
	ezos_time_t* starttime = (ezos_time_t*)user_data;
	ezos_time_t endtime;

	if (starttime)
	{
		ezos_mutex_lock(glock);
		endtime = ezos_time(NULL); 
		if ((endtime - *starttime) == 2) {
			
		}
		else {
			ezos_printf("test fail:\tezos_mutex_lock\n");
			ezos_printf("test fail:\tezos_mutex_unlock\n");
		}
		ezos_mutex_unlock(glock);
	}
}

static void ut_ezos_thread(void)
{
	ezos_time_t *starttime;
	glock = ezos_mutex_create();
	if (glock == NULL)
	{
		ezos_printf("test fail:\tezos_mutex_create\n");
		return;
	}

	starttime = ezos_malloc(sizeof(ezos_time_t));
	*starttime = ezos_time(NULL);
	ezos_mutex_lock(glock);
	ezos_thread_create(&gthread, "test_thread",test_thread_task, starttime, 16*1024, 10);
	if (gthread == NULL)
	{
		ezos_printf("test fail:\tezos_thread_create\n");
		return;
	}
	ezos_delay_ms(2000);
	ezos_mutex_unlock(glock);
	
	ezos_thread_destroy(gthread);
	gthread = NULL;
	ezos_mutex_destroy(glock);
	glock = NULL;
	ezos_free(starttime);
	return;
}

static void ut_ezos_time(void)
{
	int ret;
	ezos_timespec_t clock;
	ezos_timeval_t time_val;
	ezos_tm_t tm_time;
	ezos_time_t time_1;
	ezos_time_t time_2 = ezos_time(NULL);
	ezos_time(&time_1);
	ezos_localtime(&time_2, &tm_time);

	if ((ret = ezos_get_clock(&clock)) != 0) {
		ezos_printf("test fail:\tezos_time_get_clock:%d\n", ret);
		return;
	}
	if ((ret = ezos_gettimeofday(&time_val, NULL)) != 0) {
		ezos_printf("test fail:\tezos_gettimeofday:%d\n", ret);
		return;
	}
	if (clock.tv_sec == time_val.tv_sec && clock.tv_sec == time_1 && time_1 == time_2) {

	}
	else {
		ezos_printf("test fail:\tezos_get_clock:%d\n", clock.tv_sec);
		ezos_printf("test fail:\tezos_gettimeofday:%d\n", time_val.tv_sec);
		ezos_printf("test fail:\tezos_time: %d, %d\n", time_1, time_2);
		return;
	}
    
	ezos_delay_ms(2000);
	ezos_time(&time_2);
	if ((time_2 - time_1) != 2) {
		ezos_printf("test fail:\tez_os_delay_ms\n");
	}

	return;
}

static void ut_ezos_mem(void)
{
	char *src= NULL;
	src = ezos_malloc(10);
	uassert_not_null(src);
	src = ezos_realloc(src, 20);
	uassert_not_null(src);
	ezos_free(src);
	src = ezos_calloc(2, 10);
	uassert_not_null(src);
	ezos_free(src);
}

static void ut_ezos_libc(void)
{
	char buf[20];
	char *outstr;

	buf[0] = 'a';
	ezos_memset(buf, 0, sizeof(buf));
	if (buf[0] != 0) {
		ezos_printf("test fail:\tezos_memset\n");
		return;
	}

	ezos_memcpy(buf, "helloworld", 10);
	uassert_int_equal(0, ezos_memcmp(buf, "helloworld", 10));

	ezos_memset(buf, 0, sizeof(buf));
	ezos_sprintf(buf, "hello%s", "world");
	uassert_int_equal(0, ezos_strcmp(buf, "helloworld"));

	uassert_int_equal(0, ezos_strncmp(buf, "helloworld", 10));
	uassert_int_equal(10, ezos_strlen(buf));
	uassert_not_null(ezos_strstr(buf, "helloworld"));

	ezos_memset(buf, 0, sizeof(buf));
	ezos_snprintf(buf, sizeof(buf), "helloworld");
	uassert_int_equal(0, ezos_strcmp(buf, "helloworld"));

	ezos_memset(buf, 0, sizeof(buf));
	ezos_strncpy(buf, "helloworld", 10);
	uassert_int_equal(0, ezos_strcmp(buf, "helloworld"));

	uassert_not_null(ezos_strchr(buf, 'o'));
	uassert_not_null(ezos_strrchr(buf, 'o'));

	ezos_memmove(buf, buf+5, ezos_strlen(buf)-5);
	uassert_int_equal(0, ezos_strncmp(buf, "world", 5));


	ezos_memset(buf, 0, sizeof(buf));
	ezos_memcpy(buf, "100", 3);
	uassert_int_equal(100, ezos_atoi(buf));
	ezos_rand();
}