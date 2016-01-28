#ifndef _pals_lock_h_
#define _pals_lock_h_

#include <unistd.h>

#ifdef _POSIX_THREADS
#include <pthread.h>
typedef pthread_mutex_t pals_lock_t;
#else
#error Platform not supported
#endif
extern int pals_lock_init(pals_lock_t *lock);
extern int pals_lock_deinit(pals_lock_t *lock);
extern int pals_lock(pals_lock_t *lock);
extern int pals_trylock(pals_lock_t *lock);
extern int pals_unlock(pals_lock_t *lock);

#endif
