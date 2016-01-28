#include <pals_timer.h>
#ifdef __linux__
#include <sys/timerfd.h>
#else
#error Platform not supported
#endif

#include <stdint.h>
#include <unistd.h>
#ifdef PALS_DEBUG
#include <stdio.h>
#endif

int pals_timer_init(struct pals_timer *timer)
{
    int fd;

    // create the timer
    fd = timerfd_create(CLOCK_REALTIME, 0);
    timer->id = fd;

    return fd;
}

int pals_timer_start(struct pals_timer *timer, pals_time_t *start_time, uint64_t period_ns)
{
    int ret;
    uint64_t ns;
    uint64_t sec;
    struct itimerspec ival;

    // reset the timer
    timer->round = 0;
    timer->missed = 0;

    // make the timer periodic
    ival.it_value.tv_sec = start_time->sec;
    ival.it_value.tv_nsec = start_time->nsec;

    sec = period_ns / 1000000000L;
    ns = period_ns % 1000000000L;
    ival.it_interval.tv_sec = sec;
    ival.it_interval.tv_nsec = ns;

    ret = timerfd_settime(timer->id, TFD_TIMER_ABSTIME, &ival, NULL);

    return ret;
}

int pals_timer_stop(struct pals_timer *timer)
{
    int ret;
    struct itimerspec ival;

    // clear times
    ival.it_value.tv_sec = 0;
    ival.it_value.tv_nsec = 0;
    ival.it_interval.tv_sec = 0;
    ival.it_interval.tv_nsec = 0;

    ret = timerfd_settime(timer->id, TFD_TIMER_ABSTIME, &ival, NULL);

    return ret;
}

/*
 * pals_timer_wait
 * Returns: number of overruns on success. On failure -1 is returned.
 */
int pals_timer_wait(struct pals_timer *timer)
{
    uint64_t expired;
    int ret;

    // wait for the next timer event
    // If we have expired any the number is written to "expired"
    ret = read(timer->id, &expired, sizeof(expired));
    if (ret == -1) {
#ifdef PALS_DEBUG
	perror("read timerfd");
#endif
	return -1;
    }

    timer->round += expired;
    timer->missed += (expired - 1);

    return (int)expired - 1;
}
