#include <pals.h>
#include <stdio.h>
#include <assert.h>

//#define PERIOD 10000000L   // 10 msec
#define PERIOD 1000000000L   // 1.0 sec
#define TASK1	"task1"
#define TASK2	"task2"
#define CON1	"con1"

// tasks
struct pals_conf_task tasks[] = {
    {.name = TASK1, .prio = 4, .ip_addr = "127.0.0.1", .port = 4321, .offset = 0, .rate = 1},
    {.name = TASK2, .prio = 4, .ip_addr = "127.0.0.1", .port = 4322, .offset = PERIOD/2, .rate = 1}
};

// connections
struct pals_conf_con cons[] = {
    {.name = CON1, .len = 100, .mode = PALS_SAME_ROUND, .sender = TASK1, .n_peers = 1, .peers = (const char *[]){TASK2}}
};

// master configuration
struct pals_conf pals_conf = {
    .name = "comtest",
    .period = PERIOD,
    .mcast_addr = "226.1.1.1",
    .mcast_port = 4511,
    .n_tasks = 2,
    .tasks = tasks,
    .n_cons = 1,
    .cons = cons
};

pals_tx_port_t *port1;
pals_rx_port_t *port2;

// task1 routine for each period
int tasklet1(pals_task_t *task, int phase, void *arg)
{
    static int round;
    char buf[100];
    int ret;
    int len;
    const pals_time_t *base_time, *start_time;

    round++;
    base_time = pals_task_get_base_time(task);
    start_time = pals_task_get_start_time(task);

    printf("task1(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
	    round, base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

    sprintf(buf, "hello %d", round);
    len = strlen(buf) + 1;
    ret = pals_send(port1, buf, len);
    if (ret < 0) {
	perror("task1: send");
	return -1;
    }

    assert(ret == len);

    printf("task1: send a message(len=%d)\n", len);
    return 0;
}

// task2 routine for each period
int tasklet2(pals_task_t *task, int phase, void *arg)
{
    static int round;
    char buf[100];
    int ret;
    const pals_time_t *base_time, *start_time;

    round++;
    base_time = pals_task_get_base_time(task);
    start_time = pals_task_get_start_time(task);

    printf("task2(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
	    round, base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

    ret = pals_recv(port2, buf, sizeof(buf));
    if (ret < 0) {
	perror("task2: recv");
	return -1;
    }

    printf("task2: received message(len=%d) = '%s'\n", ret, buf);
    return 0;
}

int main()
{
    pals_env_t *env;
    pals_task_t *task1, *task2;
    pals_time_t time;

    env = pals_initialize(&pals_conf, 0);

    if (env == NULL) {
	fprintf(stderr, "configuration error\n");
	return -1;
    }

    pals_get_time(&time);
    printf("%s start at {sec=%lld, nsec=%lld}\n", pals_env_get_name(env), time.sec, time.nsec);

    task1 = pals_task_open(env, TASK1, tasklet1, (void*)1);
    task2 = pals_task_open(env, TASK2, tasklet2, (void*)2);
    if (task1 == NULL || task2 == NULL) {
	printf("task open error\n");
	return -1;
    }

    port1 = pals_tx_port_open(task1, CON1);
    port2 = pals_rx_port_open(task2, CON1);
    if (port1 == NULL || port2 == NULL) {
	printf("port open error\n");
	return -1;
    }

    pals_task_start(task1);
    pals_task_start(task2);

    pals_task_join(task1);
    pals_task_join(task2);

    return 0;
}
