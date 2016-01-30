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

//#define PERIOD 1000000000L   // 2.0 sec
#define PERIOD 1000000000L
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
	{.name = TASK2, .prio = 4, .ip_addr = "127.0.0.1", .port = 4322, .rate = 2, .offset = 0},
	{.name = TASK3, .prio = 4, .ip_addr = "127.0.0.1", .port = 4323, .rate = 3, .offset = 0},
	{.name = TASK4, .prio = 4, .ip_addr = "127.0.0.1", .port = 4324, .rate = 3, .offset = 0},
	{.name = TASK5, .prio = 4, .ip_addr = "127.0.0.1", .port = 4325, .rate = 3, .offset = 0}//////////////////////////////////////////////
};

#define NTASKS (sizeof(tasks)/sizeof(struct pals_conf_task))

// connections
struct pals_conf_con cons[] = {
	{.name = CON1, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK1, .n_peers = 0},
	{.name = CON2, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK2, .n_peers = 0},
	{.name = CON3, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK3, .n_peers = 0},
	{.name = CON4, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK4, .n_peers = 0},
	{.name = CON5, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK5, .n_peers = 0},////////////////////////////////////////////////////

};

#define NCONS (sizeof(cons)/sizeof(struct pals_conf_con))

// master configuration
struct pals_conf pals_conf = {
	.name = "multirate-comtest",
	.period = PERIOD,
	.mcast_addr = "226.1.1.1",
	.mcast_port = 4511,
	.n_tasks = 5,//origin:4//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	.tasks = tasks,
	.n_cons = 5,//origin:4//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	.cons = cons
};

pals_rx_port_t *rx_port[NCONS];
pals_tx_port_t *tx_port;

// routine for each period

int scnt=0;
int server_sockfd=-1, client_sockfd=-1, sockfd=-1;
int state, client_len;
int yes;
struct sockaddr_in clientaddr, serveraddr;

int acnt=0;

