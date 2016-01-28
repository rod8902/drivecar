#include <pals_time.h>
#include <time.h>
#include <errno.h>
#ifdef PALS_DEBUG
#include <stdio.h>
#include <assert.h>
#endif

int pals_get_time(pals_time_t *t)
{
    int res;
    struct timespec tv;

    res = clock_gettime(CLOCK_REALTIME, &tv);

    if (res != 0) {
#ifdef PALS_DEBUG
	perror("clock_gettime");
#endif
	return -1;
    }

    t->sec = tv.tv_sec;
    t->nsec = tv.tv_nsec;
    return 0;
}

void pals_cal_base_time(pals_time_t *out, const pals_time_t *in, uint64_t period)
{
    uint64_t rem;

    if (in == NULL) {
	pals_get_time(out);
	in = out;
    }

    rem = (((in->sec % period) * 1000000000L) + in->nsec) % period;

    pals_time_sub_ns(out, in, rem);
}
