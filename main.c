#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#include "wb_gcc.h"
#include "wb_pthread_pool.h"
#include "wb_object_pool.h"
#include "wb_cas_pool.h"

static char pad1[CACHE_LINE_ALIGNMENT];
static cell_pool_t* obj_pool = NULL;
static char pad2[CACHE_LINE_ALIGNMENT];
static cas_pool_t* cas_pool = NULL;
static char pad3[CACHE_LINE_ALIGNMENT];
static mutex_pool_t* mutex_pool = NULL;
static char pad4[CACHE_LINE_ALIGNMENT];


#define TMAGIC			0x0395ef09

typedef struct{
	int			index;
	uint32_t	magic;
}tobject_t;

int32_t	obj_init(void* ptr)
{
	return 0;
}

void obj_destroy(void* ptr)
{
}

void obj_reset(void* ptr, int32_t flag)
{
	tobject_t* obj = (tobject_t *)ptr;
	obj->magic = (flag == 1 ? TMAGIC : 0);
}

int32_t	obj_check(void* ptr)
{
	tobject_t* obj = (tobject_t *)ptr;
	if (obj->magic == TMAGIC)
		return 0;

	assert(0);
	return -1;
}

#define THREAD_NUM  4
#define POWER		3
#define POOL_SIZE	(1 << POWER)

static int32_t alloc_count = 0;

static void* thread_mutex_func(void* arg)
{
	tobject_t* arr[POOL_SIZE / 2] = { 0 };
	int i = 0;
	int l = 0;
	while (alloc_count < 1000000){
		arr[i] = (tobject_t *)mutex_pool_alloc(mutex_pool);
		arr[i]->index = arr[i]->index + 1;
		i++;
		if (i >= POOL_SIZE / 2){
			for (l = 0; l < i; l++){
				mutex_pool_free(mutex_pool, arr[l]);
			}
			i = 0;
		}

		WT_ATOMIC_ADD4(alloc_count, 1);
	}

	if (i >= 0)
		for (l = 0; l < i; l++)
			mutex_pool_free(mutex_pool, arr[l]);

	return NULL;
}

static void* thread_spin_func(void* arg)
{
	tobject_t* arr[POOL_SIZE / 2] = {0};
	int i = 0;
	int l = 0;
	while (alloc_count < 1000000){
		arr[i] = (tobject_t *)pool_alloc(obj_pool);
		arr[i]->index = arr[i]->index + 1;
		i++;
		if (i >= POOL_SIZE / 2){
			for (l = 0; l < i; l++){
				pool_free(obj_pool, arr[l]);
			}
			i = 0;
		}

		WT_ATOMIC_ADD4(alloc_count, 1);
	}

	if (i > 0)
		for (l = 0; l < i; l++){
			pool_free(obj_pool, arr[l]);
		}

	return NULL;
}

static  void* thread_cas_func(void* arg)
{
	tobject_t* obj = NULL;
	int i = 0;
	int l = 0;
	while (alloc_count < 1000000){
		obj = (tobject_t *)cas_pool_alloc(cas_pool);
		obj->index++;

		cas_pool_free(cas_pool, obj);

		WT_ATOMIC_ADD4(alloc_count, 1);
	}


	return NULL;
}

