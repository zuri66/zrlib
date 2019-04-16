/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <zrlib/config.h>
#include <zrlib/base/ArrayOp.h>
#include <zrlib/base/Allocator/Allocator.h>

#include <stddef.h>
#include <string.h>

// ============================================================================

typedef struct ZRVectorS ZRVector;
typedef struct ZRVectorStrategyS ZRVectorStrategy;

// ============================================================================

struct ZRVectorStrategyS
{
	size_t (*fdataSize)(void);

	/**
	 * (optional)
	 */
	void (*finitVec)(ZRVector *vec);

	void (*finsert)(ZRVector *vec, size_t pos);
	void (*fdelete)(ZRVector *vec, size_t pos);
};

struct ZRVectorS
{
	size_t objSize;
	size_t nbObj;

	/*
	 * The strategy for memory management and insertion/deletion routines.
	 */
	ZRVectorStrategy *strategy;

	/*
	 * The array of the vector's objects.
	 */
	void *array;

	/*
	 * Data for Strategy purpose.
	 */
	char sdata[];
};

// ============================================================================

#define ZRVECTOR_SIZEOF_STRUCT(strategy) (sizeof(ZRVector) + strategy->fdataSize())
#define ZRVECTOR_SIZEOF_VEC(vec) (ZRVECTOR_SIZEOF_STRUCT(vec->strategy))

// ============================================================================

static inline void ZRVECTOR_INIT(ZRVector *vec, size_t objSize, ZRVectorStrategy *strategy)
{
	ZRVector const tmp = { //
		.objSize = objSize, //
		.nbObj = 0, //
		.strategy = strategy //
		};

	*vec = tmp;

	if (strategy->finitVec)
		strategy->finitVec(vec);
	else
		memset(vec->sdata, 0, strategy->fdataSize());
}

static inline size_t ZRVECTOR_NBOBJ(ZRVector *vec)
{
	return vec->nbObj;
}

static inline size_t ZRVECTOR_OBJSIZE(ZRVector *vec)
{
	return vec->objSize;
}

static inline void* ZRVECTOR_GET(ZRVector *vec, size_t pos)
{
	return ZRARRAYOP_GET(vec->array, vec->objSize, pos);
}

static inline void ZRVECTOR_SET(ZRVector *vec, size_t pos, void *obj)
{
	ZRARRAYOP_SET(vec->array, vec->objSize, pos, obj);
}

static inline void ZRVECTOR_INSERT(ZRVector *vec, size_t pos, void *obj)
{
	vec->strategy->finsert(vec, pos);
	ZRVECTOR_SET(vec, pos, obj);
	vec->nbObj++;
}

static inline void ZRVECTOR_DELETE(ZRVector *vec, size_t pos)
{
	vec->strategy->fdelete(vec, pos);
	vec->nbObj--;
}

static inline void ZRVECTOR_ADD(ZRVector *vec, void *obj)
{
	ZRVECTOR_INSERT(vec, vec->nbObj, obj);
}

static inline void ZRVECTOR_ADDFIRST(ZRVector *vec, void *obj)
{
	ZRVECTOR_INSERT(vec, 0, obj);
}

static inline void ZRVECTOR_DEC(ZRVector *vec)
{
	ZRVECTOR_DELETE(vec, vec->nbObj - 1);
}

static inline void ZRVECTOR_DECFIRST(ZRVector *vec)
{
	ZRVECTOR_DELETE(vec, 0);
}

static inline void ZRVECTOR_POP(ZRVector *vec, void *dest)
{
	memcpy(dest, ZRVECTOR_GET(vec, vec->nbObj - 1), vec->objSize);
	ZRVECTOR_DEC(vec);
}

static inline void ZRVECTOR_POPFIRST(ZRVector *vec, void *dest)
{
	memcpy(dest, ZRVECTOR_GET(vec, 0), vec->objSize);
	ZRVECTOR_DECFIRST(vec);
}

// ============================================================================

void ZRVector_init(ZRVector *vec, size_t objSize, ZRVectorStrategy *strategy);

size_t ZRVector_nbObj(_ ZRVector *vec);
size_t ZRVector_objSize(ZRVector *vec);

void*_ ZRVector_get(ZRVector *vec, size_t pos);
void _ ZRVector_set(ZRVector *vec, size_t pos, void *obj);

void ZRVector_insert(ZRVector *vec, size_t pos, void *obj);
void ZRVector_delete(ZRVector *vec, size_t pos);

void ZRVector_add(____ ZRVector *vec, void *obj);
void ZRVector_addFirst(ZRVector *vec, void *obj);

void ZRVector_dec(____ ZRVector *vec);
void ZRVector_decFirst(ZRVector *vec);

void ZRVector_pop(____ ZRVector *vec, void *dest);
void ZRVector_popFirst(ZRVector *vec, void *dest);

#endif
