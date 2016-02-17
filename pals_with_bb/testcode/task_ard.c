#include "controlInfo.h"

pals_rx_port_t *right_port;
pals_rx_port_t *left_port;

int task_arduino(pals_task_t *task, int phase, void *arg)
{
		static int round;
		int ret;
		const pals_time_t *base_time, *start_time;

		//char bufl[100];
		//char bufr[100];

		static int lv = 1500;
		static int rv = 1500;

		int w_fd = -1;

		// open file descriptor for xuart connection
		w_fd = open(XUART1, O_RDWR | O_NOCTTY );
		if( w_fd < 0 ){
				perror(XUART1);
				return -1;       
		}

		round++;
		base_time = pals_task_get_base_time(task);
		start_time = pals_task_get_start_time(task);

		printf("task_Arduino(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
						round, base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

		ret = pals_recv(left_port, &lv, sizeof(lv));
		if (ret < 0) {
				printf("no left msg");
				//lv = 1500;
		}else{
				printf("lv:%d\n",lv);
		}
		ret = pals_recv(right_port, &rv, sizeof(rv));
		if (ret < 0) {
				printf("no right msg");
				//rv = 1500;
		}else{
				printf("rv:%d\n",rv);
		}

		write(w_fd, &lv, sizeof(lv));
		write(w_fd, &rv, sizeof(rv));

		close(w_fd);
		return 0;
}

int main(int argc, char *argv[])
{
		pals_env_t *env;
		pals_task_t *task;

		env = pals_initialize(&pals_conf, 0);

		if (env == NULL) {
				fprintf(stderr, "configuration error\n");
				return -1;
		}

		task = pals_task_open(env, TASKARDU, task_arduino, (void*)(long)4);
		if (task == NULL) {
				fprintf(stderr, "task open error\n");
				return -1;
		}

		// create ports
		right_port = pals_rx_port_open(task, CON4);
		left_port = pals_rx_port_open(task, CON3);

		if (right_port == NULL || left_port == NULL) {
				fprintf(stderr, "port open error\n");
				return -1;
		}

		pals_task_start(task);

		// wait for completion
		pals_task_join(task);
		return 0;

}

