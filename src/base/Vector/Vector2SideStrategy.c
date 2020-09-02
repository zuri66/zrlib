/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#include <zrlib/base/ResizeOp.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/struct.h>

#include <assert.h>
#include <stdbool.h>
#include <stdalign.h>

#define INITIAL_SIZE 512

typedef struct ZRVector2SideStrategyS ZRVector2SideStrategy;
typedef struct ZR2SSVectorS ZR2SSVector;

// ============================================================================

struct ZRVector2SideStrategyS
{
	ZRVectorStrategy strategy;

	bool _ (*fmustGrow)(____ size_t totalSpace, size_t usedSpace, void *vec_p);
	size_t (*fincreaseSpace)(size_t totalSpace, size_t usedSpace, void *vec_p);

	bool _ (*fmustShrink)(__ size_t totalSpace, size_t usedSpace, void *vec_p);
	size_t (*fdecreaseSpace)(size_t totalSpace, size_t usedSpace, void *vec_p);
};

#define ZR2SSSTRATEGY_VECTOR(S) (&(S)->strategy)

// ============================================================================

typedef enum
{
	ZRVectorInfos_base = 0,
	ZRVectorInfos_objs,
	ZRVectorInfos_strategy,
	ZRVectorInfos_struct,
	ZRVECTOR_INFOS_NB,
} ZRVectorInfos;

struct ZR2SSVectorS
{
	ZRVector vector;

	size_t initialArrayOffset;

	/*
	 * In bytes
	 */
	size_t initialArraySize;

	size_t initialMemoryNbObjs;

	ZRAllocator *allocator;

	/*
	 * Memory segment, or unused first memory space
	 */
	void *restrict FSpace;

	/*
	 * Effectively used memory space
	 * Replaced by the 'array' field of ZRVector
	 */
	// void * restrict USpace;
	//
	/*
	 * Unused end memory space
	 */
	void *restrict ESpace;

	/*
	 * First address of outside memory segment
	 */
	void *restrict OSpace;

	unsigned char *allocatedMemory;
	unsigned char *initialArray;

	unsigned staticStrategy :1;
};

// ============================================================================

#define ZRDATA_INITIALARRAY(V) ((V)->initialArray)
#define ZRVECTOR_INITIALARRAY(V) ZRDATA_INITIALARRAY(ZRVECTOR_2SS(V))

ZRMUSTINLINE
static inline size_t checkInitialNbObj(size_t initialNbObj)
{
	size_t const minimumNbObj = 2;

	if (initialNbObj > 0 && initialNbObj < minimumNbObj)
		return minimumNbObj;

	return initialNbObj;
}

static void getVectorInfos(ZRObjAlignInfos *out, size_t initialArrayNbObj, size_t objSize, size_t objAlignment, bool staticStrategy)
{
	initialArrayNbObj = checkInitialNbObj(initialArrayNbObj);

	out[ZRVectorInfos_base] = (ZRObjAlignInfos ) { 0, ZRTYPE_ALIGNMENT_SIZE(ZR2SSVector) };
	out[ZRVectorInfos_objs] = (ZRObjAlignInfos ) { 0, objAlignment, objSize * initialArrayNbObj };
	out[ZRVectorInfos_strategy] = staticStrategy ? (ZRObjAlignInfos ) { 0, ZRTYPE_ALIGNMENT_SIZE(ZRVector2SideStrategy) } : (ZRObjAlignInfos ) { };
	out[ZRVectorInfos_struct] = (ZRObjAlignInfos ) { };
	ZRSTRUCT_MAKEOFFSETS(ZRVECTOR_INFOS_NB - 1, out);
}

// ============================================================================

#define ZR2SS_VECTOR(V) (&(V)->vector)
#define ZRVECTOR_2SS(V) ((ZR2SSVector*)(V))
#define ZR2SS_STRATEGY(V) ((ZRVector2SideStrategy*)(ZR2SS_VECTOR(V)->strategy))

