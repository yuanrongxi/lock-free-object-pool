static inline void lock_init(spin_lock_t* l)
{
	*l = 0;
}

static inline void lock_lock(spin_lock_t* l)
{
	int i;
	while (__sync_lock_test_and_set(l, 1)){
		for (i = 0; i < 512; i++){
			WT_PAUSE();
		}

		if (*l == 1) sched_yield();
	}
};

static inline void lock_clock(spin_lock_t* l, int count)
{
	int i;
	while (__sync_lock_test_and_set(l, 1)){
		for (i = 0; i < count; i++)
			WT_PAUSE();
	}
}

static inline int lock_try_lock(spin_lock_t* l)
{
	if (__sync_lock_test_and_set(l, 1) != 0)
		return 1;

	return 0;
}

static inline void lock_unlock(spin_lock_t* l)
{
	__sync_lock_release(l);
}

static inline void lock_destroy(spin_lock_t* l)
{
}

int rwlock_init(rwlock_t* rwlock)
{
	rwlock->u = 0;

	return 0;
}

int rwlock_destroy(rwlock_t* rwlock)
{
	rwlock->u = 0;

	return 0;
}

int rwlock_try_rlock(rwlock_t* rwlock)
{
	uint64_t old, ne, pad, users, writers;

	pad = rwlock->s.pad;
	users = rwlock->s.users;
	writers = rwlock->s.writers;

	old = (pad << 48) + (users << 32) + (users << 16) + writers;
	ne = (pad << 48) + ((users + 1) << 32) + ((users + 1) << 16) + writers;

	return (WT_ATOMIC_CAS_VAL8(rwlock->u, old, ne) == old ? 0 : -1);
}

int rwlock_rlock(rwlock_t* rwlock)
{
	uint64_t me;
	uint16_t val;
	int pause_cnt;

	me = WT_ATOMIC_FETCH_ADD8(rwlock->u, (uint64_t)1 << 32);
	val = (uint16_t)(me >> 32);

	for (pause_cnt = 0; val != rwlock->s.readers;){
		if (++pause_cnt < 1000)
			WT_PAUSE();
		/*else
			su_sleep(0, 10);*/
	}

	++rwlock->s.readers;

	return 0;
}

int rwlock_runlock(rwlock_t* rwlock)
{
	WT_ATOMIC_ADD2(rwlock->s.writers, 1);
}

int rwlock_try_wlock(rwlock_t* rwlock)
{
	uint64_t old, ne, pad, readers, users;

	pad = rwlock->s.pad;
	readers = rwlock->s.readers;
	users = rwlock->s.users;

	old = (pad << 48) + (users << 32) + (readers << 16) + users;
	ne = (pad << 48) + ((users + 1) << 32) + (readers << 16) + users;

	return (WT_ATOMIC_CAS_VAL8(rwlock->u, old, ne) == old ? 0 : -1);
}

int rwlock_wlock(rwlock_t* rwlock)
{
	uint64_t me;
	uint16_t val;

	me = WT_ATOMIC_FETCH_ADD8(rwlock->u, (uint64_t)1 << 32);
	val = (uint16_t)(me >> 32); 
	while (val != rwlock->s.writers)
		WT_PAUSE();

	return 0;
}

int rwlock_wunlock(rwlock_t* rwlock)
{
	rwlock_t copy;

	copy = *rwlock;
	WT_BARRIER();

	++copy.s.writers;
	++copy.s.readers;

	rwlock->us = copy.us;

	return 0;
}
