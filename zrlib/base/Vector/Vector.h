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
	size_t (*fsdataSize)(ZRVector *vec);
	size_t (*fstrategySize)();

	/**
	 * (optional)
	 */
	void (*finitVec)(ZRVector *vec);

	/*
	 * The insert/delete functions are responsible to update properly the vec.nbObj value.
	 */
	void (*finsert)(ZRVector *vec, size_t pos, size_t nb);
	void (*fdelete)(ZRVector *vec, size_t pos, size_t nb);

	/**
	 * Clean the memory used by the vector.
	 * The vector MUST NOT be used after this call.
	 */
	void (*fdone)(ZRVector *vec);
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

static inline void ZRVECTOR_INIT(ZRVector *vec, size_t objSize, ZRVectorStrategy *strategy)
{
	*vec = (ZRVector ) { //
		.objSize = objSize, //
		.nbObj = 0, //
		.strategy = strategy, //
		};

	/*
	 * Initialisation of the vector
	 */
	if (strategy->finitVec)
		strategy->finitVec(vec);
}

static inline void ZRVECTOR_DONE(ZRVector *vec)
{
	vec->strategy->fdone(vec);
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

static inline void ZRVECTOR_GET_NB(ZRVector *vec, size_t pos, size_t nb, void *restrict dest)
{
	ZRARRAYOP_CPY(dest, vec->objSize, nb, ZRVECTOR_GET(vec, pos));
}

static inline void ZRVECTOR_SET(ZRVector *vec, size_t pos, void *obj)
{
	ZRARRAYOP_SET(vec->array, vec->objSize, pos, obj);
}

static inline void ZRVECTOR_SET_NB(ZRVector *vec, size_t pos, size_t nb, void *src)
{
	ZRARRAYOP_DEPLACE(ZRVECTOR_GET(vec, pos), vec->objSize, nb, src);
}

static inline void ZRVECTOR_INSERT(ZRVector *vec, size_t pos, void *restrict obj)
{
	vec->strategy->finsert(vec, pos, 1);
	ZRVECTOR_SET(vec, pos, obj);
}

static inline void ZRVECTOR_INSERT_NB(ZRVector *vec, size_t pos, size_t nb, void *restrict src)
{
	vec->strategy->finsert(vec, pos, nb);
	ZRVECTOR_SET_NB(vec, pos, nb, src);
}

static inline void ZRVECTOR_FILL(ZRVector *vec, size_t pos, size_t nb, void *restrict obj)
{
	vec->strategy->finsert(vec, pos, nb);
	ZRARRAYOP_FILL(ZRVECTOR_GET(vec, pos), vec->objSize, nb, obj);
}

static inline void ZRVECTOR_DELETE(ZRVector *vec, size_t pos)
{
	vec->strategy->fdelete(vec, pos, 1);
}

static inline void ZRVECTOR_DELETE_NB(ZRVector *vec, size_t pos, size_t nb)
{
	vec->strategy->fdelete(vec, pos, nb);
}

static inline void ZRVECTOR_DELETE_ALL(ZRVector *vec)
{
	vec->strategy->fdelete(vec, 0, vec->nbObj);
}

static inline void ZRVECTOR_ADD(ZRVector *vec, void *restrict obj)
{
	ZRVECTOR_INSERT(vec, vec->nbObj, obj);
}

static inline void ZRVECTOR_ADD_NB(ZRVector *vec, size_t nb, void *restrict src)
{
	ZRVECTOR_INSERT_NB(vec, vec->nbObj, nb, src);
}

static inline void ZRVECTOR_ADDFIRST(ZRVector *vec, void *restrict obj)
{
	ZRVECTOR_INSERT(vec, 0, obj);
}

static inline void ZRVECTOR_ADDFIRST_NB(ZRVector *vec, size_t nb, void *restrict src)
{
	ZRVECTOR_INSERT_NB(vec, 0, nb, src);
}

static inline void ZRVECTOR_DEC(ZRVector *vec)
{
	ZRVECTOR_DELETE(vec, vec->nbObj - 1);
}

static inline void ZRVECTOR_DEC_NB(ZRVector *vec, size_t nb)
{
	ZRVECTOR_DELETE_NB(vec, vec->nbObj - nb, nb);
}

static inline void ZRVECTOR_DECFIRST(ZRVector *vec)
{
	ZRVECTOR_DELETE(vec, 0);
}

static inline void ZRVECTOR_DECFIRST_NB(ZRVector *vec, size_t nb)
{
	ZRVECTOR_DELETE_NB(vec, 0, nb);
}

static inline void ZRVECTOR_POP(ZRVector *vec, void *restrict dest)
{
	ZRARRAYOP_CPY(dest, vec->objSize, 1, ZRVECTOR_GET(vec, vec->nbObj - 1));
	ZRVECTOR_DEC(vec);
}

static inline void ZRVECTOR_POP_NB(ZRVector *vec, size_t nb, void *restrict dest)
{
	ZRARRAYOP_CPY(dest, vec->objSize, nb, ZRVECTOR_GET(vec, vec->nbObj - 1));
	ZRVECTOR_DEC_NB(vec, nb);
}

static inline void ZRVECTOR_POPFIRST(ZRVector *vec, void *restrict dest)
{
	ZRARRAYOP_CPY(dest, vec->objSize, 1, ZRVECTOR_GET(vec, 0));
	ZRVECTOR_DECFIRST(vec);
}

static inline void ZRVECTOR_POPFIRST_NB(ZRVector *vec, size_t nb, void *restrict dest)
{
	ZRARRAYOP_CPY(dest, vec->objSize, nb, ZRVECTOR_GET(vec, 0));
	ZRVECTOR_DECFIRST_NB(vec, nb);
}

// ============================================================================

void ZRVector_init(ZRVector *vec, size_t objSize, ZRVectorStrategy *strategy);
void ZRVector_done(ZRVector *vec);

size_t ZRVector_nbObj(_ ZRVector *vec);
size_t ZRVector_objSize(ZRVector *vec);

void*_ ZRVector_get(__ ZRVector *vec, size_t pos);
void _ ZRVector_get_nb(ZRVector *vec, size_t pos, size_t nb, void *restrict dest);
void _ ZRVector_set(__ ZRVector *vec, size_t pos, __________ void *obj);
void _ ZRVector_set_nb(ZRVector *vec, size_t pos, size_t nb, void *src);
void _ ZRVector_fill(_ ZRVector *vec, size_t pos, size_t nb, void *obj);

void ZRVector_insert(__ ZRVector *vec, size_t pos, __________ void *obj);
void ZRVector_insert_nb(ZRVector *vec, size_t pos, size_t nb, void *src);

void ZRVector_delete(_____ ZRVector *vec, size_t pos);
void ZRVector_delete_nb(__ ZRVector *vec, size_t pos, size_t nb);
void ZRVector_delete_all(_ ZRVector *vec);

void ZRVector_add(_______ ZRVector *vec, __________ void *obj);
void ZRVector_add_nb(____ ZRVector *vec, size_t nb, void *src);
void ZRVector_addFirst(__ ZRVector *vec, __________ void *obj);
void ZRVector_addFirst_nb(ZRVector *vec, size_t nb, void *src);

void ZRVector_dec(_______ ZRVector *vec);
void ZRVector_dec_nb(____ ZRVector *vec, size_t nb);
void ZRVector_decFirst(__ ZRVector *vec);
void ZRVector_decFirst_nb(ZRVector *vec, size_t nb);

void ZRVector_pop(_______ ZRVector *vec, __________ void *restrict dest);
void ZRVector_pop_nb(____ ZRVector *vec, size_t nb, void *restrict dest);
void ZRVector_popFirst(__ ZRVector *vec, __________ void *restrict dest);
void ZRVector_popFirst_nb(ZRVector *vec, size_t nb, void *restrict dest);

#endif