#define ZR2SS_FSPACE(V) (V)->FSpace
#define ZR2SS_USPACE(V) ZR2SS_VECTOR(V)->array
#define ZR2SS_ESPACE(V) (V)->ESpace
#define ZR2SS_OSPACE(V) (V)->OSpace

#define ZR2SS_FSPACE_SIZEOF(V) ((char*)ZR2SS_USPACE(V) - (char*)ZR2SS_FSPACE(V))
#define ZR2SS_USPACE_SIZEOF(V) ((char*)ZR2SS_ESPACE(V) - (char*)ZR2SS_USPACE(V))
#define ZR2SS_ESPACE_SIZEOF(V) ((char*)ZR2SS_OSPACE(V) - (char*)ZR2SS_ESPACE(V))
#define ZR2SS_TOTALSPACE_SIZEOF(V) ((char*)ZR2SS_OSPACE(V) - (char*)ZR2SS_FSPACE(V))
#define ZR2SS_FREESPACE_SIZEOF(V) (ZR2SS_FSPACE_SIZEOF(V) + ZR2SS_ESPACE_SIZEOF(V))

// ============================================================================

void ZRVector2SideStrategy_finsert(ZRVector *vec, size_t pos, size_t nb);
void ZRVector2SideStrategy_fdelete(ZRVector *vec, size_t pos, size_t nb);

void ZRVector2SideStrategy_finsertGrow(ZRVector *vec, size_t pos, size_t nb);
void ZRVector2SideStrategy_fdeleteShrink(ZRVector *vec, size_t pos, size_t nb);

static void ZRVector2SideStrategy_growOnAdd(ZRVectorStrategy *strategy, bool v);
static void ZRVector2SideStrategy_shrinkOnDelete(ZRVectorStrategy *strategy, bool v);

static inline void moreSize(ZRVector *vec, size_t nbObjMore);
static inline void setFUEOSpaces(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj);

// ============================================================================

ZRMUSTINLINE
static inline bool ZRVector2SideStrategy_memoryIsAllocated(ZR2SSVector *vec)
{
	return vec->allocatedMemory != NULL ;
}

// ============================================================================

#include "Vector2SideStrategy_spaceStrategies.c"

// ============================================================================

/**
 * vec must be initialized to zero before.
 */
static
void ZRVector2SideStrategy_finitVec(ZRVector *vec)
{
	ZR2SSVector *svector = ZRVECTOR_2SS(vec);

	if (svector->initialArraySize == 0)
	{
		// Must be set before moreSize()
		ZR2SS_FSPACE(svector) =
		ZR2SS_USPACE(svector) =
		ZR2SS_OSPACE(svector) =
		ZR2SS_ESPACE(svector) = svector;
		moreSize(ZR2SS_VECTOR(svector), 0);
	}
	else
	{
		ZR2SS_VECTOR(svector)->capacity = svector->initialArraySize / vec->objSize;
		setFUEOSpaces(svector, svector->initialArray, vec->capacity, NULL, 0);
	}
}

static
void ZRVector2SideStrategy_fchangeObjSize(ZRVector *vector, size_t objSize, size_t objAlignment)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vector);
	size_t const lastAlignment = ZR2SS_VECTOR(svector)->objAlignment;

	if (ZRVector2SideStrategy_memoryIsAllocated(svector))
	{
		ZRFREE(svector->allocator, svector->allocatedMemory);
		svector->allocatedMemory = NULL;
	}
	ZR2SS_VECTOR(svector)->nbObj = 0;
	ZR2SS_VECTOR(svector)->objAlignment = objAlignment;
	ZR2SS_VECTOR(svector)->objSize = objSize;

	//Realign initial
	if (svector->initialArraySize > 0)
	{
		if (objAlignment != lastAlignment)
		{
			// alignment + 1 obj Size
			if (objAlignment * 2 <= svector->initialArraySize)
			{
				size_t const offset = ZRSTRUCT_ALIGNOFFSET(svector->initialArrayOffset, objAlignment);
				size_t const offsetDiff = offset - svector->initialArrayOffset;
				svector->initialArray = (char*)svector + offset;
				ZR2SS_VECTOR(svector)->capacity = (svector->initialArraySize - offsetDiff) / objSize;
				setFUEOSpaces(svector, svector->initialArray, ZR2SS_VECTOR(svector)->capacity, NULL, 0);
			}
			else
			{
				ZR2SS_FSPACE(svector) =
				ZR2SS_USPACE(svector) =
				ZR2SS_OSPACE(svector) =
				ZR2SS_ESPACE(svector) = svector;
				moreSize(ZR2SS_VECTOR(svector), 0);
			}
		}
	}
	else
		moreSize(ZR2SS_VECTOR(svector), 0);
}

