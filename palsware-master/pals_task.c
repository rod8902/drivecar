#include <pals_task_i.h>

int pals_task_start(pals_task_t *task)
{
    pals_lock(&task->lock);
    task->status = PALS_STARTED;
    pthread_cond_signal(&task->start_signal);
    pals_unlock(&task->lock);
    return 0;
}

int pals_task_join(pals_task_t *task)
{
    return pthread_join(task->thread, NULL);
}

const char *pals_task_get_name(const pals_task_t *task)
{
    return task->name;
}

const pals_time_t *pals_task_get_pals_base_time(const pals_task_t *task)
{
    return &task->pals_base_time;
}

const pals_time_t *pals_task_get_base_time(const pals_task_t *task)
{
    return &task->base_time;
}

const pals_time_t *pals_task_get_start_time(const pals_task_t *task)
{
    return &task->start_time;
}
