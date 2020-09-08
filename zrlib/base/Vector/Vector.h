/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#ifndef ZRVECTOR_H
#define ZRVECTOR_H

#include <zrlib/config.h>
#include <zrlib/base/Array.h>
#include <zrlib/base/Allocator/Allocator.h>

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <string.h>

// ============================================================================

typedef struct ZRVectorS ZRVector;
typedef struct ZRVectorStrategyS ZRVectorStrategy;

// ============================================================================

struct ZRVectorStrategyS
{
	/**
	 * (optional)
	 */
	void (*finitVec)(ZRVector *vec);

	/*
	 * The insert/delete functions are responsible to update properly the vec.nbObj value.
	 */
	void (*finsert)(ZRVector *vec, size_t pos, size_t nb);
	void (*fdelete)(ZRVector *vec, size_t pos, size_t nb);


	void (*fchangeObjSize)(ZRVector *vec, size_t objSize, size_t objAlignment);
	void (*fmemoryTrim)(ZRVector *vec);

	/**
	 * Clean the memory used by the vector.
	 * The vector MUST NOT be used after this call.
	 */
	void (*fdone)(ZRVector *vec);

	/**
	 * @optional
	 */
	void (*fdestroy)(ZRVector *vec);
};

struct ZRVectorS
{
	/*
	 * The strategy for memory management and insertion/deletion routines.
	 */
	ZRVectorStrategy *strategy;

	ZRArray array;
};

#define ZRVECTOR_LVPARRAY(V) (V)->array.array
#define ZRVECTOR_LVNBOBJ(V) (V)->array.nbObj
#define ZRVECTOR_LVSIZE(V) (V)->array.size
#define ZRVECTOR_LVOBJINFOS(V) (V)->array.objInfos
#define ZRVECTOR_LVOBJSIZE(V) (V)->array.objInfos.size
#define ZRVECTOR_LVOBJALIGNMENT(V) (V)->array.objInfos.alignment

#define ZRVECTOR_CAPACITY ZRVECTOR_SIZE
#define ZRVector_capacity ZRVector_size

// ============================================================================

static inline void ZRVECTOR_ADD_NB(ZRVector *vec, size_t nb, void *src);
static inline void ZRVECTOR_DELETE_ALL(ZRVector *vec);

// ============================================================================
// ROUTINE FUNCTION

static inline void ZRVECTOR_FINSERT(ZRVector *vec, size_t pos, size_t nb)
{
	vec->strategy->finsert(vec, pos, nb);
}

static inline void ZRVECTOR_FDELETE(ZRVector *vec, size_t pos, size_t nb)
{
	vec->strategy->fdelete(vec, pos, nb);
}

// ============================================================================

ZRMUSTINLINE
static inline void ZRVECTOR_INIT(ZRVector *vec, size_t objSize, size_t objAlignment, ZRVectorStrategy *strategy)
{
	*vec = ((ZRVector)
		{ //
			.array = (ZRArray) {
				.objInfos = ZROBJINFOS_DEF(objAlignment, objSize),
			},
			.strategy = strategy,//
		})
	;

	/*
	 * Initialisation of the vector
	 */
	if (strategy->finitVec)
		strategy->finitVec(vec);
}

ZRMUSTINLINE
static inline void ZRVECTOR_COPY(ZRVector *restrict dest, ZRVector *restrict src)
{
	assert(dest->array.objInfos.alignment == src->array.objInfos.alignment);
	ZRVECTOR_DELETE_ALL(dest);
// TODO: function changeSize()
	dest->array.objInfos.size = src->array.objInfos.size,
	dest->array.nbObj = 0;
	ZRVECTOR_ADD_NB(dest, src->array.nbObj, src->array.array);
}

ZRMUSTINLINE
static inline void ZRVECTOR_DONE(ZRVector *vec)
{
	vec->strategy->fdone(vec);
}

ZRMUSTINLINE
static inline void ZRVECTOR_DESTROY(ZRVector *vec)
{
	if (vec->strategy->fdestroy != NULL)
		vec->strategy->fdestroy(vec);
}

ZRMUSTINLINE
static inline void* ZRVECTOR_PARRAY(ZRVector *vec)
{
	return ZRVECTOR_LVPARRAY(vec);
}

ZRMUSTINLINE
static inline size_t ZRVECTOR_NBOBJ(ZRVector *vec)
{
	return ZRVECTOR_LVNBOBJ(vec);
}

ZRMUSTINLINE
static inline size_t ZRVECTOR_OBJSIZE(ZRVector *vec)
{
	return ZRVECTOR_LVOBJSIZE(vec);
}

ZRMUSTINLINE
static inline size_t ZRVECTOR_SIZE(ZRVector *vec)
{
	return ZRVECTOR_LVSIZE(vec);
}

ZRMUSTINLINE
static inline size_t ZRVECTOR_OBJALIGNMENT(ZRVector *vec)
{
	return ZRVECTOR_LVOBJALIGNMENT(vec);
}

