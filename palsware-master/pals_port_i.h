#ifndef _pals_port_i_h_
#define _pals_port_i_h_

#include <pals_port.h>
#include <pals_task.h>
#include <dl_list.h>
#include <pals_env_i.h>
#include <stdint.h>

#define PALS_PORT_MCAST 0x04

struct pals_msg;
typedef struct pals_msg pals_msg_t;

struct pals_tx_port {
    pals_task_t *task;	// task belong to
    const struct pals_env_con *con;
    pals_msg_t *msg;	// message buffer for sending
};

struct pals_rx_port {
    pals_task_t *task;	// task belong to
    const struct pals_env_con *con;
    uint64_t period;	// message sync period
    int s_rate;	// rate of sender
    int r_rate;	// rate of receiver
    struct dl_list msg_list;    // queue for receiving messages
};

struct pals_msg {
    struct dl_list list;
    uint16_t port_id;   // port(connection) id
    uint16_t mode;
    uint16_t sender; // sender task-id
    uint16_t len;    // length of data
    pals_time_t base_time; // base-time at sending
    pals_time_t start_time; // start-time of sender tasklet
    char data[];    // message data
};

#endif
