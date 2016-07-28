#include <pals.h>
#include <stdio.h>

//#define PERIOD 100000000L    // 100 msec
#define PERIOD 1000000000L   // 1.0 sec
#define TASK1	"task1"
#define TASK2	"task2"

struct pals_conf_task tasks[] = {
    {.name = TASK1, .prio = 0, .ip_addr = "127.0.0.1", .port = 4321, .offset = 0, .rate = 1},
    {.name = TASK2, .prio = 0, .ip_addr = "127.0.0.1", .port = 4322, .offset = 0, .rate = 5}
};

struct pals_conf pals_conf = {
    .name = "simple_test",
    .period = PERIOD,
    .mcast_addr = "226.1.1.1",
    .mcast_port = 4511,
    .n_tasks = 2,
    .tasks = tasks,
    .n_cons = 0,
};

int tasklet(pals_task_t *task, int phase, void *arg)
{
    static int round[] = {0,0};
    long id = (long)arg;
    const pals_time_t *base_time, *start_time;

    if (phase == 0)
	round[id-1]++;

    base_time = pals_task_get_base_time(task);
    start_time = pals_task_get_start_time(task);

    printf("task%d(%d,%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
	    id, round[id-1], phase, base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

    return 0;
}

int main()
{
    pals_env_t *env;
    pals_task_t *task1, *task2;
    int ret;

    env = pals_initialize(&pals_conf, 0);
    if (env == NULL) {
	fprintf(stderr, "configuration error\n");
	return -1;
    }
    printf("%s\n", pals_env_get_name(env));

    task1 = pals_task_open(env, TASK1, tasklet, (void*)1);
    task2 = pals_task_open(env, TASK2, tasklet, (void*)2);

    if (task1 == NULL || task2 == NULL) {
	fprintf(stderr, "task open error\n");
	return -1;
    }

    ret = pals_task_add_phase(task1, PERIOD/2, tasklet, (void*)1);
    if (ret < 0) {
	fprintf(stderr, "task_add_phase error\n");
	return -1;
    }

    pals_task_start(task1);
    pals_task_start(task2);

    pals_task_join(task1);
    pals_task_join(task2);
    return 0;
}