static
void ZRVector2SideStrategy_fixedMemory(ZRVectorStrategy *strategy)
{
	ZRVector2SideStrategy_growOnAdd(strategy, false);
	ZRVector2SideStrategy_shrinkOnDelete(strategy, false);
}

static
void ZRVector2SideStrategy_dynamicMemory(ZRVectorStrategy *strategy)
{
	ZRVector2SideStrategy_growOnAdd(strategy, true);
	ZRVector2SideStrategy_shrinkOnDelete(strategy, true);
}

static
void ZRVector2SideStrategy_growOnAdd(ZRVectorStrategy *strategy, bool v)
{
	ZRVector2SideStrategy *twoSideStrategy = (ZRVector2SideStrategy*)strategy;
	twoSideStrategy->strategy.finsert = v ? ZRVector2SideStrategy_finsertGrow : ZRVector2SideStrategy_finsert;
}

static
void ZRVector2SideStrategy_shrinkOnDelete(ZRVectorStrategy *strategy, bool v)
{
	ZRVector2SideStrategy *twoSideStrategy = (ZRVector2SideStrategy*)strategy;
	twoSideStrategy->strategy.fdelete = v ? ZRVector2SideStrategy_fdeleteShrink : ZRVector2SideStrategy_fdelete;
}

void ZRVector2SideStrategy_growStrategy( //
	ZRVectorStrategy *strategy, //
	bool _ (*mustGrow)(____ size_t totalSpace, size_t usedSpace, void*), //
	size_t (*increaseSpace)(size_t totalSpace, size_t usedSpace, void*) //
	)
{
	((ZRVector2SideStrategy*)strategy)->fmustGrow = mustGrow;
	((ZRVector2SideStrategy*)strategy)->fincreaseSpace = increaseSpace;
}

void ZRVector2SideStrategy_shrinkStrategy( //
	ZRVectorStrategy *strategy, //
	bool _ (*mustShrink)(__ size_t totalSpace, size_t usedSpace, void *vec), //
	size_t (*decreaseSpace)(size_t totalSpace, size_t usedSpace, void *vec) //
	)
{
	((ZRVector2SideStrategy*)strategy)->fmustShrink = mustShrink;
	((ZRVector2SideStrategy*)strategy)->fdecreaseSpace = decreaseSpace;
}

static
void ZRVector2SideStrategy_fmemoryTrim(ZRVector *vec)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);

	if (!ZRVector2SideStrategy_memoryIsAllocated(svector))
		return;

	size_t const objSize = vec->objSize;
	size_t const nbObj = checkInitialNbObj(vec->nbObj);
	size_t const objAlignment = vec->objAlignment;
	void *lastSpace = svector->allocatedMemory;

	ZRArrayOp_deplace(ZR2SS_FSPACE(svector), objSize, nbObj, ZR2SS_USPACE(svector));
	svector->allocatedMemory = ZRAALLOC(svector->allocator, objAlignment, objSize * nbObj);
	ZRARRAYOP_CPY(svector->allocatedMemory, objSize, nbObj, lastSpace);
	ZRFREE(svector->allocator, lastSpace);

	ZR2SS_FSPACE(svector) =
	ZR2SS_USPACE(svector) = svector->allocatedMemory;
	ZR2SS_OSPACE(svector) =
	ZR2SS_ESPACE(svector) = (char*)svector->allocatedMemory + nbObj * objSize;
	ZR2SS_VECTOR(svector)->capacity = nbObj;
}

