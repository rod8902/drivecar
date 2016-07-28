#include "pals_lock.h"

int pals_lock_init(pals_lock_t *lock)
{
    return pthread_mutex_init(lock, NULL);
}

int pals_lock_deinit(pals_lock_t *lock)
{
    return pthread_mutex_destroy(lock);
}

int pals_lock(pals_lock_t *lock)
{
    return pthread_mutex_lock(lock);
}

int pals_trylock(pals_lock_t *lock)
{
    return pthread_mutex_trylock(lock);
}

int pals_unlock(pals_lock_t *lock)
{
    return pthread_mutex_unlock(lock);
}
