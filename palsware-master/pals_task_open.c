#include <pals_task_i.h>
#include <pals_env_i.h>
#include <stdlib.h>
#include <errno.h>
#ifdef PALS_DEBUG
#include <stdio.h>
#include <assert.h>
#endif

extern void *_pals_task_main_posix(void *arg);

pals_task_t *pals_task_open(pals_env_t *env, const char* name,
	int (*tasklet)(pals_task_t *task, int phase, void *arg), void *arg)
{
    struct pals_task *task;
    pthread_attr_t attr;
    struct sched_param sched;
    int ret;
    const struct pals_env_task *env_task;
    uint64_t offset, period;
    int prio;

    if (env == NULL) {
	errno = EINVAL;
	return NULL;
    }

    env_task = pals_env_find_task(env, name);

    if (env_task == NULL) {
	errno = EINVAL;
	return NULL;
    }

    task = calloc(1, sizeof(struct pals_task));

    if (task == NULL) {
#ifdef PALS_DEBUG
	perror("malloc");
#endif
	return NULL;
    }

    task->env = env;
    task->name = env_task->name;
    task->id = env_task->id;
    task->addr = env_task->addr;
    task->port = env_task->port;
    task->num_phases = 0;

    task->rate = env_task->rate;
    task->period = period = env_task->period;
    task->offset = offset = env_task->offset;
    task->sched_delay_limit = (period - offset)/2;
#ifdef PALS_DEBUG
    assert(offset < period);
#endif

    if (env->n_cons > 0) {
	task->rx_ports = calloc(env->n_cons, sizeof(pals_rx_port_t *));
	if (task->rx_ports == NULL) {
#ifdef PALS_DEBUG
	    perror("malloc");
#endif
	    goto fail;
	}
    }

    if (tasklet != NULL) {
	ret = pals_task_add_phase(task, offset, tasklet, arg);
#ifdef PALS_DEBUG
	assert(ret == 0);
#endif
    }

    task->status = PALS_INACTIVE;
    task->phase = 0;

    pals_lock_init(&task->lock);
    pthread_cond_init(&task->start_signal, NULL);

    ret = pthread_attr_init(&attr);
    if (ret != 0) {
#ifdef PALS_DEBUG
	perror("pthread_attr_init");
#endif
	free(task);
	return NULL;
    }

    // scheduling priority
    prio = env_task->prio;
    if (prio > 0 && prio < 100) {
	pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	sched.sched_priority = prio;
	pthread_attr_setschedparam(&attr, &sched);
    } else {
	// SCHED_OTHER scheduling policy
	pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
	prio = 0;
    }
    task->prio = prio;

    ret = pthread_create(&task->thread, &attr, &_pals_task_main_posix, task);

    if (ret != 0) {
#ifdef PALS_DEBUG
	perror("pthread_create");
#endif
	goto fail;
    }

    return task;

fail:
    if (task != NULL) {
	if (task->rx_ports != NULL)
	    free(task->rx_ports);
	free(task);
    }
    return NULL;
}
