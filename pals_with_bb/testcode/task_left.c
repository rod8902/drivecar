#include "controlInfo.h"

#define DIV	10	// rod

pals_rx_port_t *rx_port;
pals_tx_port_t *tx_port;

// routine for each period
int lv = 1500;	// 1500 ~ 1600
int rv = 1500;	// 1400 ~ 1500

int task_left(pals_task_t *task, int phase, void *arg){

		static int round;
		int ret;
		const pals_time_t *base_time, *start_time;
		int id = (long)arg;
		int i=0;

		int dir = 0;

		//int r_acc = 0, r_brk = 0, rot=90;
		int rot = 90;
		cntInfo info;

		int speed = 0;
		int dmin_lv = lv - 1500;

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
		ret = pals_recv(rx_port, &info, sizeof(info));
		if (ret < 0) {
				perror("recv");
				printf("task%d(%d): received(con%d) message = '%d %d %d'\n", id+1, round, i+1, info.acc, info.brk, info.rot);

		} else {
				printf("task%d(%d): received(con%d) message = '%d %d %d'\n", id+1, round, i+1, info.acc, info.brk, info.rot ); 
		}



		speed = speed + info.acc - info.brk;

		printf("acc = %d, brk = %d, rot = %d, speed = %d\n", info.acc, info.brk, info.rot, speed);
		rot = info.rot;
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
/*
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
*/		
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
