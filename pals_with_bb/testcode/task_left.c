#include <pals.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <fcntl.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <ifaddrs.h>

//#define PERIOD 1000000000L   // 2.0 sec
#define PERIOD 100000000L



#define TASK1	"task1"
#define TASK2	"task2"
#define TASK3	"task3"
#define TASK4	"task4"
#define TASK5	"task5"

#define CON1	"con1"
#define CON2	"con2"
#define CON3	"con3"
#define CON4	"con4"
#define CON5	"con5"


#define XUART1 "/dev/ttyO1"
#define DIV	10	// rod

// tasks
struct pals_conf_task tasks[] = {
		{.name = TASK1, .prio = 4, .ip_addr = "127.0.0.1", .port = 4321, .rate = 1, .offset = 0},
		{.name = TASK2, .prio = 4, .ip_addr = "127.0.0.1", .port = 4322, .rate = 1, .offset = 0},
		{.name = TASK3, .prio = 4, .ip_addr = "127.0.0.1", .port = 4323, .rate = 1, .offset = 0},
		{.name = TASK4, .prio = 4, .ip_addr = "127.0.0.1", .port = 4324, .rate = 1, .offset = 0},
		{.name = TASK5, .prio = 4, .ip_addr = "127.0.0.1", .port = 4325, .rate = 1, .offset = 0}	// new
};

#define NTASKS (sizeof(tasks)/sizeof(struct pals_conf_task))

// connections
struct pals_conf_con cons[] = {
		{.name = CON1, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK1, .n_peers = 0},
		{.name = CON2, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK2, .n_peers = 0},
		{.name = CON3, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK3, .n_peers = 0},
		{.name = CON4, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK4, .n_peers = 0},
		{.name = CON5, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK5, .n_peers = 0}	// new
};

#define NCONS (sizeof(cons)/sizeof(struct pals_conf_con))
//int NCONS;
// master configuration
struct pals_conf pals_conf = {
		.name = "multirate-comtest with active-standby",
		.period = PERIOD,
		.mcast_addr = "226.1.1.1",
		.mcast_port = 4511,
		.n_tasks = 5,	// origin: 4
		.tasks = tasks,
		.n_cons = 5,	//origin:4	
		.cons = cons
};

pals_rx_port_t *rx_port;
pals_tx_port_t *tx_port;

// routine for each period

int scnt=0;	// 1st time, To connect andriod client for tasklet_server
int server_sockfd=-1, client_sockfd=-1, sockfd=-1;
int state, client_len;
int yes;
struct sockaddr_in clientaddr, serveraddr;

int acnt=0;

int lv = 1500;	// 1500 ~ 1600
int rv = 1500;	// 1400 ~ 1500

