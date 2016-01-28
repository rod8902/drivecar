#include <pals.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define PERIOD 2000000000L   // 2.0 sec
#define TASK1	"task1"
#define TASK2	"task2"
#define TASK3	"task3"
#define TASK4	"task4"
#define CON1	"con1"
#define CON2	"con2"
#define CON3	"con3"
#define CON4	"con4"

// tasks
struct pals_conf_task tasks[] = {
		{.name = TASK1, .prio = 4, .ip_addr = "127.0.0.1", .port = 4321, .rate = 1, .offset = 0},
		{.name = TASK2, .prio = 4, .ip_addr = "127.0.0.1", .port = 4322, .rate = 2, .offset = 0},
		{.name = TASK3, .prio = 4, .ip_addr = "127.0.0.1", .port = 4323, .rate = 3, .offset = 0},
		{.name = TASK4, .prio = 4, .ip_addr = "127.0.0.1", .port = 4324, .rate = 3, .offset = 0}
};

#define NTASKS (sizeof(tasks)/sizeof(struct pals_conf_task))

// connections
struct pals_conf_con cons[] = {
		{.name = CON1, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK1, .n_peers = 0},
		{.name = CON2, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK2, .n_peers = 0},
		{.name = CON3, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK3, .n_peers = 0},
		{.name = CON4, .len = 100, .mode = PALS_NEXT_ROUND, .sender = TASK4, .n_peers = 0},
};

#define NCONS (sizeof(cons)/sizeof(struct pals_conf_con))

// master configuration
struct pals_conf pals_conf = {
		.name = "multirate-comtest",
		.period = PERIOD,
		.mcast_addr = "226.1.1.1",
		.mcast_port = 4511,
		.n_tasks = 4,
		.tasks = tasks,
		.n_cons = 4,
		.cons = cons
};

pals_rx_port_t *rx_port[NCONS];
pals_tx_port_t *tx_port;

// routine for each period
int tasklet(pals_task_t *task, int phase, void *arg)
{
		static int round[NTASKS];
		char buf[100];
		int ret;
		int len;
		const pals_time_t *base_time, *start_time;
		int id = (long)arg;
		int i;

		round[id]++;
		base_time = pals_task_get_base_time(task);
		start_time = pals_task_get_start_time(task);

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

		return 0;
}

int main(int argc, char *argv[])
{
		pals_env_t *env;
		pals_task_t *task;
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
		task = pals_task_open(env, name, tasklet, (void*)(long)id);
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