// ============================================================================

ZRMUSTINLINE
static inline size_t getInitialMemoryNbObjs(ZR2SSVector *svector)
{
	if (svector->initialMemoryNbObjs > 0)
		return svector->initialMemoryNbObjs;

	if (svector->initialArraySize > 0)
		return svector->initialArraySize / ZR2SS_VECTOR(svector)->objSize;

	return INITIAL_SIZE;
}

ZRMUSTINLINE
static inline void setFUEOSpaces(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj)
{
	assert(fspaceNbObj >= sourceNbObj);
	assert(fspaceNbObj >= 2);
	size_t const objSize = ZR2SS_VECTOR(svector)->objSize;

	ZR2SS_FSPACE(svector) = fspace;
	ZR2SS_OSPACE(svector) = (char*)ZR2SS_FSPACE(svector) + (fspaceNbObj * objSize);

	if (sourceNbObj == 0)
	{
		ZR2SS_USPACE(svector) = (char*)ZR2SS_FSPACE(svector) + ((fspaceNbObj * objSize) / 2);
		ZR2SS_ESPACE(svector) = (char*)ZR2SS_USPACE(svector);
	}
	else
	{
		ZR2SS_USPACE(svector) = (char*)ZR2SS_FSPACE(svector) + ((fspaceNbObj - sourceNbObj) * objSize) / 2;
		ZR2SS_ESPACE(svector) = (char*)ZR2SS_USPACE(svector) + (sourceNbObj * objSize);

		if (sourceNbObj > 0)
			ZRARRAYOP_CPY(ZR2SS_USPACE(svector), objSize, sourceNbObj, source);
	}
}

ZRMUSTINLINE
static inline void centerFUEOSpaces(ZR2SSVector *svector, size_t nbMore)
{
	assert(nbMore > 0);
	size_t const objSize = ZR2SS_VECTOR(svector)->objSize;
	size_t const nbObj = ZR2SS_VECTOR(svector)->nbObj;
	size_t const newNbObj = nbObj + nbMore;
	void *const lastUSpace = ZR2SS_USPACE(svector);

	size_t const uspaceOffset = (ZR2SS_TOTALSPACE_SIZEOF(svector) - (newNbObj * objSize)) / 2;
	ZR2SS_USPACE(svector) = (char*)ZR2SS_FSPACE(svector) + uspaceOffset;

// We dont add the new elements here, this is a virtual setting
	ZR2SS_ESPACE(svector) = (char*)ZR2SS_USPACE(svector) + (nbObj * objSize);

	ZRARRAYOP_CPY(ZR2SS_USPACE(svector), objSize, nbObj, lastUSpace);
}

static inline bool mustGrow(ZRVector *vec, size_t nbObjMore);

static inline void moreSize(ZRVector *vec, size_t nbObjMore)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	size_t const objAlignment = vec->objAlignment;
	size_t const objSize = vec->objSize;
	size_t const nbObj = vec->nbObj;
	size_t const totalSpace = ZR2SS_TOTALSPACE_SIZEOF(svector);

	bool const isAllocated = ZRVector2SideStrategy_memoryIsAllocated(svector);

	size_t const moreUsedSpace = nbObjMore * objSize;
	size_t const nextUsedSpace = ZR2SS_USPACE_SIZEOF(svector) + moreUsedSpace;

	zrfincreaseSpace fincreaseSpace = ZR2SS_STRATEGY(svector)->fincreaseSpace;
	zrfmustGrow fmustGrow = ZR2SS_STRATEGY(svector)->fmustGrow;

	ZRArray2 new = ZRRESIZE_MAKEMORESIZE(
		totalSpace, nextUsedSpace, getInitialMemoryNbObjs(svector) * objSize, objAlignment,
		svector->allocatedMemory, svector->allocator,
		fmustGrow, fincreaseSpace, svector
		);

	size_t const nextTotalNbObj = new.size / objSize;

	setFUEOSpaces(svector, new.array, nextTotalNbObj, ZR2SS_USPACE(svector), nbObj);

	if (isAllocated)
		ZRFREE(svector->allocator, svector->allocatedMemory);

	svector->allocatedMemory = new.array;
	ZR2SS_VECTOR(svector)->capacity = nextTotalNbObj;
}

