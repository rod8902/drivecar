#ifndef _pals_task_i_h_
#define _pals_task_i_h_

#include <pals_port_i.h>
#include <pals_lock.h>
#include <pals_time.h>
#include <pals_timer.h>
#include <pals_env_i.h>
#include <pals_task.h>
#include <unistd.h>
#ifdef _POSIX_THREADS
#include <pthread.h>
#else
#error Platform not supported
#endif
#ifdef __linux__
#include <sys/timerfd.h>
#else
#error Platform not supported
#endif
#include <arpa/inet.h>

#define PALS_MAX_PHASES	8

#define PALS_TASK_MULTIRATE 0x01
#define PALS_TIME_ANALYSIS	0x10

struct pals_phase {
    int phase;
    uint64_t offset; // start offset time (in nonoseconds)
    pals_tasklet_t *tasklet; // routine to be executed
    void *arg;	// arg
    long missed;
};

struct pals_task {
    const pals_env_t *env;
    int id; // global task id (auto generated)
    const char *name;	// human readable task name (global unique)
    int prio;  // priority
    int rate;	// for multirate
    uint64_t period; // task period, in nanosecond
    uint64_t offset; // start offset time (in nonoseconds)
    uint64_t sched_delay_limit;	// allowed schedule delay limit
    int flags;
    int status;
    in_addr_t addr; // inet address
    int port;	// UDP port number
    int num_phases;  // number of phases
    struct pals_phase phases[PALS_MAX_PHASES];

    int phase;	// current phase
    pals_time_t pals_base_time;  // PALS base time
    pals_time_t base_time;  // task base time
    pals_time_t start_time; // start_time of current tasklet
    pals_lock_t lock;

    uint64_t sched_delay;   // schedule delay of the current running tasklet (in nanoseconds)
    /*
    uint64_t sched_count;
    uint64_t sched_missed;
    uint64_t sched_delay_avg;
    */

    // POSIX
    pthread_t thread;
    pthread_cond_t start_signal;
    pals_timer_t timer;   // master periodic timer(interval is the pals period)
    pals_timer_t timer_phase;
    pals_timer_t timer_multirate;
    int tx_sock; // send socket
    int rx_sock;	// recv socket
    int mcast_tx_sock;
    int mcast_rx_sock;
    pals_rx_port_t **rx_ports;	// rx ports
    pals_msg_t *rx_msg;	// recv buffer
    size_t rx_msg_len;
};

/*
 * task status
 */
enum {
    PALS_INACTIVE,
    PALS_STARTED,
    PALS_WAITING,
    PALS_RUNNING,
    PALS_FAILED,
    PALS_ABORTED
};

static inline int pals_task_is_active(pals_task_t *task)
{
    return (task->status != PALS_INACTIVE);
}

#endif