void test_mutex_pool()
{
	pthread_t threads[THREAD_NUM];
	tobject_t* o = NULL;
	tobject_t* ar[POOL_SIZE];
	int i = 0;
	struct timeval b, e;

	mutex_pool = mutex_pool_create("pthread_mutex", sizeof(tobject_t), POOL_SIZE, obj_init, obj_destroy, obj_check, obj_reset);

	for (i = 0; i < POOL_SIZE; i++){
		o = (tobject_t *)mutex_pool_alloc(mutex_pool);
		printf("alloc obj = 0x%lx\n", (uint64_t)o);
		ar[i] = o;
	}

	for (i = 0; i < POOL_SIZE; i++){
		mutex_pool_free(mutex_pool, ar[i]);
	}

	o = (tobject_t *)mutex_pool_alloc(mutex_pool);
	printf("only o = 0x%lx\n", (uint64_t)o);
	mutex_pool_free(mutex_pool, o);
	
	gettimeofday(&b, NULL);
	for (i = 0; i < THREAD_NUM; i++)
		pthread_create(&threads[i], NULL, thread_mutex_func, NULL);

	for (i = 0; i < THREAD_NUM; i++)
		pthread_join(threads[i], NULL);

	gettimeofday(&e, NULL);
	printf("mutex alloc_count = %d, delay = %lu ms\n", alloc_count, ((e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec)) / 1000);
	
	mutex_pool_print(mutex_pool);

	mutex_pool_destroy(mutex_pool);
}

void test_spin_pool()
{
	pthread_t threads[THREAD_NUM];
	tobject_t* o = NULL;
	tobject_t* ar[POOL_SIZE];
	int i = 0;
	struct timeval b, e;

	obj_pool = pool_create("spin", sizeof(tobject_t), POOL_SIZE, obj_init, obj_destroy, obj_check, obj_reset);

	for (i = 0; i < POOL_SIZE; i++){
		o = (tobject_t *)pool_alloc(obj_pool);
		printf("alloc obj = 0x%lx\n", (uint64_t)o);
		ar[i] = o;
	}

	for (i = 0; i < POOL_SIZE; i++){
		pool_free(obj_pool, ar[i]);
	}

	o = (tobject_t *)pool_alloc(obj_pool);
	printf("only o = 0x%lx\n", (uint64_t)o);
	pool_free(obj_pool, o);

	gettimeofday(&b, NULL);
	for (i = 0; i < THREAD_NUM; i++)
		pthread_create(&threads[i], NULL, thread_spin_func, NULL);

	for (i = 0; i < THREAD_NUM; i++)
		pthread_join(threads[i], NULL);

	gettimeofday(&e, NULL);
	printf("spin alloc_count = %d, delay = %lu ms\n", alloc_count, ((e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec)) / 1000);

	pool_print(obj_pool);

	pool_destroy(obj_pool);
}

void test_cas_pool()
{
	pthread_t threads[THREAD_NUM];
	tobject_t* o = NULL;
	tobject_t* ar[POOL_SIZE];
	int i = 0;
	struct timeval b, e;

	cas_pool = cas_pool_create("cas", sizeof(tobject_t), obj_init, obj_destroy, obj_check, obj_reset);

	for (i = 0; i < POOL_SIZE; i++){
		o = (tobject_t *)cas_pool_alloc(cas_pool);
		printf("alloc obj = 0x%lx\n", (uint64_t)o);
		ar[i] = o;
	}

	for (i = 0; i < POOL_SIZE; i++){
		cas_pool_free(cas_pool, ar[i]);
	}

	o = (tobject_t *)cas_pool_alloc(cas_pool);
	printf("only o = 0x%lx\n", (uint64_t)o);

	cas_pool_print(cas_pool);
	cas_pool_free(cas_pool, o);

	gettimeofday(&b, NULL);
	for (i = 0; i < THREAD_NUM; i++)
	pthread_create(&threads[i], NULL, thread_cas_func, NULL);

	for (i = 0; i < THREAD_NUM; i++)
	pthread_join(threads[i], NULL);

	gettimeofday(&e, NULL);
	printf("cas alloc_count = %d, delay = %lu ms\n", alloc_count, ((e.tv_sec - b.tv_sec) * 1000000 + (e.tv_usec - b.tv_usec)) / 1000);
	
	cas_pool_print(cas_pool);

	cas_pool_destroy(cas_pool);
}

int main(int argc, const char* argv[])
{
	test_mutex_pool();

	alloc_count = 0;
	test_spin_pool();

	alloc_count = 0;
	
	test_cas_pool();

	return 0;
}



