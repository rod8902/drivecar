#include <pals_env_i.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef PALS_DEBUG
#include <stdio.h>
#include <assert.h>
#endif

const struct pals_env_task *pals_env_find_task(const pals_env_t *env, const char *name);

pals_env_t *pals_initialize(const struct pals_conf *conf, int flags)
{
    pals_env_t *env;
    int i, n;
    uint64_t pals_period;
    int rate;
    const struct pals_env_task *task;

    env = (pals_env_t *)calloc(1, sizeof(pals_env_t));

    if (env == NULL) {
#ifdef PALS_DEBUG
	perror("calloc");
#endif
	return NULL;
    }

    env->name = conf->name;
    env->period = pals_period = conf->period;
    env->mode = flags;
    if (conf->mcast_addr != NULL) {
	env->mcast_addr = inet_addr(conf->mcast_addr);
	env->mcast_port = conf->mcast_port;
	env->mcast_ttl = conf->mcast_ttl;
    } else {
	env->mcast_addr = 0;
	env->mcast_port = 0;
	env->mcast_ttl = 0;
    }
    // TODO: check values

    env->n_tasks = 0;
    n = conf->n_tasks;
    if (n > 0) {
	env->tasks = (struct pals_env_task *)calloc(n, sizeof(struct pals_env_task));
	if (env->tasks == NULL) {
	    errno = EINVAL;
	    goto fail;
	}
	for (i = 0; i < n; i++) {
	    // check name dupulication
	    task = pals_env_find_task(env, conf->tasks[i].name);
	    if (task != NULL) {
		// a same name task is aleady exists
		errno = EINVAL;
		goto fail;
	    }
	    env->tasks[i].id = i;	// index
	    env->tasks[i].name = conf->tasks[i].name;
	    env->tasks[i].prio = conf->tasks[i].prio;
	    env->tasks[i].addr = inet_addr(conf->tasks[i].ip_addr);
	    env->tasks[i].port = conf->tasks[i].port;
	    env->tasks[i].offset = conf->tasks[i].offset;
	    env->tasks[i].rate = rate = conf->tasks[i].rate;


	    if (rate == 1 || rate == 0) {
		env->tasks[i].period = pals_period;
		if (rate == 0)
		    env->tasks[i].rate = 1;
	    } else if (rate > 1) {
		// multirate task
		env->tasks[i].period = pals_period/rate;
	    } else {
		errno = EINVAL;
		goto fail;
	    }

	    if (env->tasks[i].offset >= env->tasks[i].period) {
		errno = EINVAL;
		goto fail;
	    }
	    env->n_tasks++;
	}
#ifdef PALS_DEBUG
	assert(env->n_tasks == n);
#endif
    }

    n = env->n_cons = conf->n_cons;
    if (n > 0) {
	env->cons = (struct pals_env_con *)calloc(n, sizeof(struct pals_env_con));
	if (env->cons == NULL) {
	    errno = EINVAL;
	    goto fail;
	}
	for (i = 0; i < n; i++) {
	    int n_peers, ii;
	    struct pals_env_con *pcon = &env->cons[i];

	    pcon->id = i;	// index
	    pcon->name = conf->cons[i].name;
	    pcon->len = conf->cons[i].len;
	    pcon->mode = conf->cons[i].mode;
	    task = pals_env_find_task(env, conf->cons[i].sender);
	    if (task == NULL) {
		errno = EINVAL;
		goto fail;
	    }

	    pcon->sender = task->id;
	    pcon->n_peers = n_peers = conf->cons[i].n_peers;

	    if (n_peers != 1) {
		// multicasting
		pcon->mode |= PALS_PORT_MCAST;
	    }

	    if (n_peers <= 0 ) {
		continue;
	    }

	    pcon->peers = (const struct pals_env_task **)calloc(n_peers, sizeof(struct pals_env_task *));
	    for (ii = 0; ii < n_peers; ii++) {
		task = pals_env_find_task(env, conf->cons[i].peers[ii]);
		if (task == NULL) {
		    errno = EINVAL;
		    goto fail;
		}
		pcon->peers[ii] = task;
	    }
	}
    }

    return env;

fail:
    if (env->tasks)
	free(env->tasks);
    if (env->cons) {
	for (i = 0; i < env->n_cons; i++) {
	    if (env->cons[i].peers)
		free(env->cons[i].peers);
	}
	free(env->cons);
    }
    free(env);
    return NULL;
}

const struct pals_env_task *pals_env_find_task(const pals_env_t *env, const char *name)
{
    int i;

    if (name == NULL)
	return NULL;

    for (i = 0; i < env->n_tasks; i++) {
	if (strcmp(name, env->tasks[i].name) == 0) {
	    return &env->tasks[i];
	}
    }

    return NULL;
}

const struct pals_env_con *pals_env_find_con(const pals_env_t *env, const char *name)
{
    int i;

    if (name == NULL)
	return NULL;

    for (i = 0; i < env->n_cons; i++) {
	if (strcmp(name, env->cons[i].name) == 0) {
	    return &env->cons[i];
	}
    }

    return NULL;
}

uint64_t pals_env_get_period(const struct pals_env *env)
{
    return env->period;
}

const char *pals_env_get_name(const pals_env_t *env)
{
    return env->name;
}

int pals_env_get_task_id(const pals_env_t *env, const char *name)
{
    const struct pals_env_task *task;

    task = pals_env_find_task(env, name);
    if (task == NULL) {
	return -1;
    }
    return task->id;
}

const char *pals_env_get_task_name(const pals_env_t *env, int id)
{
    if (id < 0 || id >= env->n_tasks) {
	return NULL;
    }
    return env->tasks[id].name;
}

int pals_env_con_is_peer(const struct pals_env_con *pe, int task_id)
{
    int i, n_peers;

    n_peers = pe->n_peers;
    if (n_peers <= 0) {
	return 1;
    }
    for (i = 0; i < pe->n_peers; i++) {
	if (task_id == pe->peers[i]->id)
	    return 1;
    }
    return 0;
}
