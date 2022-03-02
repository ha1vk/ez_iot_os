#ifndef _NET_CTRL_TCP_H_
#define _NET_CTRL_TCP_H_

#ifdef __cplusplus
extern "C"
{
#endif

void tcp_server_task(void *pvParameters);

void music_server_start(void);

void music_server_stop(void);    


#ifdef __cplusplus
}
#endif
#endif /* _EZVIZ_DISTRIBUTION_H_ */
