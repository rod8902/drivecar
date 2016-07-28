#ifndef _pals_env_h_
#define _pals_env_h_

#include <pals_time.h>
#include <pals_conf.h>

struct pals_env;
typedef struct pals_env pals_env_t;

extern pals_env_t *pals_initialize(const struct pals_conf *conf, int flags);

extern uint64_t pals_env_get_period(const pals_env_t *env);
extern const char *pals_env_get_name(const pals_env_t *env);

extern int pals_env_get_task_id(const pals_env_t *env, const char *name);
extern const char *pals_env_get_task_name(const pals_env_t *env, int id);

#endif
