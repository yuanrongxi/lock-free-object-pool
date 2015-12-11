#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "wb_pthread_pool.h"

mutex_pool_t* mutex_pool_create(const char* name, size_t ob_size, size_t array_size,
	mutex_constructor_t constructor, mutex_destructor_t destructor, mutex_check_t check, mutex_reset_t reset)
{
	mutex_pool_t* pool = (mutex_pool_t *)calloc(1, sizeof(mutex_pool_t));
	char* nm = strdup(name);
	void** ptr = (void **)calloc(1, sizeof(void*)* array_size);
	if (ptr == NULL || nm == NULL || pool == NULL){
		free(ptr);
		free(nm);
		free(pool);

		return NULL;
	}

	pool->name = nm;
	pool->ptr = ptr;
	pool->array_size = array_size;
	pool->ob_size = ob_size;

	pool->constructor = constructor;
	pool->destructor = destructor;
	pool->check = check;
	pool->reset = reset;

	pthread_mutex_init(&(pool->mutex), NULL);

	return pool;
}

void mutex_pool_destroy(mutex_pool_t* pool)
{
	pthread_mutex_destroy(&(pool->mutex));

	while (pool->curr > 0){
		void* ptr = pool->ptr[--pool->curr];
		if (pool->destructor)
			pool->destructor(ptr);

		free(ptr);
	}

	free(pool->name);
	free(pool->ptr);
	free(pool);
}

void* mutex_pool_alloc(mutex_pool_t* pool)
{
	void* ret;

	pthread_mutex_lock(&(pool->mutex));

	if (pool->curr > 0){
		ret = pool->ptr[--pool->curr];
		pthread_mutex_unlock(&(pool->mutex));
	}
	else{
		pthread_mutex_unlock(&(pool->mutex));

		ret = malloc(pool->ob_size);
		if (ret != NULL && pool->constructor(ret) != 0){
			free(ret);
			ret = NULL;
		}
	}

	if (ret != NULL)
		pool->reset(ret, 1);

	return ret;
}

void mutex_pool_free(mutex_pool_t* pool, void* ob)
{
	if (ob == NULL && pool->check(ob) != 0){
		return;
	}

	pool->reset(ob, 0);

	pthread_mutex_lock(&(pool->mutex));

	if (pool->curr < pool->array_size){
		pool->ptr[pool->curr++] = ob;
		pthread_mutex_unlock(&(pool->mutex));
	}
	else{
		size_t new_size = 2 * pool->array_size;
		void** new_ptr = (void**)realloc(pool->ptr, sizeof(void *) * new_size);
		if (new_ptr != NULL){
			pool->array_size = new_size;
			pool->ptr = new_ptr;
			pool->ptr[pool->curr++] = ob;

			pthread_mutex_unlock(&(pool->mutex));
		}
		else{
			pthread_mutex_unlock(&(pool->mutex));

			pool->destructor(ob);
			free(ob);
		}
	}
}

void mutex_pool_print(mutex_pool_t* pool)
{
	if (pool == NULL){
		printf("%s pool's ptr = NULL!", pool->name);
		return;
	}
	printf("%s pool:\n", pool->name);
	printf("\t name = %s\n\tob size = %d\n\tarray size = %d\n\tcurr = %d\n",
		pool->name, (int)pool->ob_size, pool->array_size, pool->curr);
}

int32_t mutex_get_pool_info(mutex_pool_t* pool, char* buf)
{
	int32_t pos = 0;
	assert(pool != NULL);

	pthread_mutex_lock(&(pool->mutex));

	pos = sprintf(buf, "%s pool:\n", pool->name);
	pos += sprintf(buf + pos, "\tob size = %d\n\tarray size = %d\n\tcurr = %d\n", (int)pool->ob_size, pool->array_size, pool->curr);

	pthread_mutex_unlock(&(pool->mutex));

	return pos;
}





