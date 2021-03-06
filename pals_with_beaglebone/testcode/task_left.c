#include "controlInfo.h"

pals_rx_port_t *rx_port;
pals_rx_port_t *right_rx_port;
pals_tx_port_t *tx_port;

int cv = 0;

cntInfo info;

int task_left(pals_task_t *task, int phase, void *arg){

		static int round;
		int ret;
		const pals_time_t *base_time, *start_time;
		int id = (long)arg;
		int i=0;

		int dv = 0;
		int goal = 0;

		int wheel_velocity = 0;
		int wheel_control = 0;

		int right_wheel_control = 0;
	
		int delta = 0;

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
		dv = (info.acc - info.brk);
		if( dv < 0 ){
			goal = 0;
		}else 
			goal = RATE*dv;

		if ( goal-cv > 0 ){
			delta = 1;
		}else if( goal - cv < 0){
			delta = -1;
		}else
			delta = 0;

		
		cv = cv + (goal - cv)/((dv > 0) ? DIV :(DIV*4)) + ((dv < 0) ? 2*dv : 0) + delta; //`* info.rot/180; 

		if(info.brk > 45){
			cv = 0;
		}

		if( cv < 0 ){
			cv = 0;
		}
		
		if(info.rot > 90){
			info.rot = 90;
		}
		wheel_velocity = cv * info.rot /90;

		wheel_control = 1500 + wheel_velocity;	
		
		printf("dv=%d, cv=%d, goal=%d, wheel_velocity=%d, wheel_control=%d, delta = %d\n", dv, cv, goal, wheel_velocity, wheel_control, delta);
/*
		ret = pals_recv(right_rx_port, &right_wheel_control, sizeof(right_wheel_control));
		if (ret < 0) {
				perror("recv from right");
				wheel_control = 1500;

		} else {
				printf("task%d(%d): received(con%d) message = '%d %d %d'\n", id+1, round, i+1, info.acc, info.brk, info.rot );
		}
*/
		ret = pals_send(tx_port, &wheel_control , sizeof(wheel_control));
		if ( ret < 0) {
				perror("send");
		} else {
				printf("task_left(%d): sent a message(len=%d)\n", round, sizeof(wheel_control));
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

		sprintf(name, "con4");

		right_rx_port = pals_rx_port_open(task, name);

		if (right_rx_port == NULL) {
				perror("rx port open\n");
				return -1;
		}

		pals_task_start(task);

		pals_task_join(task);
		return 0;
}
