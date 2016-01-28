#ifndef _pals_timer_h_
#define _pals_timer_h_

#include <pals_time.h>

struct pals_timer {
    int id;   // timer id
    long round;
    long missed;
};

typedef struct pals_timer pals_timer_t;

extern int pals_timer_init(pals_timer_t *timer);
extern int pals_timer_deinit(pals_timer_t *timer);
extern int pals_timer_start(pals_timer_t *timer, pals_time_t *time, uint64_t period);
extern int pals_timer_wait(pals_timer_t *timer);
extern int pals_timer_stop(pals_timer_t *timer);

//extern long pals_timer_get_missed(pals_timer_t *timer);
//extern long pals_timer_get_round(pals_timer_t *timer);

static inline long pals_timer_get_missed(pals_timer_t *timer)
{
    return timer->missed;
}

static inline long pals_timer_get_round(pals_timer_t *timer)
{
    return timer->round;
}

#endif
