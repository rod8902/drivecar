#ifndef CONTROLINFO_H_
#define CONTROLINFO_H_

#include <pals.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>


#define PERIOD 300000000L

#define TASKSERVER "task_server"
#define TASKARDU "task_arduino"
#define TASKLEFT "task_left"
#define TASKRIGHT "task_right"

#define CON1 "con1"
#define CON2 "con2"
#define CON3 "con3"
#define CON4 "con4"

#define XUART1 "/dev/ttyS0"
#define DIV	10
#define POSITIVE_RATE 10
#define NEGATIVE_RATE 30

struct pals_conf_task tasks[] = {
		{.name = TASKSERVER, .prio = 4, .ip_addr = "127.0.0.1", .port = 4321, .rate = 1, .offset = 0},
		{.name = TASKARDU, .prio = 4, .ip_addr = "127.0.0.1", .port = 4322, .rate = 1, .offset = 0},
		{.name = TASKLEFT, .prio = 4, .ip_addr = "127.0.0.1", .port = 4323, .rate = 1, .offset = 0},
		{.name = TASKRIGHT, .prio = 4, .ip_addr = "127.0.0.1", .port = 4324, .rate = 1, .offset = 0}
};

#define NTASKS (sizeof(tasks)/sizeof(struct pals_conf_task))

struct pals_conf_con cons[] = {
		{.name = CON1, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASKSERVER, .n_peers = 0},
		{.name = CON2, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASKARDU, .n_peers = 0},
		{.name = CON3, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASKLEFT, .n_peers = 0},
		{.name = CON4, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASKRIGHT, .n_peers = 0}
};

#define NCONS (sizeof(cons)/sizeof(struct pals_conf_con))

struct pals_conf pals_conf = {
		.name = "Drive car",
		.period = PERIOD,
		.mcast_addr = "226.1.1.1",
		.mcast_port = 4511,
		.n_tasks = NTASKS,
		.tasks = tasks,
		.n_cons = NCONS,
		.cons = cons
};
#pragma pack(1)
typedef struct cntInfo{
		int acc;
		int brk;
		int rot;
}cntInfo; // 컨트롤정보를 가지는 구조체
int makeSocketNonBlocking(int sock){
		int curFlags = fcntl(sock, F_GETFL, 0);
		return fcntl(sock, F_SETFL, curFlags | O_NONBLOCK) >= 0;
}
#endif //CONTROLINFO_H_
