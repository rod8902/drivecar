#ifndef _pals_env_i_h_
#define _pals_env_i_h_

#include <pals_env.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PALS_PORT_MCAST 0x04

// for a task
struct pals_env_task {
    int id;
    const char *name;
    int prio;
    int rate;
    uint64_t offset;
    uint64_t period;
    in_addr_t addr;
    int port;
};

// for a connection
struct pals_env_con {
    int id;
    const char *name;
    int len;
    int mode;
    int sender;
    int n_peers;
    const struct pals_env_task **peers;
};

// for a pals system
struct pals_env {
    const char *name;
    uint64_t period;
    int mode;
    in_addr_t mcast_addr;
    int mcast_port;
    int mcast_ttl;
    int n_tasks;
    struct pals_env_task *tasks;
    int n_cons;
    struct pals_env_con *cons;
};

extern const struct pals_env_task *pals_env_find_task(const pals_env_t *env, const char *name);
extern const struct pals_env_con *pals_env_find_con(const pals_env_t *env, const char *name);
extern int pals_env_con_is_peer(const struct pals_env_con *env_con, int task_id);

#endif