ZRMUSTINLINE
static inline void* ZRVECTOR_GET(ZRVector *vec, size_t pos)
{
	return ZRARRAYOP_GET(vec->array.array, ZRVECTOR_OBJSIZE(vec), pos);
}

ZRMUSTINLINE
static inline void ZRVECTOR_GET_NB(ZRVector *vec, size_t pos, size_t nb, void *dest)
{
	ZRARRAYOP_CPY(dest, ZRVECTOR_OBJSIZE(vec), nb, ZRVECTOR_GET(vec, pos));
}

ZRMUSTINLINE
static inline void ZRVECTOR_SET(ZRVector *vec, size_t pos, void *obj)
{
	ZRARRAYOP_SET(vec->array.array, ZRVECTOR_OBJSIZE(vec), pos, obj);
}

ZRMUSTINLINE
static inline void ZRVECTOR_SET_NB(ZRVector *vec, size_t pos, size_t nb, void *src)
{
	ZRARRAYOP_DEPLACE(ZRVECTOR_GET(vec, pos), ZRVECTOR_OBJSIZE(vec), nb, src);
}

ZRMUSTINLINE
static inline void ZRVECTOR_INSERT(ZRVector *vec, size_t pos, void *obj)
{
	vec->strategy->finsert(vec, pos, 1);
	ZRVECTOR_SET(vec, pos, obj);
}

ZRMUSTINLINE
static inline void ZRVECTOR_INSERT_NB(ZRVector *vec, size_t pos, size_t nb, void *src)
{
	vec->strategy->finsert(vec, pos, nb);
	ZRVECTOR_SET_NB(vec, pos, nb, src);
}

ZRMUSTINLINE
static inline void ZRVECTOR_CHANGEOBJSIZE(ZRVector *vec, size_t objSize, size_t objAlignment)
{
	vec->strategy->fchangeObjSize(vec, objSize, objAlignment);
}

ZRMUSTINLINE
static inline void ZRVECTOR_MEMORYTRIM(ZRVector *vec)
{
	vec->strategy->fmemoryTrim(vec);
}

ZRMUSTINLINE
static inline void ZRVECTOR_FILL(ZRVector *vec, size_t pos, size_t nb, void *obj)
{
	vec->strategy->finsert(vec, pos, nb);
	ZRARRAYOP_FILL(ZRVECTOR_GET(vec, pos), ZRVECTOR_OBJSIZE(vec), nb, obj);
}

ZRMUSTINLINE
static inline void ZRVECTOR_RESERVE(ZRVector *vec, size_t pos, size_t nb)
{
	vec->strategy->finsert(vec, pos, nb);
}

ZRMUSTINLINE
static inline void ZRVECTOR_DELETE(ZRVector *vec, size_t pos)
{
	vec->strategy->fdelete(vec, pos, 1);
}

ZRMUSTINLINE
static inline void ZRVECTOR_DELETE_NB(ZRVector *vec, size_t pos, size_t nb)
{
	vec->strategy->fdelete(vec, pos, nb);
}

ZRMUSTINLINE
static inline void ZRVECTOR_DELETE_ALL(ZRVector *vec)
{
	vec->strategy->fdelete(vec, 0, ZRVECTOR_NBOBJ(vec));
}

ZRMUSTINLINE
static inline void ZRVECTOR_ADD(ZRVector *vec, void *obj)
{
	ZRVECTOR_INSERT(vec, ZRVECTOR_NBOBJ(vec), obj);
}

ZRMUSTINLINE
static inline void ZRVECTOR_ADD_NB(ZRVector *vec, size_t nb, void *src)
{
	ZRVECTOR_INSERT_NB(vec, ZRVECTOR_NBOBJ(vec), nb, src);
}

ZRMUSTINLINE
static inline void ZRVECTOR_ADDFIRST(ZRVector *vec, void *obj)
{
	ZRVECTOR_INSERT(vec, 0, obj);
}

ZRMUSTINLINE
static inline void ZRVECTOR_ADDFIRST_NB(ZRVector *vec, size_t nb, void *src)
{
	ZRVECTOR_INSERT_NB(vec, 0, nb, src);
}

ZRMUSTINLINE
static inline void ZRVECTOR_DEC(ZRVector *vec)
{
	ZRVECTOR_DELETE(vec, ZRVECTOR_NBOBJ(vec) - 1);
}

ZRMUSTINLINE
static inline void ZRVECTOR_DEC_NB(ZRVector *vec, size_t nb)
{
	ZRVECTOR_DELETE_NB(vec, ZRVECTOR_NBOBJ(vec) - nb, nb);
}

ZRMUSTINLINE
static inline void ZRVECTOR_DECFIRST(ZRVector *vec)
{
	ZRVECTOR_DELETE(vec, 0);
}

