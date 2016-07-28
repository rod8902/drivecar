#include <active-standby.h>

// tasks
static struct pals_conf_task tasks[] = {
    {.name = SUPERVISOR, .prio = 0, .ip_addr = "127.0.0.1", .port = 4320, .offset = 0, .rate = 1},
    {.name = SIDE1, .prio = 0, .ip_addr = "127.0.0.1", .port = 4321, .offset = 0, .rate = 1},
    {.name = SIDE2, .prio = 0, .ip_addr = "127.0.0.1", .port = 4322, .offset = 0, .rate = 1}
};

// connections
static struct pals_conf_con cons[] = {
    {.name = CON_CMD, .len = sizeof(int), .mode = 0, .sender = SUPERVISOR, .n_peers = 2, .peers = (const char *[]){SIDE1,SIDE2}},
    {.name = CON_STATE1, .len = sizeof(int), .mode = 0, .sender = SIDE1, .n_peers = 1, .peers = (const char *[]){SIDE2}},
    {.name = CON_STATE2, .len = sizeof(int), .mode = 0, .sender = SIDE2, .n_peers = 1, .peers = (const char *[]){SIDE1}}
};

// master configuration
struct pals_conf pals_conf = {
    .name = "active-standby",
    .period = PERIOD,
    .mcast_addr = "226.1.1.1",
    .mcast_port = 4511,
    .n_tasks = 3,
    .tasks = tasks,
    .n_cons = 3,
    .cons = cons
};
