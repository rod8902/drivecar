#include <pals_port_i.h>
#include <pals_task_i.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#ifdef PALS_DEBUG
#include <stdio.h>
#include <assert.h>
#endif

extern int sock_recvfrom(int sock, void *buf, int buflen, struct sockaddr *saddr);
static pals_msg_t *_pals_recv_msg(struct pals_task *task, int sock);

static inline pals_msg_t *_add_msg_list(pals_rx_port_t *port, const struct pals_msg *msg)
{
    // add the message to the queue
    pals_msg_t *new = (pals_msg_t *)malloc(sizeof(pals_msg_t) + msg->len);
    if (new == NULL) {
#ifdef PALS_DEBUG
	perror("malloc");
#endif
    } else {
	memcpy(new, msg, sizeof(pals_msg_t) + msg->len);
	dl_list_add_tail(&port->msg_list, &new->list);
    }
    return new;
}

enum {
    PALS_MSG_MATCH = 0,
    PALS_MSG_OLD,
    PALS_MSG_BAD,
    PALS_MSG_OTHER
};

/*
 * _msg_test_and_dispatch
 */
static int _msg_test_and_dispatch(pals_rx_port_t *port, pals_msg_t *msg)
{
    pals_time_t base_time;
    const struct pals_task *task = port->task;
    uint64_t msg_period;
    pals_rx_port_t *p;

    p = task->rx_ports[msg->port_id];
    if (p == NULL)
	return PALS_MSG_BAD;

    msg_period = p->period;

    // calculate message base time
    pals_cal_base_time(&msg->base_time, &msg->base_time, msg_period);
    if ((p->con->mode & PALS_SAME_ROUND) == 0) {
	// for next round
	pals_time_add_ns(&msg->base_time, &msg->base_time, msg_period);
    }

    pals_cal_base_time(&base_time, &task->base_time, msg_period);
#if TEST
    printf("test_and_dispatch: port(id=%d,period=%llu,s_rate=%d,r_rate=%d) msg_base_time=(%llu,%llu), base_time=(%llu,%llu)\n",
	    p->con->id, p->period, p->s_rate, p->r_rate, msg->base_time.sec, msg->base_time.nsec, base_time.sec, base_time.nsec);
#endif

    if (p == port && pals_time_equal(&msg->base_time, &base_time)) {
#ifdef PALS_DEBUG
	assert(msg->port_id == port->con->id);
#endif
	return PALS_MSG_MATCH;
    } else if (pals_time_before(&msg->base_time, &base_time)) {
	return PALS_MSG_OLD;
    } else {
	_add_msg_list(p, msg);
	return PALS_MSG_OTHER;
    }
}

static inline int copy_msg_to_buffer(const pals_msg_t *msg, void *buf, size_t buflen)
{
    size_t len;

    len = msg->len;
    if (buflen < len) {
	errno = EMSGSIZE;
	return -1;
    }

    memcpy(buf, msg->data, len);
    return len;
}

/*
 * pals_recv
 * Returns: return received message size. -1 if error
 * errno: EMSGSIZE, ENOMSG
 */
int pals_recv(pals_rx_port_t *port, void *buf, size_t buflen)
{
    int ret;
    pals_msg_t *msg, *n;
    pals_msg_t *last_match = NULL;
    int sock;
    struct pals_task *task = port->task;
    struct pals_time base_time;	// expected message base time
    const int s_rate = port->s_rate;
    const int r_rate = port->r_rate;

    // receiver task's base time
    pals_cal_base_time(&base_time, &task->base_time, port->period);

    dl_list_for_each_safe(msg, n, &port->msg_list, pals_msg_t, list) {
	if (pals_time_before(&msg->base_time, &base_time)) {
	    // remove old messages
	    dl_list_del(&msg->list);
	    free(msg);
	} else if (pals_time_equal(&msg->base_time, &base_time)) {
	    if (s_rate > 1) {
		if (last_match != NULL) {
		    // remove previous matching message
		    dl_list_del(&last_match->list);
		    free(last_match);
		}
		last_match = msg;
	    } else {
		ret = copy_msg_to_buffer(msg, buf, buflen);
		if (r_rate == 1) {
		    dl_list_del(&msg->list);
		    free(msg);
		}
		return ret;
	    }
	}
    }

    if (port->con->mode & PALS_PORT_MCAST) {
	sock = task->mcast_rx_sock;
    } else {
	sock = task->rx_sock;
    }

    if (sock > 0) while ((msg = _pals_recv_msg(task, sock)) != NULL) {
	if (_msg_test_and_dispatch(port, msg) == PALS_MSG_MATCH) {
	    if (s_rate == 1) {
		if (r_rate > 1) {
		    _add_msg_list(port, msg);
		}
		return copy_msg_to_buffer(msg, buf, buflen);
	    } else {
		// s_rate > 1
		if (last_match != NULL) {
		    // remove previous matching message
		    dl_list_del(&last_match->list);
		    free(last_match);
		}
		last_match = _add_msg_list(port, msg);
	    }
	}
    }

    if (last_match != NULL) {
#ifdef PALS_DEBUG
	assert(s_rate > 1);
#endif
	ret = copy_msg_to_buffer(last_match, buf, buflen);
	if (r_rate == 1) {
	    dl_list_del(&last_match->list);
	    free(last_match);
	}
	return ret;
    }

    errno = ENOMSG;
    return -1;
}

static pals_msg_t *_pals_recv_msg(struct pals_task *task, int sock)
{
    pals_msg_t *msg;
    size_t buflen;
    struct sockaddr saddr;
    int len;

    msg = task->rx_msg;
    buflen = task->rx_msg_len + sizeof(pals_msg_t);

    len = sock_recvfrom(sock, msg, buflen, &saddr);
    if (len < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
	// no messages
	return NULL;
    } else if (len == 0) {
	// target shutdown
	return NULL;
    } else if (len < 0) {
	// other error
	return NULL;
    } else if (len < sizeof(pals_msg_t)) {
	// too short. something wrong
	return NULL;
    }

    // convert to host values
    msg->mode = ntohs(msg->mode);
    msg->sender = ntohs(msg->sender);
    msg->port_id = ntohs(msg->port_id);
    msg->len = ntohs(msg->len);
    pals_time_ntoh(&msg->base_time, &msg->base_time);
    pals_time_ntoh(&msg->start_time, &msg->start_time);

#ifdef PALS_DEBUG
    assert((len - sizeof(pals_msg_t)) == msg->len);
#endif

    return msg;
}
