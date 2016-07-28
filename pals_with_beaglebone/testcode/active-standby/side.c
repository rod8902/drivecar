#include <active-standby.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

enum {
		INACTIVE = 0,
		ACTIVE = 1,
		STANDBY = 2
};

int myside; // 1 or 2
int mystate = INACTIVE;
int other_state = INACTIVE;
int cmd;
pals_rx_port_t *cmd_port;
pals_tx_port_t *state_tx_port;
pals_rx_port_t *state_rx_port;

// routine for each period
int tasklet(pals_task_t *task, int phase, void *arg)
{
		static int round;
		int ret;
		const pals_time_t *base_time, *start_time;

		round++;
		base_time = pals_task_get_base_time(task);
		start_time = pals_task_get_start_time(task);

		printf("side%d(%d): (base_time={sec=%llu,nsec=%llu}, start_time={sec=%llu,nsec=%llu})\n",
						myside, round, base_time->sec, base_time->nsec, start_time->sec, start_time->nsec);

		ret = pals_recv(state_rx_port, &other_state, sizeof(other_state));
		if (ret < 0) {
				// no alive message
				other_state = INACTIVE;
				printf("  side%d: NO_MSG from %s\n", myside, (myside == 1) ? SIDE2 : SIDE1);
		}

		ret = pals_recv(cmd_port, &cmd, sizeof(cmd));
		if (ret < 0) {
				// no toggle message
				cmd = NO_MSG;
		}

		// decide current state
		if (mystate == other_state) {
				mystate = (myside == 1)? ACTIVE : STANDBY;
		} else if (mystate == INACTIVE) {
				// the other side alive aleady before me
				mystate = STANDBY;
				assert(other_state == ACTIVE);
		} else if (other_state == INACTIVE) {
				mystate = ACTIVE;
		} else if (cmd == TOGGLE) {
				// flip the state
				mystate = (mystate == ACTIVE)? STANDBY : ACTIVE;
				printf("  side%d: toggle to %s state\n", myside, (mystate==ACTIVE)? "ACTIVE" : "STANDBY");
		}
		printf("  side%d: My State = %s\n", myside, (mystate==ACTIVE)? "ACTIVE" : "STANDBY");

		// send my state to the other
		ret = pals_send(state_tx_port, &mystate, sizeof(mystate));
		if (ret < 0) {
				perror("send");
				return -1;
		}

		assert(ret == sizeof(mystate));

		return 0;
}

int main(int argc, char *argv[])
{
		pals_env_t *env;
		pals_task_t *task;

		if (argc != 2) {
				fprintf(stderr, "%s: need one argument\n", argv[0]);
				fprintf(stderr, "Usage: %s {1|2}\n", argv[0]);
				return -1;
		}

		myside = atoi(argv[1]);
		if (myside < 1 || myside > 2) {
				fprintf(stderr, "%s: wrong argument\n", argv[0]);
				fprintf(stderr, "Usage: %s {1|2}\n", argv[0]);
				return -1;
		}

		env = pals_initialize(&pals_conf, 0);

		if (env == NULL) {
				fprintf(stderr, "configuration error\n");
				return -1;
		}

		// create task
		task = pals_task_open(env, (myside == 1)? SIDE1 : SIDE2, tasklet, (void*)(long)myside);
		if (task == NULL) {
				fprintf(stderr, "task open error\n");
				return -1;
		}

		// create ports
		cmd_port = pals_rx_port_open(task, CON_CMD);
		state_rx_port = pals_rx_port_open(task, (myside == 1)? CON_STATE2 : CON_STATE1);
		state_tx_port = pals_tx_port_open(task, (myside == 1)? CON_STATE1 : CON_STATE2);

		if (cmd_port == NULL || state_tx_port == NULL || state_rx_port == NULL) {
				fprintf(stderr, "port open error\n");
				return -1;
		}

		// start the task
		pals_task_start(task);

		// wait for completion
		pals_task_join(task);
		return 0;
}
