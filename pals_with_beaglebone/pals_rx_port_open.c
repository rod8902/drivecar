#include <pals_port_i.h>
#include <pals_task_i.h>
#include <pals_env_i.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef PALS_DEBUG
#include <stdio.h>
#include <assert.h>
#endif

extern int sock_rx_open(int port);
extern int sock_mcast_rx_open(in_addr_t mcast_addr, int port);

static int gcd(int a, int b);
static int _task_mcast_rx_sock_open(struct pals_task *task);
static int _task_rx_sock_open(struct pals_task *task);

/**
 * pals_rx_port_open
 */
pals_rx_port_t *pals_rx_port_open(pals_task_t *task, const char *name)
{
    pals_rx_port_t *p;
    const pals_env_t *env = task->env;
    const struct pals_env_con *con;
    int ret;
    int msg_rate;
    const struct pals_env_task *sender;

    con = pals_env_find_con(env, name);

    if (con == NULL || !pals_env_con_is_peer(con, task->id)) {
	errno = EINVAL;
	return NULL;
    }

    p = calloc(1, sizeof(pals_rx_port_t));

    if (p == NULL) {
#ifdef PALS_DEBUG
	perror("calloc");
#endif
	return NULL;
    }

    p->task = task;
    p->con = con;

    if (task->rx_msg_len < con->len)
	task->rx_msg_len = con->len;

    sender = &env->tasks[con->sender];
    msg_rate = gcd(sender->rate, task->rate);
    p->period = env->period/msg_rate;
    p->s_rate = sender->rate/msg_rate;
    p->r_rate = task->rate/msg_rate;

    if (con->n_peers == 1) {
	ret = _task_rx_sock_open(task);
    } else {
	ret = _task_mcast_rx_sock_open(task);
    }

    if (ret < 0) {
#ifdef PALS_DEBUG
	perror("open_rx_sock");
#endif
	free(p);
	return NULL;
    }

    dl_list_init(&p->msg_list);

    task->rx_ports[con->id] = p;

    return p;
}

static int gcd(int a, int b)
{
    int mod = a%b;
    if (mod == 0)
	return b;
    else
	return gcd(b, mod);
}

static int _task_mcast_rx_sock_open(struct pals_task *task)
{
    int sock = task->mcast_rx_sock;
    const struct pals_env *env = task->env;

    if (sock > 0) {
	// aleady opened
	return sock;
    }

    if (env->mcast_addr == 0 || env->mcast_port == 0) {
	errno = EINVAL;
	return -1;
    }

    sock = sock_mcast_rx_open(env->mcast_addr, env->mcast_port);
    if (sock < 0) {
	return -1;
    }
    task->mcast_rx_sock = sock;

    return sock;
}

static int _task_rx_sock_open(struct pals_task *task)
{
    int sock = task->rx_sock;

    if (sock > 0) {
	// aleady opened
	return sock;
    }

    if (task->port <= 0) {
	errno = EINVAL;
	return -1;
    }

    sock = sock_rx_open(task->port);
    if (sock < 0) {
	return -1;
    }
    task->rx_sock = sock;

    return sock;
}