int tasklet_server(pals_task_t *task, int phase, void *arg){


	static int round[NTASKS];
	char buf[100]={0};
	int ret;
	int len;
	const pals_time_t *base_time, *start_time;
	int id = (long)arg;
	int i;


	char sbuf[13]={0};

	round[id]++;
	base_time = pals_task_get_base_time(task);
	start_time = pals_task_get_start_time(task);
	
	if(scnt==0){

		printf("task%d(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
				id+1, round[id], base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

		for (i = 0; i < NCONS; i++) {
			if (i == id) {
				sprintf(buf, "msg from task%d(%d)", id+1, round[id]);
				len = strlen(buf) + 1;
				ret = pals_send(tx_port, buf, len);
				if (ret < 0) {
					perror("send");
				} else {
					assert(ret == len);
					printf("task%d(%d): sent a message(len=%d)\n", id+1, round[id], len);
				}
			} else {
				ret = pals_recv(rx_port[i], buf, sizeof(buf));
				if (ret < 0) {
					perror("recv");
				} else {
					printf("task%d(%d): received(con%d) message = '%s'\n", id+1, round[id], i+1, buf);
				}
			}
		}
		if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket error : ");
			exit(0);
		}

		memset(&serveraddr, 0x00, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr("10.0.1.65");
		serveraddr.sin_port = htons(9000);

		setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		state = bind(server_sockfd, (struct sockaddr *)&serveraddr,
				sizeof(serveraddr));

		if (state == -1) {
			perror("bind error : ");
			exit(0);
		}

		state = listen(server_sockfd, 1);
		if (state == -1) {
			perror("listen error : ");
			exit(0);
		}

		client_sockfd = server_sockfd;

		fflush(stdout);

		client_len = sizeof(clientaddr);
		client_sockfd = accept(server_sockfd, (struct sockaddr*) &clientaddr, &client_len);

	}
	scnt++;

	if(scnt!=0){
		if (read(client_sockfd, sbuf, sizeof(sbuf)) > 0) {
			printf("read:%s\n",sbuf);
			printf("task%d(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
					id+1, round[id], base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

			for (i = 0; i < NCONS; i++) {
				if (i == id) {
					//sprintf(sbuf, "msg from task%d(%d)", id+1, round[id]);
					len = strlen(sbuf) + 1;
					ret = pals_send(tx_port, sbuf, len);
					if (ret < 0) {
						perror("send");
					} else {
						assert(ret == len);
						printf("task%d(%d): sent a message(len=%d)\n", id+1, round[id], len);
					}
				} else {
					ret = pals_recv(rx_port[i], buf, sizeof(buf));
					if (ret < 0) {
						perror("recv");
					} else {
						printf("task%d(%d): received(con%d) message = '%s'\n", id+1, round[id], i+1, buf);
					}
				}
			}

			//sprintf(buf, "msg from task%d(%d)", id+1, round[id]);
			len = strlen(buf) + 1;
			ret = pals_send(tx_port, buf, len);
		}
	}
	return 0;

}



int tasklet_acc(pals_task_t *task, int phase, void *arg)
{
	static int round[NTASKS];
	char buf[100] = {0};
	int ret;
	int len;
	const pals_time_t *base_time, *start_time;
	int id = (long)arg;
	int i;

	int d_acc = 0;

	round[id]++;
	base_time = pals_task_get_base_time(task);
	start_time = pals_task_get_start_time(task);

	printf("Accel Task \n");
	printf("task%d(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
			id+1, round[id], base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

	for (i = 0; i < NCONS; i++) {
		if (i == id) {	// id == 1
			//sprintf(buf, "msg from task%d(%d)", id+1, round[id]);
			printf("d_acc = %d\n", d_acc);
			sprintf(buf, "%d", d_acc);
			len = strlen(buf) + 1;
			ret = pals_send(tx_port, buf, len);
			if (ret < 0) {
				perror("send");
			} else {
				assert(ret == len);
				printf("task%d(%d): sent a message\n", id+1, round[id], len);
			}
		} else {
			ret = pals_recv(rx_port[i], buf, sizeof(buf));
			if (ret < 0) {
				perror("recv");
				if(i==2){	// No received msg from brake_task
					d_acc = 0;
				}
			} else {
				printf("%s from %d \n", buf, i);
				
				if(i==0){	// From server
					if((buf[0]='a' && buf[4]=='b')&& buf[8]=='r'){
						d_acc = (buf[1]-48)*100+(buf[2]-48)*10+(buf[3]-48);
					}
					printf("'%s' from server\n", buf);
				}else{
					printf("task%d(%d): received(con%d) message = '%s'\n", id+1, round[id], i+1, buf);
				}
			}
		}
	}

	return 0;
}
int tasklet_brk(pals_task_t *task, int phase, void *arg)
{
	static int round[NTASKS];
	char buf[100];
	int ret;
	int len;
	const pals_time_t *base_time, *start_time;
	int id = (long)arg;
	int i;

	int d_brk = 0;

	round[id]++;
	base_time = pals_task_get_base_time(task);
	start_time = pals_task_get_start_time(task);

	printf("Brake Task\n");
	printf("task%d(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
			id+1, round[id], base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

	for (i = 0; i < NCONS; i++) {
		if (i == id) {	// id == 2
			//sprintf(buf, "msg from task%d(%d)", id+1, round[id]);
			printf("d_brk = %d\n", d_brk);
			sprintf(buf, "%d", d_brk);
			len = strlen(buf) + 1;
			ret = pals_send(tx_port, buf, len);
			if (ret < 0) {
				perror("send");
			} else {
				assert(ret == len);
				printf("task%d(%d): sent a message(len=%d)\n", id+1, round[id], len);
			}
		} else {
			ret = pals_recv(rx_port[i], buf, sizeof(buf));
			if (ret < 0) {
				perror("recv");
			} else {
				if(i == 0){
					if((buf[0]='a' && buf[4]=='b') && buf[8]=='r'){
						d_brk = (buf[5]-48)*100+(buf[6]-48)*10+(buf[7]-48);
					}
					printf("'%s' from server\n",buf);
				}else{
					printf("task%d(%d): received(con%d) message = '%s'\n", id+1, round[id], i+1, buf);
				}
			}
		}
	}

	return 0;

}
int tasklet_rot(pals_task_t *task, int phase, void *arg)
{
	static int round[NTASKS];
	char buf[100];
	int ret;
	int len;
	const pals_time_t *base_time, *start_time;
	int id = (long)arg;
	int i;

	int d_rot = 0;

	round[id]++;
	base_time = pals_task_get_base_time(task);
	start_time = pals_task_get_start_time(task);

	printf("Rotate Task\n");
	printf("task%d(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
			id+1, round[id], base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

	for (i = 0; i < NCONS; i++) {
		if (i == id) {	// id == 3
			//sprintf(buf, "msg from task%d(%d)", id+1, round[id]);
			printf("d_rot = %d\n", d_rot);
			sprintf(buf, "%d", d_rot);
			len = strlen(buf) + 1;
			ret = pals_send(tx_port, buf, len);
			if (ret < 0) {
				perror("send");
			} else {
				assert(ret == len);
				printf("task%d(%d): sent a message(len=%d)\n", id+1, round[id], len);
			}
		} else {
			ret = pals_recv(rx_port[i], buf, sizeof(buf));
			if (ret < 0) {
				perror("recv");
			} else {
				if(i==0){	//From Server
					if((buf[0]='a' && buf[4]=='b') && buf[8]=='r'){
						d_rot = (buf[9]-48)*100+(buf[10]-48)*10+(buf[11]-48);
					}
					
					printf("'%s' from server\n",buf);
				}else{
					printf("task%d(%d): received(con%d) message = '%s'\n", id+1, round[id], i+1, buf);
				}

			}
		}
	}

	return 0;

}

int lv = 1500;	// 1500 ~ 1600
int rv = 1500;	// 1400 ~ 1500

int tasklet_ard(pals_task_t *task, int phase, void *arg){

	static int round[NTASKS];
	char buf[100];
	int ret;
	int len;
	const pals_time_t *base_time, *start_time;
	int id = (long)arg;
	int i;

	int r_acc = 0, r_brk = 0, rot=90;

	int dmax_lv = 1600-lv;
	int dmax_rv = rv - 1400;
	int dmin_lv = lv-1500;
	int dmin_rv = 1500 - rv;

	int w_fd = -1; //rod
	
	int speed = 0;
	
	// open file descriptor for xuart connection
	if(acnt==0){
		w_fd = open(XUART1, O_RDWR | O_NOCTTY );
		if( w_fd < 0 ){
			perror(XUART1);
			return -1;       
		}
	}
	//acnt++;

	round[id]++;
	base_time = pals_task_get_base_time(task);
	start_time = pals_task_get_start_time(task);

	printf("Arduino Task\n");

	printf("task%d(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
			id+1, round[id], base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

	for (i = 0; i < NCONS; i++) {
		if (i == id) {
			sprintf(buf, "msg from task%d(%d)", id+1, round[id]);
			len = strlen(buf) + 1;
			ret = pals_send(tx_port, buf, len);
			if (ret < 0) {
				perror("send");
			} else {
				assert(ret == len);
				printf("task%d(%d): sent a message(len=%d)\n", id+1, round[id], len);
			}
		} else {
			ret = pals_recv(rx_port[i], buf, sizeof(buf));
			if (ret < 0) {
				perror("recv");
			} else {
				switch(i){
					case 1:	
						r_acc = atoi(buf)/DIV;	
						break;
					case 2: 
						r_brk= atoi(buf)/DIV;
						break;
					case 3: 
						rot=atoi(buf);
						break;
					default: 
						printf("task%d(%d): received(con%d) message = '%s'\n", id+1, round[id], i+1, buf); 
						break;
				}
			}
		}
	}
	
	speed = speed + r_acc - r_brk;

	printf("acc = %d, brk = %d, rot = %d, speed = %d\n", r_acc, r_brk, rot, speed);
	if( speed != 0 ){
		if(rot >= 85 && rot <= 95){	// straight
			lv = lv + speed;
			rv = rv - speed;

			if(dmax_lv != dmax_rv){
				lv = 1500+(dmin_lv+dmin_rv)/2;
				rv = 1500-(dmin_lv+dmin_rv)/2;
			}					
		}else if(rot>=0 && rot<85){		// rot: 0~85
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
				rv = rv - rot*2;
			}else{	// left turn and speed_down
				lv = lv - (dmin_lv/(6 + speed));
				rv = rv + (dmin_rv/(6 + speed));
			}
		}else if(rot>95 && rot<=180){	// rot: 96~180
			if( speed > 0 ){
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
				lv = lv + (rot-90)*2; 
			}else{
				lv = lv - (dmin_lv/( 6 + speed));
				rv = rv + (dmin_rv/( 6 + speed));
			}						
		}
/*
		// reset
		if(speed < 0 && (lv ==1500 %% rv==1500)){
			speed = 0;
		}
*/			
	}else{		// speed is zero
		lv= lv-1;
		rv= rv+1;
		
	}
	
	printf(" lv = %d, rv = %d\n", lv, rv);

	if( lv > 1600 ){
		lv = 1600;
	}else if(lv < 1500){
		lv = 1500;
	}
	if( rv > 1500 ){
		rv = 1500;
	}else if(rv < 1400){
		rv = 1400;
	}
	
	ret = write(w_fd, &lv, sizeof(lv));
	ret = write(w_fd, &rv, sizeof(rv));

	return 0;

}

int main(int argc, char *argv[])
{
	pals_env_t *env;
	pals_task_t *task = NULL;
	char name[100];
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

	sprintf(name, "task%d", id+1);
	/*
	   if(id+1==1){
	//task = pals_task_open(env, name, tasklet1, (void*)(long)id);
	task = pals_task_open(env, name, tasklet_server, (void*)(long)id);
	}
	else if(id+1==2){
	task = pals_task_open(env, name, tasklet_b, (void*)(long)id);
	}
	 */
	switch (id+1) {
		case 1 : task = pals_task_open(env, name, tasklet_server, (void*)(long)id); break;
		case 2 : task = pals_task_open(env, name, tasklet_acc, (void*)(long)id); break;
		case 3 : task = pals_task_open(env, name, tasklet_brk, (void*)(long)id);; break;
		case 4 : task = pals_task_open(env, name, tasklet_rot, (void*)(long)id);; break;
		case 5 : task = pals_task_open(env, name, tasklet_ard, (void*)(long)id); break;
		default : ; break;
	}

	if (task == NULL) {
		perror("task open\n");
		return -1;
	}

	for (i=0; i<NCONS; i++) {
		sprintf(name, "con%d", i+1);
		if (i == id) {
			tx_port = pals_tx_port_open(task, name);
			if (tx_port == NULL) {
				perror("tx port open\n");
				return -1;
			}
		} else {
			rx_port[i] = pals_rx_port_open(task, name);
			if (rx_port[i] == NULL) {
				perror("rx port open\n");
				return -1;
			}
		}
	}

	pals_task_start(task);

	pals_task_join(task);
	return 0;
}
