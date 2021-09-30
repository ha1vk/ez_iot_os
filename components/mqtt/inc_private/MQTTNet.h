#ifndef H_MQTTNET_H_
#define H_MQTTNET_H_

#include "time_interface.h"

typedef struct MQTTNetwork
{
    int socket_fd;
    int (*mqttread)(struct MQTTNetwork *, unsigned char *, int, int);
    int (*mqttwrite)(struct MQTTNetwork *, unsigned char *, int, int);
} Network;

void MQTTNetInit(Network *net_work);
unsigned int MQTTNetConnect(Network *net_work, char *ip, int port);
void MQTTNetDisconnect(Network *net_work);
void MQTTNetFini(Network *net_work);

unsigned int MQTTNetGetLastError();
void MQTTNetSetLastError(unsigned int code);

typedef struct Timer
{
    ez_timespec *end_time;
} Timer;

void TimerInit(Timer *assign_timer);
char TimerIsExpiredByDiff(Timer *assign_timer, unsigned int time_ms);
char TimerIsExpired(Timer *assign_timer);
void TimerCountdownMS(Timer *assign_timer, unsigned int time_count);
void TimerCountdown(Timer *assign_timer, unsigned int time_count);
int TimerLeftMS(Timer *assign_timer);
void TimerFini(Timer *assign_timer);

#endif