ZRMUSTINLINE
static inline void lessSize(ZRVector *vec)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	assert(ZRVector2SideStrategy_memoryIsAllocated(svector));
	size_t const objAlignment = vec->objAlignment;
	size_t const objSize = vec->objSize;
	size_t const nbObj = vec->nbObj;
	size_t const totalSpace = ZR2SS_TOTALSPACE_SIZEOF(svector);
	size_t const usedSpace = ZR2SS_USPACE_SIZEOF(svector);
	size_t const initialMemorySpace = getInitialMemoryNbObjs(svector) * objSize;

	zrfdecreaseSpace fdecreaseSpace = ZR2SS_STRATEGY(svector)->fdecreaseSpace;
	zrfmustShrink fmustShrink = ZR2SS_STRATEGY(svector)->fmustShrink;

	ZRArray2 new = ZRRESIZE_MAKELESSSIZE(
		totalSpace, usedSpace, initialMemorySpace, objAlignment,
		svector->allocatedMemory, svector->initialArray, svector->initialArraySize, svector->allocator,
		fmustShrink, fdecreaseSpace, svector
		);

	size_t const nextTotalNbObj = new.size / objSize;
	void *lastUSpace = ZR2SS_USPACE(svector);
	setFUEOSpaces(svector, new.array, nextTotalNbObj, ZR2SS_USPACE(svector), nbObj);

	ZRFREE(svector->allocator, lastUSpace);

	if (new.array == svector->initialArray)
		svector->allocatedMemory = NULL;

	ZR2SS_VECTOR(svector)->capacity = nextTotalNbObj;
}

ZRMUSTINLINE
static inline bool mustGrow(ZRVector *vec, size_t nbObjMore)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	size_t const used = ZR2SS_USPACE_SIZEOF(svector);
	size_t const free = ZR2SS_FREESPACE_SIZEOF(svector);
	size_t const objNbBytes = nbObjMore * ZR2SS_VECTOR(svector)->objSize;

	if (free < objNbBytes)
		return true;

	if (ZRVector2SideStrategy_memoryIsAllocated(svector))
		return ZR2SS_STRATEGY(svector)->fmustGrow(ZR2SS_TOTALSPACE_SIZEOF(svector), used + objNbBytes, ZR2SS_VECTOR(svector));

	return false;
}

ZRMUSTINLINE
static inline bool mustShrink(ZRVector *vec)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	size_t const used = ZR2SS_USPACE_SIZEOF(svector);
	size_t const free = ZR2SS_FREESPACE_SIZEOF(svector);

	if (ZRVector2SideStrategy_memoryIsAllocated(svector))
		return ZR2SS_STRATEGY(svector)->fmustShrink(free, used, ZR2SS_VECTOR(svector));

	return false;
}

static bool enoughSpaceForInsert(ZR2SSVector *svector, size_t pos, size_t nbMore)
{
	size_t freeSpace;
	size_t const nbBytesMore = nbMore * ZR2SS_VECTOR(svector)->objSize;
	size_t const middleOffset = ZR2SS_VECTOR(svector)->nbObj / 2;
	bool const toTheRight = pos >= middleOffset;

	if (toTheRight)
		freeSpace = ZR2SS_ESPACE_SIZEOF(svector);
	else
		freeSpace = ZR2SS_FSPACE_SIZEOF(svector);

	return freeSpace >= nbBytesMore;
}

/**
 * Shift some object on the right or left to free space for new objects.
 *
 * This function assume that it exists enough space (according to CenterAdd) to do its work.
 */
