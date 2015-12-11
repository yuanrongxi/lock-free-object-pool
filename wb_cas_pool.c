#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "wb_cas_pool.h"

#define HEAD_PTR(ptr)	(((char*)(ptr)) - sizeof(cas_object_t))
#define OBJ_PTR(head)	(char*)((head)->ptr)

#define CAS_MAGIC	0x9875ef6d

/* idea from go's compiler core */
static inline uint64_t cas_obj_pack(cas_object_t* ob)
{
	return (((uint64_t)ob) << 16) | (ob->index & ((1 << 16) - 1));
}

static inline cas_object_t* cas_obj_unpack(uint64_t node)
{
	return (cas_object_t*)(node >> 16);
}

/* call this function when program init! */
cas_pool_t* cas_pool_create(const char* name, size_t ob_size, cas_constructor_t constructor, cas_destructor_t destructor,
							cas_check_t check, cas_reset_t reset)
{
	cas_pool_t* pool = (cas_pool_t *)calloc(1, sizeof(cas_pool_t));
	char* nm = strdup(name);
	if (nm == NULL || pool == NULL){
		free(nm);
		free(pool);
		return NULL;
	}

	pool->name = nm;
	pool->ob_size = ob_size;
	pool->constructor = constructor;
	pool->destructor = destructor;
	pool->check = check;
	pool->reset = reset;

	return pool;
}

/* call this function before program destroy */
void cas_pool_destroy(cas_pool_t* pool)
{
	uint64_t node, next;
	cas_object_t* ob;
	while (pool->header != 0){
		ob = cas_obj_unpack(pool->header);

		next = ob->next;
		pool->destructor(OBJ_PTR(ob));
		free(ob);

		pool->header = next;
	}

	free(pool->name);
	free(pool);
}

static inline void* cas_alloc()
{
	cas_object_t* obj = (cas_object_t* )malloc(sizeof(cas_object_t));
	obj->magic = CAS_MAGIC;
	obj->next = 0;
	obj->index = 0;

	return OBJ_PTR(obj);
}

void* cas_pool_alloc(cas_pool_t* pool)
{
	void* ptr;
	cas_object_t* ob;
	uint64_t node, next;

	do{
		node = pool->header;
		WT_READ_BARRIER();

		if (node == 0){
			ptr = cas_alloc();
			pool->constructor(ptr);

			goto exit_lapel;
		}

		/* ptr + node->index */
		ob = cas_obj_unpack(node);
		next = ob->next;
	} while (!__WT_ATOMIC_CAS(pool->header, node, next, sizeof(uint64_t)));

	ptr = OBJ_PTR(ob);
	WT_ATOMIC_SUB4(pool->pool_size, 1);

exit_lapel:
	pool->reset(ptr, 1);

	return ptr;
}

void cas_pool_free(cas_pool_t* pool, void* ptr)
{
	uint64_t old, node;
	cas_object_t* ob = HEAD_PTR(ptr);

	if (ob->magic != CAS_MAGIC || pool->check(ptr) != 0){
		assert(0);
		return;
	}

	pool->reset(ptr, 0);
	
	/* generate global node! double-CAS */
	++ob->index;
	node = cas_obj_pack(ob);

	do{
		old = pool->header;
		WT_READ_BARRIER();

		ob->next = old;
	} while (!__WT_ATOMIC_CAS(pool->header, old, node, sizeof(uint64_t)));

	WT_ATOMIC_ADD4(pool->pool_size, 1);
}

void cas_pool_print(cas_pool_t* pool)
{
	if (pool == NULL){
		printf("cas pool's ptr = NULL!");
		return;
	}

	printf("cas pool:\n");
	printf("\t name = %s\n\tob size = %d\n\tarray size = %d\n", pool->name, (int)pool->ob_size, pool->pool_size);
}

int32_t cas_get_pool_info(cas_pool_t* pool, char* buf)
{
	int32_t pos = 0;
	assert(pool != NULL);

	pos = sprintf(buf, "%s pool:\n", pool->name);
	pos += sprintf(buf + pos, "\tob size = %d\n\tarray size = %d\n", (int)pool->ob_size, pool->pool_size);

	return pos;
}