#ifndef __WB_CAS_POOL_H_
#define __WB_CAS_POOL_H_

#include <stdlib.h>
#include <stdint.h>

#include "wb_gcc.h"

typedef int32_t(*cas_constructor_t)(void* ob);
typedef void(*cas_destructor_t)(void* ob);
typedef int32_t(*cas_check_t)(void* ob);
typedef void(*cas_reset_t)(void* ob, int32_t flag);

typedef struct cas_object_t
{
	uint64_t				next;
	uint32_t				magic;
	uint32_t				index;
	char					ptr[];
}cas_object_t;

typedef struct CACHE_ALIGN cas_pool_t
{
	uint64_t				header;
	int32_t					pool_size;

	size_t					ob_size;

	cas_constructor_t		constructor; 
	cas_destructor_t		destructor;	
	cas_check_t				check;
	cas_reset_t				reset;

	char*					name;		/*pool name*/
}cas_pool_t;

cas_pool_t*			cas_pool_create(const char* name, size_t ob_size, cas_constructor_t constructor, cas_destructor_t destructor,
							cas_check_t check, cas_reset_t reset);

void				cas_pool_destroy(cas_pool_t* pool);
void*				cas_pool_alloc(cas_pool_t* pool);
void				cas_pool_free(cas_pool_t* pool, void* ptr);

void				cas_pool_print(cas_pool_t* pool);
int32_t				cas_get_pool_info(cas_pool_t* pool, char* buf);

#endif



