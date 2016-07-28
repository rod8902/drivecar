#include <pals_port_i.h>
#include <pals_task_i.h>
#include <pals_env_i.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#ifdef PALS_DEBUG
#include <stdio.h>
#include <assert.h>
#endif

extern int sock_tx_open();
extern int sock_mcast_tx_open(unsigned char ttl);

static int _task_mcast_tx_sock_open(struct pals_task *task);
static int _task_tx_sock_open(struct pals_task *task);

/**
 * pals_tx_port_open
 */
pals_tx_port_t *pals_tx_port_open(pals_task_t *task, const char *name)
{
    pals_tx_port_t *p;
    const struct pals_env_con *con;
    pals_msg_t *msg;
    int ret;

    con = pals_env_find_con(task->env, name);

    if (con == NULL || task->id != con->sender) {
	errno = EINVAL;
	return NULL;
    }

    p = calloc(1, sizeof(pals_tx_port_t));

    if (p == NULL) {
#ifdef PALS_DEBUG
	perror("calloc");
#endif
	return NULL;
    }

    p->task = task;
    p->con = con;

    if (con->n_peers == 1) {
	ret = _task_tx_sock_open(task);
    } else {
	ret = _task_mcast_tx_sock_open(task);
    }
    if (ret < 0) {
#ifdef PALS_DEBUG
	perror("open_tx_sock");
#endif
	goto fail;
    }

    // alloc buffer for sending message
    p->msg = msg = (pals_msg_t *)calloc(1, sizeof(pals_msg_t) + con->len);
    if (msg == NULL) {
#ifdef PALS_DEBUG
	perror("calloc");
#endif
	goto fail;
    }
    msg->port_id = htons(con->id);
    msg->mode = htons(con->mode);
    msg->sender = htons(task->id);

    return p;

fail:
    free(p);
    return NULL;
}

static int _task_mcast_tx_sock_open(struct pals_task *task)
{
    int sock = task->mcast_tx_sock;
    const struct pals_env *env = task->env;

    if (sock > 0) {
	// aleady opened
	return sock;
    }

    if (env->mcast_addr == 0 || env->mcast_port == 0) {
	errno = EINVAL;
	return -1;
    }
    sock = sock_mcast_tx_open(env->mcast_ttl);
    if (sock < 0) {
	return -1;
    }
    task->mcast_tx_sock = sock;

    return sock;
}

static int _task_tx_sock_open(struct pals_task *task)
{
    int sock = task->tx_sock;

    if (sock > 0) {
	// aleady opened
	return sock;
    }

    sock = sock_tx_open();
    if (sock < 0) {
	return -1;
    }
    task->tx_sock = sock;

    return sock;
}
