
#define	WT_STATIC_ASSERT(cond)	(void)sizeof(char[1 - 2 * !(cond)])

#define WT_SIZET_FMT		"zu"


#define WT_COMPILER_TYPE_ALIGN(x)			__attribute__((align(x)))

#define WT_PACKED_STRUCT_BEGIN(name)		struct __attribute__ ((__packed__)) name {
#define WT_PACKED_STRUCT_END				}

#define WT_GCC_FUNC_ATTRIBUTE(x)
#define WT_GCC_FUNC_DECL_ATTRIBUTE(x) __attribute__(x)

#define __WT_ATOMIC_ADD(v, val, n)			(WT_STATIC_ASSERT(sizeof(v) == (n)), __sync_add_and_fetch(&(v), val))
#define __WT_ATOMIC_FETCH_ADD(v, val, n)	(WT_STATIC_ASSERT(sizeof(v) == (n)), __sync_fetch_and_add(&(v), val))
#define __WT_ATOMIC_CAS(v, old, ne, n)		(WT_STATIC_ASSERT(sizeof(v) == (n)), __sync_val_compare_and_swap(&(v), old, ne) == (old))
#define	__WT_ATOMIC_CAS_VAL(v, old, ne, n)	(WT_STATIC_ASSERT(sizeof(v) == (n)), __sync_val_compare_and_swap(&(v), old, ne))
#define __WT_ATOMIC_STORE(v, val, n)		(WT_STATIC_ASSERT(sizeof(v) == (n)), __sync_lock_test_and_set(&(v), val))

#define __WT_ATOMIC_SUB(v, val, n)			(WT_STATIC_ASSERT(sizeof(v) == (n)), __sync_sub_and_fetch(&(v), val))

/*int8_t*/
#define	WT_ATOMIC_ADD1(v, val)				__WT_ATOMIC_ADD(v, val, 1)
#define	WT_ATOMIC_FETCH_ADD1(v, val)		__WT_ATOMIC_FETCH_ADD(v, val, 1)
#define	WT_ATOMIC_CAS1(v, old, ne)			__WT_ATOMIC_CAS(v, old, ne, 1)
#define	WT_ATOMIC_CAS_VAL1(v, old, ne)		__WT_ATOMIC_CAS_VAL(v, old, ne, 1)
#define	WT_ATOMIC_STORE1(v, val)			__WT_ATOMIC_STORE(v, val, 1)
#define	WT_ATOMIC_SUB1(v, val)				__WT_ATOMIC_SUB(v, val, 1)

/*int16_t*/
#define	WT_ATOMIC_ADD2(v, val)				__WT_ATOMIC_ADD(v, val, 2)
#define	WT_ATOMIC_FETCH_ADD2(v, val)		__WT_ATOMIC_FETCH_ADD(v, val, 2)
#define	WT_ATOMIC_CAS2(v, old, ne)			__WT_ATOMIC_CAS(v, old, ne, 2)
#define	WT_ATOMIC_CAS_VAL2(v, old, ne)		__WT_ATOMIC_CAS_VAL(v, old, ne, 2)
#define	WT_ATOMIC_STORE2(v, val)			__WT_ATOMIC_STORE(v, val, 2)
#define	WT_ATOMIC_SUB2(v, val)				__WT_ATOMIC_SUB(v, val, 2)

/*int32_t*/
#define	WT_ATOMIC_ADD4(v, val)				__WT_ATOMIC_ADD(v, val, 4)
#define	WT_ATOMIC_FETCH_ADD4(v, val)		__WT_ATOMIC_FETCH_ADD(v, val, 4)
#define	WT_ATOMIC_CAS4(v, old, ne)			__WT_ATOMIC_CAS(v, old, ne, 4)
#define	WT_ATOMIC_CAS_VAL4(v, old, ne)		__WT_ATOMIC_CAS_VAL(v, old, ne, 4)
#define	WT_ATOMIC_STORE4(v, val)			__WT_ATOMIC_STORE(v, val, 4)
#define	WT_ATOMIC_SUB4(v, val)				__WT_ATOMIC_SUB(v, val, 4)

/*int64_t*/
#define	WT_ATOMIC_ADD8(v, val)				__WT_ATOMIC_ADD(v, val, 8)
#define	WT_ATOMIC_FETCH_ADD8(v, val)		__WT_ATOMIC_FETCH_ADD(v, val, 8)
#define	WT_ATOMIC_CAS8(v, old, ne)			__WT_ATOMIC_CAS(v, old, ne, 8)
#define	WT_ATOMIC_CAS_VAL8(v, old, ne)		__WT_ATOMIC_CAS_VAL(v, old, ne, 8)
#define	WT_ATOMIC_STORE8(v, val)			__WT_ATOMIC_STORE(v, val, 8)
#define	WT_ATOMIC_SUB8(v, val)				__WT_ATOMIC_SUB(v, val, 8)

#ifdef SU_SERVER
#define WT_BARRIER()						__asm__ volatile("" ::: "memory")
#define WT_PAUSE()							__asm__ volatile("pause\n" ::: "memory")

#if defined(x86_64) || defined(__x86_64__) 
#define WT_FULL_BARRIER() do{ __asm__ volatile("mfence" ::: "memory");}while(0)
#define WT_READ_BARRIER() do{ __asm__ volatile("lfence" ::: "memory");}while(0)
#define WT_WRITE_BARRIER() do{ __asm__ volatile("sfence" ::: "memory");}while(0)
#elif defined(i386) || defined(__i386__) 
#define WT_FULL_BARRIER() do{__asm__ volatile ("lock; addl $0, 0(%%esp)" ::: "memory");}while(0)
#define WT_READ_BARRIER() WT_FULL_BARRIER()
#define WT_WRITE_BARRIER() WT_FULL_BARRIER()
#else 
#error "No write barrier implementation for this hardware"
#endif
#else
#define WT_BARRIER()
#define WT_PAUSE()
#define WT_FULL_BARRIER()
#define WT_READ_BARRIER()
#define WT_WRITE_BARRIER()
#endif

#define	CACHE_LINE_ALIGNMENT	64	

#define CACHE_ALIGN			__attribute__((aligned(CACHE_LINE_ALIGNMENT)))

/**********************************************************************************************/




