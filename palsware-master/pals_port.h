#ifndef _pals_port_h_
#define _pals_port_h_

#include <pals_task.h>
#include <string.h>

#define PALS_NEXT_ROUND 0
#define PALS_SAME_ROUND 0x02

struct pals_rx_port;
struct pals_tx_port;
typedef struct pals_rx_port pals_rx_port_t;
typedef struct pals_tx_port pals_tx_port_t;

extern pals_tx_port_t *pals_tx_port_open(pals_task_t *task, const char *con_name);
extern pals_rx_port_t *pals_rx_port_open(pals_task_t *task, const char *con_name);

extern int pals_send(pals_tx_port_t *port, const void *buf, size_t len);
extern int pals_recv(pals_rx_port_t *port, void *buf, size_t buf_len);

#endif