int task_left(pals_task_t *task, int phase, void *arg){

		static int round;
		char buf[100]={0};
		int ret;
		int len;
		const pals_time_t *base_time, *start_time;
		int id = (long)arg;
		int i;

		int dir = 0;

		int r_acc = 0, r_brk = 0, rot=90;

		int dmax_lv = 1600-lv;
		int dmax_rv = rv - 1400;
		int dmin_lv = lv-1500;
		int dmin_rv = 1500 - rv;

		struct ctlInfo info;

		int speed = 0;

		// open file descriptor for xuart connection
		round++;
		base_time = pals_task_get_base_time(task);
		start_time = pals_task_get_start_time(task);

		printf("Left wheel Task\n");

		printf("task%d(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
						id+1, round, base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);
		/*
		   sprintf(buf, "msg from task%d(%d)", id+1, round[id]);
		   len = strlen(buf) + 1;
		   ret = pals_send(tx_port, buf, len);
		   if (ret < 0) {
		   perror("send");
		   else {
		   assert(ret == len);
		   printf("task%d(%d): sent a message(len=%d)\n", id+1, round[id], len);
		   }
		 */		
		ret = pals_recv(rx_port, info, sizeof(info));
		if (ret < 0) {
				perror("recv");
				printf("task%d(%d): received(con%d) message = '%d %d %d'\n", id+1, round, i+1, info->acc, info->brk, info->rot);

		} else {

				printf("task%d(%d): received(con%d) message = '%d %d %d'\n", id+1, round, i+1, info->acc, info->brk, info->rot ); 


		}



		speed = speed + info->acc - info->brk;

		printf("acc = %d, brk = %d, rot = %d, speed = %d\n", info->acc, info->brk, info->rot, speed);
		if( speed != 0 ){
				if(rot >= 85 && rot <= 95){	// straight

						lv = lv + speed;
						//rv = rv - speed;
						/*
						   if(dmax_lv != dmax_rv){
						   lv = 1500+(dmin_lv+dmin_rv)/2;
						   rv = 1500-(dmin_lv+dmin_rv)/2;
						   }
						 */
						if(dir == 1) {
								lv = 1500+(dmin_lv+90-rot);
								//좌회전에서 직진으로 변경시, 왼쪽바퀴의 속도를 이전 상태의 각도와 현재 각도 차 만큼 추가적으로 보정한다 
						}
						dir = 0;
						//직진
				}else if(rot>=0 && rot<85){		// rot: 0~85
						dir = 1;
						//좌회전
						if( speed > 0 ){
								if( lv > 1510+rot ){
										lv = lv - (90-rot) - speed;
										if( lv <=1510+rot ){
												lv = 1510+rot;
										}
								}else if( lv < 1510+rot ){
										lv = lv + (90-rot) + speed;
										if( lv >=1510+rot ){
												lv = 1510+rot;
										}
								}
								//rv = rv - rot*2;
						}else{	// left turn and speed_down
								lv = lv - (dmin_lv/(6 + speed));
								//rv = rv + (dmin_rv/(6 + speed));
						}
				}else if(rot>95 && rot<=180){	// rot: 96~180
						dir = 2;
						//우회전
						if( speed > 0 ){
								/*
								   if( rv > 1400+(rot-90) ){
								   rv = rv - (rot-90) - speed;
								   if( rv <= 1400+(rot-90) ){
								   rv = 1400+(rot-90);
								   }
								   }else if( rv < 1400+(rot-90) ){
								   rv = rv + (rot-90) + speed;
								   if( rv >= 1400+(rot-90) ){
								   rv = 1400+(rot-90);
								   }
								   }
								 */
								lv = lv + (rot-90)*2; 
						}else{
								lv = lv - (dmin_lv/( 6 + speed));
								//rv = rv + (dmin_rv/( 6 + speed));
						}						
				}
		}else{		// speed is zero
				lv= lv-1;
				//rv= rv+1;

		}

		printf(" lv = %d\n", lv);

		if( lv > 1600 ){
				lv = 1600;
		}else if(lv < 1500){
				lv = 1500;
		}
		/*
		   if( rv > 1500 ){
		   rv = 1500;
		   }else if(rv < 1400){
		   rv = 1400;
		   }
		 */
		//	ret = write(w_fd, &lv, sizeof(lv));
		//	ret = write(w_fd, &rv, sizeof(rv));

		ret = pals_send(tx_port, lv , sizeof(lv));
		if ( ret < 0) {
				perror("send");
		} else {
				printf("task_left(%d): sent a message(len=%d)\n", round, sizeof(lv));
		}

		return 0;

}

int main(int argc, char *argv[])
{
		pals_env_t *env;
		pals_task_t *task = NULL;
		char name[100] = {0};
		pals_time_t time;
		int i;
		int id;

		if (argc != 2) {
				fprintf(stderr, "%s: need one argument\n", argv[0]);
				fprintf(stderr, "Usage: %s {1..%d}\n", argv[0], NTASKS);
				return -1;
		}

		id = atoi(argv[1]);
		if (id < 1 || id > NTASKS) {
				fprintf(stderr, "%s: wrong argument\n", argv[0]);
				fprintf(stderr, "Usage: %s {1..%d}\n", argv[0], NTASKS);
				return -1;
		}
		id--;

		env = pals_initialize(&pals_conf, 0);

		if (env == NULL) {
				fprintf(stderr, "configuration error\n");
				return -1;
		}

		pals_get_time(&time);
		printf("%s start at {sec=%lld, nsec=%lld}\n", pals_env_get_name(env), time.sec, time.nsec);

		sprintf(name, "task_left");

		task = pals_task_open(env, name, task_left, (void*)(long)id);

		if (task == NULL) {
				perror("task open\n");
				return -1;
		}


		sprintf(name, "con3");


		tx_port = pals_tx_port_open(task, name);
		if (tx_port == NULL) {
				perror("tx port open\n");
				return -1;
		}

		spirntf(name, "con1");

		rx_port = pals_rx_port_open(task, name);
		if (rx_port == NULL) {
				perror("rx port open\n");
				return -1;
		}



		pals_task_start(task);

		pals_task_join(task);
		return 0;
}
