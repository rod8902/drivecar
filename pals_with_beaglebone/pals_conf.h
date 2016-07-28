#ifndef _pals_conf_h_
#define _pals_conf_h_

#include <stdint.h>

/*
 * configuration for a task
 */
struct pals_conf_task {
    const char *name;
    int prio;
    const char *ip_addr; // ip-address
    int port;	// UDP port number
    uint64_t offset;    // dispatch offset (in nanoseconds)
    int rate;	// freq. rate with respect to the pals frequency
    //uint64_t period;    // multi-rate task if non zero
};

/*
 * configuration for a connection
 */
struct pals_conf_con {
    const char *name;
    int len;	// length of message
    int mode;	// operation mode
    const char *sender;
    int n_peers;    // number of receivers(tasks)
    const char **peers;	// list of receivers(names of tasks)
};

/*
 * configuration for a pals system
 */
struct pals_conf {
    const char *name;
    uint64_t period;    // pals period (in nanoseconds)
    const char *mcast_addr; // ip-addr for UDP multicasting
    int mcast_port; // port number for UDP multicasting
    int mcast_ttl;  // TTL value
    int n_tasks;
    const struct pals_conf_task *tasks;
    int n_cons;
    const struct pals_conf_con *cons;
};

#endif