ZRMUSTINLINE
static inline void operationAdd_shift(ZRVector *vec, size_t pos, size_t nbMore)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	assert(ZR2SS_FREESPACE_SIZEOF(svector) >= nbMore * ZR2SS_VECTOR(svector)->objSize);

	if (!enoughSpaceForInsert(svector, pos, nbMore))
		centerFUEOSpaces(svector, nbMore);

	size_t const nbObj = ZR2SS_VECTOR(svector)->nbObj;
	size_t const objSize = ZR2SS_VECTOR(svector)->objSize;
	size_t const nbBytesMore = nbMore * objSize;
	size_t const middleOffset = nbObj / 2;

// The insertion will be act on the right part ?
	bool const toTheRight = pos >= middleOffset;

	void *dest;
	void *src;
	size_t nbObjForDepl;

	if (toTheRight)
	{
		nbObjForDepl = nbObj - pos;
		src = ZRVECTOR_GET(ZR2SS_VECTOR(svector), pos);
		dest = (char*)src + nbBytesMore;
		ZR2SS_ESPACE(svector) = (char*)ZR2SS_ESPACE(svector) + nbBytesMore;
	}
	else
	{
		nbObjForDepl = pos;
		src = ZRVECTOR_GET(ZR2SS_VECTOR(svector), 0);
		dest = (char*)src - nbBytesMore;
		ZR2SS_USPACE(svector) = dest;
	}

	if (nbObjForDepl == 0)
		return;

	ZRARRAYOP_DEPLACE(dest, objSize, nbObjForDepl, src);
}

ZRMUSTINLINE
static inline void operationDel_shift(ZRVector *vec, size_t pos, size_t nbLess)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	size_t const nbObj = ZR2SS_VECTOR(svector)->nbObj;
	size_t const objSize = ZR2SS_VECTOR(svector)->objSize;
	size_t const nbBytesLess = nbLess * objSize;
	size_t const middleOffset = nbObj / 2;
	bool const toTheRight = pos >= middleOffset;

	void *dest;
	void *src;
	size_t nbObjForDepl;

	if (toTheRight)
	{
		nbObjForDepl = nbObj - pos - nbLess;
		dest = ZRVECTOR_GET(ZR2SS_VECTOR(svector), pos);
		src = dest + nbBytesLess;
		ZR2SS_ESPACE(svector) = (char*)ZR2SS_ESPACE(svector) - nbBytesLess;
	}
	else
	{
		nbObjForDepl = pos;
		src = ZRVECTOR_GET(ZR2SS_VECTOR(svector), 0);
		dest = src + nbBytesLess;
		ZR2SS_USPACE(svector) = dest;
	}

	if (nbObjForDepl > 0)
		ZRARRAYOP_DEPLACE(dest, objSize, nbObjForDepl, src);
}

// ============================================================================

void ZRVector2SideStrategy_finsert(ZRVector *vec, size_t pos, size_t nb)
{
	operationAdd_shift(vec, pos, nb);
	vec->nbObj += nb;
}

void ZRVector2SideStrategy_finsertGrow(ZRVector *vec, size_t pos, size_t nb)
{
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);

	if (mustGrow(vec, nb))
		moreSize(vec, nb);

	operationAdd_shift(vec, pos, nb);
	vec->nbObj += nb;
}

void ZRVector2SideStrategy_fdelete(ZRVector *vec, size_t pos, size_t nb)
{
	assert(ZR2SS_FSPACE(ZRVECTOR_2SS(vec)) != NULL);
	assert(nb <= vec->nbObj);

	operationDel_shift(vec, pos, nb);
	vec->nbObj -= nb;
}

void ZRVector2SideStrategy_fdeleteShrink(ZRVector *vec, size_t pos, size_t nb)
{
	assert(ZR2SS_FSPACE(ZRVECTOR_2SS(vec)) != NULL);
	assert(nb <= vec->nbObj);

	operationDel_shift(vec, pos, nb);
	vec->nbObj -= nb;

	if (mustShrink(vec))
		lessSize(vec);
}

