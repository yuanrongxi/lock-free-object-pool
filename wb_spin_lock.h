#ifndef __SU_SPIN_LOCK_H_
#define __SU_SPIN_LOCK_H_

#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

#include "wb_gcc.h"

typedef int				spin_lock_t;

static inline void lock_init(spin_lock_t* l);
static inline void lock_lock(spin_lock_t* l);
static inline void lock_clock(spin_lock_t* l, int count);
static inline int lock_try_lock(spin_lock_t* l);
static inline void lock_unlock(spin_lock_t* l);
static inline void lock_destroy(spin_lock_t* l);

typedef union
{
	uint64_t u;
	uint32_t us;				
	struct{
		uint16_t writers;
		uint16_t readers;
		uint16_t users;
		uint16_t pad;
	} s;
}rwlock_t;

static inline int rwlock_init(rwlock_t* rwlock);
static inline int rwlock_destroy(rwlock_t* rwlock);
static inline int rwlock_try_rlock(rwlock_t* rwlock);
static inline int rwlock_rlock(rwlock_t* rwlock);
static inline int rwlock_runlock(rwlock_t* rwlock);
static inline int rwlock_try_wlock(rwlock_t* rwlock);
static inline int rwlock_wlock(rwlock_t* rwlock);
static inline int rwlock_wunlock(rwlock_t* rwlock);

#include "wb_spin_lock.inl"

#endif







