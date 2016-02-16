#include "controlInfo.h"

pals_rx_port_t *rx_port;
pals_tx_port_t *tx_port;

int lv = 1500;	// 1500 ~ 1600
int cv = 0;

cntInfo info;

int task_left(pals_task_t *task, int phase, void *arg){

		static int round;
		int ret;
		const pals_time_t *base_time, *start_time;
		int id = (long)arg;
		int i=0;

		int rot = 90;

		int dv = 0;
		int goal = 0;
		int dev = 0;
		//int dmin_lv = lv - 1500;

		round++;
		base_time = pals_task_get_base_time(task);
		start_time = pals_task_get_start_time(task);

		printf("Left wheel Task\n");

		printf("task%d(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
						id+1, round, base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);
		ret = pals_recv(rx_port, &info, sizeof(info));
		if (ret < 0) {
				perror("recv");
				printf("task%d(%d): received(con%d) message = '%d %d %d'\n", id+1, round, i+1, info.acc, info.brk, info.rot);

		} else {
				printf("task%d(%d): received(con%d) message = '%d %d %d'\n", id+1, round, i+1, info.acc, info.brk, info.rot ); 
		}
		
		
		printf("acc = %d, brk = %d, rot = %d, dv = %d\n", info.acc, info.brk, info.rot, dv);
		
		//rot = info.rot;
		
		// dv is from +5 to -5
		dv = (info.acc - info.brk)/DIV;
		if(dv > 0){
			goal = RATE*dv;
		}else{
			goal = 0;
		}
		dev = info.rot/30 ;	// from 0 to 6
		cv = (cv + (goal - cv)/RATE) * info.rot/180; 

		if ( cv > goal){
			cv = goal;
		}else if (cv < 0){
			cv = 0;
		}
		
		lv = 1500 + cv;	
		printf("dv=%d, cv=%d, goal=%d, dev=%d, lv=%d\n", dv, cv, goal, dev, lv);

/*
		if( dv != 0 ){
				if(rot >= 85 && rot <= 95){	// straight
						lv = lv + dv;
						//좌회전에서 직진으로 변경시, 왼쪽바퀴의 속도를 이전 상태의 각도와 현재 각도 차 만큼 추가적으로 보정한다 
				}else if(rot>=0 && rot<85){		// Turn left
						lv = 1510 + RATE*(dv-1);
				}else if(rot>95 && rot<=180){	// Turn right
						lv = 1510 + (RATE-2)*(dv-1) + (18 - (rot*2)/DIV );
				}
		}else{		// dv is zero
				lv = lv-1;

		}
*/
		printf("pre_lv = %d\n", lv);

		if( lv > (1500+goal) ){
				lv = 1500+goal;
		}else if(lv < 1500){
				lv = 1500;
		}
		
		printf("post_lv = %d\n", lv);

		ret = pals_send(tx_port, &lv , sizeof(lv));
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
		//int i;
		int id=3;

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