static
void ZRVector2SideStrategy_fdone(ZRVector *vec)
{
	if (ZRVector2SideStrategy_memoryIsAllocated(ZRVECTOR_2SS(vec)))
		ZRFREE(ZRVECTOR_2SS(vec)->allocator, ZRVECTOR_2SS(vec)->allocatedMemory);

	if (ZRVECTOR_2SS(vec)->staticStrategy == 0)
		ZRFREE(ZRVECTOR_2SS(vec)->allocator, vec->strategy);
}

/**
 * Delete a vector created by one of the creation helper functions
 */
static
void ZRVector2SideStrategy_fdestroy(ZRVector *vec)
{
	ZRAllocator *const allocator = ZRVECTOR_2SS(vec)->allocator;
	ZRVECTOR_DONE(vec);
	ZRFREE(allocator, vec);
}

// ============================================================================
// Help
// ============================================================================

typedef struct
{
	ZRObjAlignInfos vectorInfos[ZRVECTOR_INFOS_NB];
	ZRObjInfos objInfos;
	size_t initialArrayNbObj;
	size_t initialMemoryNbObj;
	ZRAllocator *allocator;
	unsigned fixed :1;
	unsigned staticStrategy :1;
} ZR2SSInitInfos;

ZRObjInfos ZRVector2SideStrategyInfos_objInfos(void)
{
	ZRObjInfos ret = { ZRTYPE_ALIGNMENT_SIZE(ZR2SSInitInfos) };
	return ret;
}

ZRMUSTINLINE
static inline void ZRVector2SideStrategyInfos_validate(ZR2SSInitInfos *initInfos)
{
	getVectorInfos((void*)initInfos, initInfos->initialArrayNbObj, ZROBJINFOS_SIZE_ALIGNMENT(initInfos->objInfos), (bool)initInfos->staticStrategy);
}

void ZRVector2SideStrategyInfos(void *infos_out, size_t initialArrayNbObj, size_t initialMemoryNbObj, size_t objSize, size_t objAlignment, ZRAllocator *allocator, bool fixed)
{
	initialMemoryNbObj = checkInitialNbObj(initialMemoryNbObj);
	initialArrayNbObj = checkInitialNbObj(initialArrayNbObj);

	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	*initInfos = (ZR2SSInitInfos ) { //
		.objInfos = { objAlignment, objSize },
		.initialArrayNbObj = initialArrayNbObj,
		.initialMemoryNbObj = initialMemoryNbObj,
		.allocator = allocator,
		.fixed = (int)fixed,
		};
	ZRVector2SideStrategyInfos_validate(initInfos);
}

void ZRVector2SideStrategyInfos_staticStrategy(void *infos_out)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	initInfos->staticStrategy = 1;
	ZRVector2SideStrategyInfos_validate(initInfos);
}

void ZRVector2SideStrategyInfos_fixed(void *infos_out, size_t initialArraySize, size_t initialMemorySpace, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZRVector2SideStrategyInfos(infos_out, initialArraySize, initialMemorySpace, objSize, objAlignment, allocator, true);
}

void ZRVector2SideStrategyInfos_dynamic(void *infos_out, size_t initialArraySpace, size_t initialMemorySpace, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZRVector2SideStrategyInfos(infos_out, initialArraySpace, initialMemorySpace, objSize, objAlignment, allocator, false);
}

ZRObjInfos ZRVector2SideStrategy_objInfos(void *infos)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos;
	return ZROBJALIGNINFOS_CPYOBJINFOS(initInfos->vectorInfos[ZRVectorInfos_struct]);
}

static void ZRVector2SideStrategy_initStrategy(ZRVector2SideStrategy *strategy)
{
	*strategy = (ZRVector2SideStrategy ) //
		{ //
		.strategy = { //
			.finitVec = ZRVector2SideStrategy_finitVec, //
			.finsert = ZRVector2SideStrategy_finsert, //
			.fdelete = ZRVector2SideStrategy_fdelete, //
			.fchangeObjSize = ZRVector2SideStrategy_fchangeObjSize, //
			.fmemoryTrim = ZRVector2SideStrategy_fmemoryTrim, //
			.fdone = ZRVector2SideStrategy_fdone, //
			.fdestroy = ZRVector2SideStrategy_fdone, //
			},//
		.fmustGrow = mustGrowSimple, //
		.fincreaseSpace = increaseSpaceTwice, //
		.fmustShrink = mustShrink4, //
		.fdecreaseSpace = decreaseSpaceTwice, //
		};
}

