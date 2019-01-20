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

typedef struct ZRVectorS ZRVector;

typedef struct
{
	void (*finsert)(ZRVector *vec, size_t pos);
	void (*fdelete)(ZRVector *vec, size_t pos);
} ZRVectorStrategy;

struct ZRVectorS
{
	size_t objSize;
	size_t nbObj;

	ZRAllocator *allocator;
	ZRVectorStrategy *strategy;

	// Memory segment, or unused first memory space
	void * restrict FSpace;

	// Effectively used memory space
	void * restrict USpace;

	// Unused end memory space
	void * restrict ESpace;

	// First address of outside memory segment
	void * restrict OSpace;
};

// ============================================================================

#define ZRVECTOR_FSPACE_SIZEOF(V) ((V)->USpace - (V)->FSpace)
#define ZRVECTOR_USPACE_SIZEOF(V) ((V)->ESpace - (V)->USpace)
#define ZRVECTOR_ESPACE_SIZEOF(V) ((V)->OSpace - (V)->ESpace)
#define ZRVECTOR_TOTALSPACE_SIZEOF(V)  ((V)->OSpace - (V)->FSpace)
#define ZRVECTOR_FREESPACE_SIZEOF(V) (ZRVECTOR_FSPACE_SIZEOF(V) + ZRVECTOR_ESPACE_SIZEOF(V))

// ============================================================================

static inline ZRVector ZRVECTOR_INIT(size_t objSize, ZRAllocator *allocator, ZRVectorStrategy *strategy)
{
	ZRVector ret =
		{ .objSize = objSize, .allocator = allocator, .strategy = strategy };
	return ret;
}

static inline void ZRVECTOR_CONSTRUCT(ZRVector *vec, size_t objSize, ZRAllocator *allocator, ZRVectorStrategy *strategy)
{
	*vec = ZRVECTOR_INIT(objSize, allocator, strategy);
}

static inline size_t ZRVECTOR_SIZE(ZRVector *vec)
{
	return vec->nbObj;
}

static inline void* ZRVECTOR_GET(ZRVector *vec, size_t pos)
{
	return ZRARRAYOP_GET(vec->USpace, vec->objSize, pos);
}

static inline void ZRVECTOR_SET(ZRVector *vec, size_t pos, void *obj)
{
	ZRARRAYOP_SET(vec->USpace, vec->objSize, pos, obj);
}

static inline void ZRVECTOR_INSERT(ZRVector *vec, size_t pos, void *obj)
{
	vec->strategy->finsert(vec, pos);
	ZRVECTOR_SET(vec, pos, obj);
}

static inline void ZRVECTOR_DELETE(ZRVector *vec, size_t pos)
{
	vec->strategy->fdelete(vec, pos);
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
	memcpy(dest, vec->USpace, vec->objSize);
	ZRVECTOR_DECFIRST(vec);
}

// ============================================================================

ZRVector ZRVector_init/* */(/*           */size_t objSize, ZRAllocator *allocator, ZRVectorStrategy *strategy);
void/* */ZRVector_construct(ZRVector *vec, size_t objSize, ZRAllocator *allocator, ZRVectorStrategy *strategy);

size_t ZRVector_size(ZRVector *vec);

/**/void* ZRVector_get(ZRVector *vec, size_t pos);
/* */void ZRVector_set(ZRVector *vec, size_t pos, void *obj);

void ZRVector_insert(ZRVector *vec, size_t pos, void *obj);
void ZRVector_delete(ZRVector *vec, size_t pos);

void ZRVector_add/* */(ZRVector *vec, void *obj);
void ZRVector_addFirst(ZRVector *vec, void *obj);

void ZRVector_dec/* */(ZRVector *vec);
void ZRVector_decFirst(ZRVector *vec);

void ZRVector_pop/* */(ZRVector *vec, void *dest);
void ZRVector_popFirst(ZRVector *vec, void *dest);

#endif
