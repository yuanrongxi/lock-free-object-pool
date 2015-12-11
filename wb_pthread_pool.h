#ifndef __WB_PTHREAD_POOL_H_
#define __WB_PTHREAD_POOL_H_

#include <stdint.h>
#include <pthread.h>
#include "wb_gcc.h"

/*对象初始化函数指针*/
typedef int32_t(*mutex_constructor_t)(void* ob);
/*对象撤销函数指针*/
typedef void(*mutex_destructor_t)(void* ob);
/*合法性校验函数指针*/
typedef int32_t(*mutex_check_t)(void* ob);
/*对象复位函数指针*/
typedef void(*mutex_reset_t)(void* ob, int32_t flag);

typedef struct CACHE_ALIGN mutex_pool_s
{
	void**				ptr;		/*object array*/
	size_t				ob_size;	/*对象占用的空间字节数*/

	pthread_mutex_t		mutex;		/*latch*/
	int32_t				array_size;	/*ptr数组的长度*/
	int32_t				curr;		/*当前空闲的ptr位置*/

	mutex_constructor_t	constructor; /*对象初始化函数指针*/
	mutex_destructor_t	destructor;	/*对象撤销回收函数指针*/
	mutex_check_t		check;
	mutex_reset_t		reset;

	char*				name;		/*pool name*/
}mutex_pool_t;

mutex_pool_t*			mutex_pool_create(const char* name, size_t ob_size, size_t array_size, mutex_constructor_t constructor, mutex_destructor_t destructor,
	mutex_check_t check, mutex_reset_t reset);

void					mutex_pool_destroy(mutex_pool_t* pool);

void*					mutex_pool_alloc(mutex_pool_t* pool);

void					mutex_pool_free(mutex_pool_t* pool, void* ob);

void					mutex_pool_print(mutex_pool_t* pool);

int32_t					mutex_get_pool_info(mutex_pool_t* pool, char* buf);

#endif
