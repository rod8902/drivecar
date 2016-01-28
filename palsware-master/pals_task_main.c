#include <pals_task_i.h>
#include <pals_env_i.h>
#include <pals_port_i.h>
#ifdef PALS_DEBUG
#include <stdio.h>
#include <assert.h>
#endif

static int _pals_process_phases(struct pals_task *task)
{
    struct pals_phase *p;
    int phase;
    int ret;

    pals_time_t cur_time; // in nanoseconds
    pals_time_t start_time; // in nanoseconds

    phase = 0;
    p = &task->phases[0];

    while (1) {
	task->phase = phase;
	task->status = PALS_RUNNING;
	ret = p->tasklet(task, phase, p->arg);
	task->status = PALS_WAITING;

	// TODO: process return value

	if (++phase >= task->num_phases) {
	    break;
	}

	// set timer for the next phase
	p = &task->phases[phase];

	pals_time_add_ns(&start_time, &task->start_time, p->offset - (p-1)->offset);

	pals_get_time(&cur_time);

	if (pals_time_before(&start_time, &cur_time)) {
	    // schedule timing error
	    // TODO
	}

	// set one-shot timer for the next phase
	task->start_time = start_time;
	ret = pals_timer_start(&task->timer_phase, &start_time, 0);
#ifdef PALS_DEBUG
	assert(ret == 0);
#endif

	// wait for the next phase
	ret = pals_timer_wait(&task->timer_phase);
#ifdef PALS_DEBUG
	assert(ret == 0);
#endif
    }
    task->phase = 0;
    return 0;
}

#ifndef MAX
#define MAX(a,b)    (((a) >= (b))? (a) : (b))
#endif

static int _pals_task_main(struct pals_task *task)
{
    const uint64_t pals_period = task->env->period;
    const uint64_t task_period = task->period;
    int64_t sched_delay;    // in nanoseconds
    int ret;
    pals_time_t start_time;
    int multirate_cnt;

    // TODO: global sync to start?

    // prepare the rx message buffer
    if (task->rx_msg_len > 0) {
	task->rx_msg = (pals_msg_t *)calloc(1, sizeof(pals_msg_t) + task->rx_msg_len);
	if (task->rx_msg == NULL) {
#ifdef PALS_DEBUG
	    perror("calloc");
#endif
	    return -1;
	}
    }

    // initialize timers
    pals_timer_init(&task->timer);
    if (task->num_phases > 1)
	pals_timer_init(&task->timer_phase);
    if (task->rate > 1)
	pals_timer_init(&task->timer_multirate);

    pals_get_time(&start_time);
    // delay start time to acquire enough time for receiving messages sent at the previous period
    pals_time_add_ns(&start_time, &start_time, MAX(pals_period, 100000000L));

    pals_cal_base_time(&start_time, &start_time, pals_period);
    pals_time_add_ns(&start_time, &start_time, pals_period + task->offset);
    task->start_time = start_time;

    // start the timer
    ret = pals_timer_start(&task->timer, &start_time, pals_period);
#ifdef PALS_DEBUG
    assert(ret == 0);
#endif

    while (1) {
	// wait for next period
	task->status = PALS_WAITING;
	ret = pals_timer_wait(&task->timer);
#ifdef PALS_DEBUG
	assert(ret >= 0);
#endif
	if (ret > 0) {
	    // overruns occured
#ifdef PALS_DEBUG
	    fprintf(stderr, "%s(%lu): timer overruns(%d) occured(total=%lu))\n",
		    task->name, task->timer.round, ret, task->timer.missed);
#endif
	    // and just skip this period
	    // TODO?
	    task->timer.missed++;
	    continue;
	}

	// get current time
	pals_get_time(&start_time);

	// recalculate the current base time and start_time
	pals_cal_base_time(&task->pals_base_time, &start_time, pals_period);
	task->base_time = task->pals_base_time;
	pals_time_add_ns(&task->start_time, &task->pals_base_time, task->offset);

	// calculate schedule delay
	sched_delay = pals_time_diff_ns(&start_time, &task->start_time);
	if (sched_delay < 0) {
	    // unexpected condition
	    task->timer.missed++;
	    continue;
	}

	if (sched_delay > task->sched_delay_limit) {
	    task->timer.missed++;
	    continue;
	}
#ifdef PALS_DEBUG
	assert(sched_delay > 0 && sched_delay < (pals_period - task->offset));
#endif
	multirate_cnt = task->rate;
	if (multirate_cnt > 1) {
	    pals_time_add_ns(&start_time, &task->start_time, task_period);
	    ret = pals_timer_start(&task->timer_multirate, &start_time, task_period);
#ifdef PALS_DEBUG
	    assert(ret == 0);
#endif
	}

	while (1) {
	    task->sched_delay = sched_delay;
	    // process phases for each period
	    _pals_process_phases(task);

	    if (--multirate_cnt <= 0) {
		break;
	    }
	    task->status = PALS_WAITING;
	    ret = pals_timer_wait(&task->timer_multirate);
	    if (ret > 0) {
		multirate_cnt -= ret;
		if (multirate_cnt <=0)
		    break;
	    }
	    pals_time_add_ns(&task->base_time, &task->base_time, (ret+1)*task_period);
	    pals_time_add_ns(&task->start_time, &task->start_time, (ret+1)*task_period);
	}

	if (task->rate > 1) {
	    ret = pals_timer_stop(&task->timer_multirate);
#ifdef PALS_DEBUG
	    assert(ret == 0);
#endif
	}
    }
    return 0;
}

void *_pals_task_main_posix(void *arg)
{
    struct pals_task *task;
    int ret;

    task = (struct pals_task *)arg;

    // wait for start signal

    pals_lock(&task->lock);
    if (task->status == PALS_INACTIVE) {
	pthread_cond_wait(&task->start_signal, &task->lock);
    }
    pals_unlock(&task->lock);
#ifdef PALS_DEBUG
    assert(task->status == PALS_STARTED);
#endif

    ret = _pals_task_main(task);
    return (void *)(long)ret;
}
