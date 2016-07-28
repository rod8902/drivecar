#include <pals_port_i.h>
#include <pals_task_i.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#ifdef PALS_DEBUG
#include <assert.h>
#endif

extern int sock_sendto(int sock, void *buf, int len, in_addr_t addr, int port);
extern int sock_mcast_send(int sock, void *buf, int len, in_addr_t mcast_addr, int port);

/*
 * pals_send
 */
int pals_send(pals_tx_port_t *port, const void *buf, size_t len)
{
    int ret;
    pals_msg_t *msg;
    size_t msg_len;
    struct pals_task *task;

    if (len > port->con->len) {
	errno = EMSGSIZE;
	return -1;
    }

    msg = port->msg;
    task = port->task;

#ifdef PALS_DEBUG
    assert(task->id == port->con->sender && ntohs(msg->sender) == task->id);
#endif

    //msg->mode = htons((uint16_t)port->con->mode);
    //msg->sender = htons((uint16_t)task->id);
    //msg->port_id = htons((uint16_t)port->con->id);
    msg->len = htons((uint16_t)len);

    pals_time_hton(&msg->base_time, &task->base_time);
    pals_time_hton(&msg->start_time, &task->start_time);

    if (buf != NULL && len > 0)
	memcpy(&msg->data, buf, len);

    msg_len = sizeof(pals_msg_t) + len;

    if (port->con->mode & PALS_PORT_MCAST) {
	if (task->mcast_tx_sock > 0) {
	    ret = sock_mcast_send(task->mcast_tx_sock, msg, msg_len,
		    task->env->mcast_addr, task->env->mcast_port);
	} else {
	    errno = EINVAL;
	    ret = -1;
	}
    } else {
	const struct pals_env_task *peer;
#ifdef PALS_DEBUG
	assert(port->con->n_peers == 1);
#endif
	peer = port->con->peers[0];
	if (task->tx_sock > 0 && peer->addr != 0 && peer->port > 0) {
	    ret = sock_sendto(task->tx_sock, msg, msg_len, peer->addr, peer->port);
	} else {
	    errno = EINVAL;
	    ret = -1;
	}
    }

    if (ret < 0)
	return -1;

#ifdef PALS_DEBUG
    assert(ret == msg_len);
#endif
    return len;
}
