#ifndef _pals_time_h_
#define _pals_time_h_

#include <stdint.h>

struct pals_time {
    int64_t sec;	// seconds since EPOCH
    int64_t nsec;	// in nanoseconds
};

typedef struct pals_time pals_time_t;

extern int pals_get_time(pals_time_t *t);

static inline void pals_time_copy(pals_time_t *dst, const pals_time_t *src)
{
    dst->sec = src->sec;
    dst->nsec = src->nsec;
}

static inline void pals_time_set(pals_time_t *a, int64_t sec, int64_t nsec)
{
    a->sec = sec;
    a->nsec = nsec;
}

static inline int pals_time_equal(const pals_time_t *a, const pals_time_t *b)
{
    return (a->sec == b->sec && a->nsec == b->nsec);
}

static inline int pals_time_before(const pals_time_t *a, const pals_time_t *b)
{
    return (a->sec < b->sec || (a->sec == b->sec && a->nsec < b->nsec));
}

/**
 * @r = @a + @ns
 */
static inline void pals_time_add_ns(pals_time_t *r, const pals_time_t *a, int64_t ns)
{
    r->sec = a->sec + ns/1000000000L;
    r->nsec = a->nsec + ns%1000000000L;
    if (r->nsec >= 1000000000L) {
	r->sec++;
	r->nsec -= 1000000000L;
    }
}

/**
 * @r = @a - @ns
 */
static inline void pals_time_sub_ns(pals_time_t *r, const pals_time_t *a, int64_t ns)
{
    r->sec = a->sec - ns/1000000000L;
    r->nsec = a->nsec - ns%1000000000L;
    if (r->nsec < 0) {
	r->sec--;
	r->nsec += 1000000000L;
    }
}

static inline void pals_time_sub(pals_time_t *r, const pals_time_t *a, const pals_time_t *b)
{
    r->sec = a->sec - b->sec;
    r->nsec = a->nsec - b->nsec;
    if (r->nsec < 0) {
	r->sec--;
	r->nsec += 1000000000L;
    }
}

static inline int64_t pals_time_diff_ns(const pals_time_t *a, const pals_time_t *b)
{
    return (a->sec - b->sec)*1000000000L + (a->nsec - b->nsec);
}

#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define hton64(x)  bswap_64(x)
# define ntoh64(x)  bswap_64(x)
#else
# define hton64(x)  (x)
# define ntoh64(x)  (x)
#endif

static inline void pals_time_hton(pals_time_t *dst, pals_time_t *src)
{
    dst->sec = hton64(src->sec);
    dst->nsec = hton64(src->nsec);
}

static inline void pals_time_ntoh(pals_time_t *dst, pals_time_t *src)
{
    dst->sec = ntoh64(src->sec);
    dst->nsec = ntoh64(src->nsec);
}

extern void pals_cal_base_time(pals_time_t *out, const pals_time_t *in, uint64_t period);

#endif
