#include "controlInfo.h"

pals_rx_port_t *rx_port;
pals_tx_port_t *tx_port;

int rv = 1500;	// 1400 ~ 1500
int cv = 0;	// 현재 속도

cntInfo info;

int task_right(pals_task_t *task, int phase, void *arg){

		static int round;
		int ret;
		const pals_time_t *base_time, *start_time;
		int id = (long)arg;
		int i = 0;

		int rot = 90;

		int dv = 0;	// 가속 수준
		int dev = 0;	// 편차

		int goal=0;

		round++;
		base_time = pals_task_get_base_time(task);
		start_time = pals_task_get_start_time(task);

		printf("Right wheel Task\n");

		printf("task%d(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
						id+1, round, base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);
		ret = pals_recv(rx_port, &info, sizeof(info));
		if (ret < 0) {
				perror("recv");
				printf("task%d(%d): received(con%d) message = '%d %d %d'\n", id+1, round, i+1, info.acc, info.brk, info.rot); 

		}
		else {
				printf("task%d(%d): received(con%d) message = '%d %d %d'\n", id+1, round, i+1, info.acc, info.brk, info.rot); 

		}
		//rot = info.rot;

		printf("acc = %d, brk = %d, rot = %d, dv = %d\n", info.acc, info.brk, info.rot, dv);
	
		// dv is from +5 to -5
		dv = (info.acc - info.brk)/DIV;
		if(dv > 0){
			goal = RATE*dv;
		}else{
			goal = 0;
		}
		dev = 6 - info.rot/30 ;	// from 0 to 6
		cv = (cv + (goal - cv)/RATE) * dev; 

		if( cv > goal){
			cv = goal;
		}else if (cv < 0){
			cv =0;
		}

		rv = 1490 - cv;	
/*
		if( dv != 0 ){
				if(rot >= 85 && rot <= 95){	// straight
						rv = rv - dv;

						//우회전에서 직진으로 변경시, 오른쪽 바퀴의 속도를 이전 상태의 각도와 현개 각도 차 만큼 추가적으로 보정한다
				}else if(rot>=0 && rot<85){		// Turn left
					rv = 1490 - RATE*(dv-1);
				}else if(rot>95 && rot<=180){	// rot: 96~180
					rv = 1490 - RATE*(dv-1) - (((rot-90)*2)/DIV + 1);
				}
				
		}else{		// dv is zero
				rv= rv+1;
		}
*/
		printf("pre_rv = %d\n", rv);

		if( rv > 1500 ){
			rv = 1500;
		}else if(rv < 1500-RATE*dv){
			rv = 1500-RATE*dv;
		}

		printf("post_rv = %d\n", rv);
		
		ret = pals_send(tx_port, &rv, sizeof(rv));
		if (ret < 0) {
				perror("send");
		} else {
				printf("task_right(%d): sent a message(len=%d)\n", round, sizeof(rv));
		}

		return 0;
}

int main(int argc, char *argv[])
{
		pals_env_t *env;
		pals_task_t *task = NULL;
		char name[100] = {0};
		pals_time_t time;
		//int i;
		int id = 4;
		id--;

		env = pals_initialize(&pals_conf, 0);

		if (env == NULL) {
				fprintf(stderr, "configuration error\n");
				return -1;
		}

		pals_get_time(&time);
		printf("%s start at {sec=%lld, nsec=%lld}\n", pals_env_get_name(env), time.sec, time.nsec);

		sprintf(name, "task_right");

		task = pals_task_open(env, name, task_right, (void*)(long)id);

		if (task == NULL) {
				perror("task open\n");
				return -1;
		}


		sprintf(name, "con4");

		tx_port = pals_tx_port_open(task, name);
		if (tx_port == NULL) {
				perror("tx port open\n");
				return -1;
		}

		sprintf(name, "con1");
		rx_port = pals_rx_port_open(task, name);
		if (rx_port == NULL) {
				perror("rx port open\n");
				return -1;
		}

		pals_task_start(task);

		pals_task_join(task);
		return 0;
}