void ZRVector2SideStrategy_init(ZRVector *vector, void *infos_p)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_p;
	ZRObjAlignInfos *infos = initInfos->vectorInfos;
	ZR2SSVector *ssvector = ZRVECTOR_2SS(vector);

	*ssvector = (ZR2SSVector )
		{ //
		.allocator = initInfos->allocator,
		.initialArrayOffset = infos[ZRVectorInfos_objs].offset,
		.initialArray = (char*)vector + infos[ZRVectorInfos_objs].offset,
		.initialArraySize = infos[ZRVectorInfos_objs].size,
		.initialMemoryNbObjs = initInfos->initialMemoryNbObj,
		.staticStrategy = initInfos->staticStrategy,
		};

	ZRVector2SideStrategy *strategy;

	if (initInfos->staticStrategy)
		strategy = (ZRVector2SideStrategy*)((char*)vector + infos[ZRVectorInfos_strategy].offset);
	else
		strategy = ZRALLOC(initInfos->allocator, sizeof(*strategy));

	ZRVector2SideStrategy_initStrategy(strategy);

	if (initInfos->fixed)
		/*
		 * ZRVector2SideStrategy_fixedMemory(ZR2SSSTRATEGY_VECTOR(strategy));
		 * Done in the initStrategy()
		 */
		;
	else
		ZRVector2SideStrategy_dynamicMemory(ZR2SSSTRATEGY_VECTOR(strategy));

	ZRVECTOR_INIT(vector, initInfos->objInfos.size, initInfos->objInfos.alignment, ZR2SSSTRATEGY_VECTOR(strategy));
}

ZRVector* ZRVector2SideStrategy_new(void *infos_p)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_p;

	ZR2SSVector *ret = ZRALLOC(initInfos->allocator, initInfos->vectorInfos[ZRVectorInfos_struct].size);
	ZRVector2SideStrategy_init(ZR2SS_VECTOR(ret), infos_p);
	ZR2SSSTRATEGY_VECTOR(ZR2SS_STRATEGY(ret))->fdestroy = ZRVector2SideStrategy_fdestroy;
	return ZR2SS_VECTOR(ret);
}

ZRVector* ZRVector2SideStrategy_createFixed(size_t initialNbObjs, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZR2SSInitInfos initInfos;
	ZRVector2SideStrategyInfos_fixed(&initInfos, initialNbObjs, 0, objSize, objAlignment, allocator);
	return ZRVector2SideStrategy_new(&initInfos);
}

ZRVector* ZRVector2SideStrategy_createDynamic(size_t initialNbObjs, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZR2SSInitInfos initInfos;
	ZRVector2SideStrategyInfos_dynamic(&initInfos, initialNbObjs, initialNbObjs, objSize, objAlignment, allocator);
	return ZRVector2SideStrategy_new(&initInfos);
}

ZRVector* ZRVector2SideStrategy_createFixedM(size_t initialArrayNbObj, size_t initialMemoryNbObj, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZR2SSInitInfos initInfos;
	ZRVector2SideStrategyInfos_fixed(&initInfos, initialArrayNbObj, initialMemoryNbObj, objSize, objAlignment, allocator);
	return ZRVector2SideStrategy_new(&initInfos);
}

ZRVector* ZRVector2SideStrategy_createDynamicM(size_t initialArrayNbObj, size_t initialMemoryNbObj, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZR2SSInitInfos initInfos;
	ZRVector2SideStrategyInfos_dynamic(&initInfos, initialArrayNbObj, initialMemoryNbObj, objSize, objAlignment, allocator);
	return ZRVector2SideStrategy_new(&initInfos);
}
