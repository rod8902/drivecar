
#ifndef _pals_task_h_
#define _pals_task_h_

#include <pals_env.h>
#include <pals_time.h>

struct pals_task;
typedef struct pals_task pals_task_t;

typedef int pals_tasklet_t(struct pals_task *task, int phase, void *arg);

extern pals_task_t *pals_task_open(pals_env_t *env, const char* name, pals_tasklet_t *tasklet, void *arg);

extern int pals_task_add_phase(pals_task_t *task, uint64_t offset, pals_tasklet_t *tasklet, void *arg);

extern int pals_task_start(pals_task_t *task);

extern int pals_task_join(pals_task_t *task);

extern const char *pals_task_get_name(const pals_task_t *task);
extern const pals_time_t *pals_task_get_pals_base_time(const pals_task_t *task);
extern const pals_time_t *pals_task_get_base_time(const pals_task_t *task);
extern const pals_time_t *pals_task_get_start_time(const pals_task_t *task);

#endif
