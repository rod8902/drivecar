#include <pals_task_i.h>
#include <errno.h>

int pals_task_add_phase(pals_task_t *task, uint64_t offset, pals_tasklet_t *tasklet, void *arg)
{
    int phase;
    uint64_t period;

    if (tasklet == NULL) {
	errno = EINVAL;
	return -1;
    }

    if (pals_task_is_active(task)) {
	errno = EPERM;
	return -1;
    }

    period = task->period;
    phase = task->num_phases;

    /*
    if (task->rate > 1) {
	// not allowed for multirate tasks
	errno = EPERM;
	return -1;
    }
    */

    if (phase >= PALS_MAX_PHASES) {
	// too many phases
	errno = ENOBUFS;
	return -1;
    }

    if (offset >= period ||
	    (phase > 0 && offset <= task->phases[phase-1].offset)) {
	// wrong offset time
	// pals_error("wrong offset time");
	errno = EINVAL;
	return -1;
    }

    task->phases[phase].phase = phase;
    task->phases[phase].offset = offset;
    task->phases[phase].tasklet = tasklet;
    task->phases[phase].arg = arg;
    task->num_phases++;

    // reset task's schedule delay limit
    task->sched_delay_limit = (period - offset)/2;

    return phase;
}