ZRMUSTINLINE
static inline void ZRVECTOR_DECFIRST_NB(ZRVector *vec, size_t nb)
{
	ZRVECTOR_DELETE_NB(vec, 0, nb);
}

static inline void ZRVECTOR_POP(ZRVector *vec, void *dest)
{
	ZRARRAYOP_CPY(dest, ZRVECTOR_OBJSIZE(vec), 1, ZRVECTOR_GET(vec, ZRVECTOR_NBOBJ(vec) - 1));
	ZRVECTOR_DEC(vec);
}

ZRMUSTINLINE
static inline void ZRVECTOR_POP_NB(ZRVector *vec, size_t nb, void *dest)
{
	ZRARRAYOP_CPY(dest, ZRVECTOR_OBJSIZE(vec), nb, ZRVECTOR_GET(vec, ZRVECTOR_NBOBJ(vec) - 1));
	ZRVECTOR_DEC_NB(vec, nb);
}

ZRMUSTINLINE
static inline void ZRVECTOR_POPFIRST(ZRVector *vec, void *dest)
{
	ZRARRAYOP_CPY(dest, ZRVECTOR_OBJSIZE(vec), 1, ZRVECTOR_GET(vec, 0));
	ZRVECTOR_DECFIRST(vec);
}

ZRMUSTINLINE
static inline void ZRVECTOR_POPFIRST_NB(ZRVector *vec, size_t nb, void *dest)
{
	ZRARRAYOP_CPY(dest, ZRVECTOR_OBJSIZE(vec), nb, ZRVECTOR_GET(vec, 0));
	ZRVECTOR_DECFIRST_NB(vec, nb);
}

// Pointer help functions

ZRMUSTINLINE
static inline void ZRVECTOR_SETPTR_NB(ZRVector *vec, size_t pos, size_t nb, void *src, size_t srcObjSize)
{
	assert(ZRVECTOR_OBJSIZE(vec) == sizeof(void*));

	for (; nb; nb--, pos++)
	{
		ZRVECTOR_SET(vec, pos, &src);
		src = (char*)src + srcObjSize;
	}
}

ZRMUSTINLINE
static inline void ZRVECTOR_INSERTPTR_NB(ZRVector *vec, size_t pos, size_t nb, void *src, size_t srcObjSize)
{
	vec->strategy->finsert(vec, pos, nb);
	ZRVECTOR_SETPTR_NB(vec, pos, nb, src, srcObjSize);
}

ZRMUSTINLINE
static inline void ZRVECTOR_ADDPTR_NB(ZRVector *vec, size_t nb, void *src, size_t srcObjSize)
{
	ZRVECTOR_INSERTPTR_NB(vec, ZRVECTOR_NBOBJ(vec), nb, src, srcObjSize);
}

ZRMUSTINLINE
static inline void ZRVECTOR_ADDFIRSTPTR_NB(ZRVector *vec, size_t nb, void *src, size_t srcObjSize)
{
	ZRVECTOR_INSERTPTR_NB(vec, 0, nb, src, srcObjSize);
}

// ============================================================================

void ZRVector_init(ZRVector *vec, size_t objSize, size_t objAlignment, ZRVectorStrategy *strategy);
void ZRVector_copy(ZRVector *restrict dest, ZRVector *restrict src);
void ZRVector_done(ZRVector *vec);
void ZRVector_destroy(ZRVector *vec);
void ZRVector_changeObjSize(ZRVector *vec, size_t objSize, size_t objAlignment);
void ZRVector_memoryTrim(ZRVector *vec);

void *ZRVector_parray(ZRVector *vec);
size_t ZRVector_nbObj(_ ZRVector *vec);
size_t ZRVector_objSize(ZRVector *vec);
size_t ZRVector_objAlignment(ZRVector *vec);
size_t ZRVector_size(ZRVector *vec);

void* ZRVector_get(__ ZRVector *vec, size_t pos);
void _ ZRVector_get_nb(ZRVector *vec, size_t pos, size_t nb, void *restrict dest);
void _ ZRVector_set(__ ZRVector *vec, size_t pos, __________ void *obj);
void _ ZRVector_set_nb(ZRVector *vec, size_t pos, size_t nb, void *src);
void _ ZRVector_fill(_ ZRVector *vec, size_t pos, size_t nb, void *obj);
void _ ZRVector_reserve(ZRVector *vec, size_t pos, size_t nb);

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

// ============================================================================
// Pointer help functions

void ZRVector_setPtr_nb(____ ZRVector *vec, size_t pos, size_t nb, void *src, size_t srcObjSize);
void ZRVector_insertPtr_nb(_ ZRVector *vec, size_t pos, size_t nb, void *src, size_t srcObjSize);
void ZRVector_addPtr_nb(____ ZRVector *vec, ___________ size_t nb, void *src, size_t srcObjSize);
void ZRVector_addFirstPtr_nb(ZRVector *vec, ___________ size_t nb, void *src, size_t srcObjSize);

#endif
