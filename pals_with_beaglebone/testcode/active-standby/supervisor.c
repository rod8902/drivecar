#include <active-standby.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

int cmd;
pals_tx_port_t *cmd_port;

static int toggle_pressed(void)
{
    static char buf[1024];
    const char *ret;

    ret = fgets(buf, sizeof(buf), stdin);

    if (ret != buf) {
	return 0;
    }
    return 1;
}

// routine for each period
int tasklet(pals_task_t *task, int phase, void *arg)
{
    static int round;
    int ret;
    const pals_time_t *base_time, *start_time;

    round++;
    base_time = pals_task_get_base_time(task);
    start_time = pals_task_get_start_time(task);

    printf("supervisor(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
	    round, base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

    if (toggle_pressed()) {
	// send my state to the other
	cmd = TOGGLE;
	printf("  toggle switch pressed\n");
	ret = pals_send(cmd_port, &cmd, sizeof(cmd));
	if (ret < 0) {
	    perror("send");
	    return -1;
	}
	assert(ret == sizeof(cmd));
    }

    return 0;
}

int main(int argc, char *argv[])
{
    pals_env_t *env;
    pals_task_t *task;
    int flags;

    env = pals_initialize(&pals_conf, 0);

    if (env == NULL) {
	fprintf(stderr, "configuration error\n");
	return -1;
    }

    // set non blocking stdin
    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // create task
    task = pals_task_open(env, SUPERVISOR, tasklet, 0);
    if (task == NULL) {
	fprintf(stderr, "task open error\n");
	return -1;
    }

    // create ports
    cmd_port = pals_tx_port_open(task, CON_CMD);

    if (cmd_port == NULL) {
	fprintf(stderr, "port open error\n");
	return -1;
    }

    // start the task
    pals_task_start(task);

    // wait for completion
    pals_task_join(task);
    return 0;
}
