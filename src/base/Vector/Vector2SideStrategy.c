/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <assert.h>
#include <stdbool.h>

#define INITIAL_SIZE 512

typedef struct ZRVector2SideStrategyS ZRVector2SideStrategy;
typedef struct ZRVector2SideDataS ZRVector2SideData;

// ============================================================================

struct ZRVector2SideStrategyS
{
	ZRVectorStrategy strategy;

	/*
	 * Allocator for the vector's array.
	 */
	ZRAllocator *allocator;

	bool _ (*fmustGrow)(____ size_t totalSpace, size_t usedSpace, ZRVector*);
	size_t (*fincreaseSpace)(size_t totalSpace, size_t usedSpace, ZRVector*);

	bool _ (*fmustShrink)(__ size_t totalSpace, size_t usedSpace, ZRVector *vec);
	size_t (*fdecreaseSpace)(size_t totalSpace, size_t usedSpace, ZRVector *vec);

	size_t initialMemorySize;

	size_t initialArraySize;
};

// ============================================================================

struct ZRVector2SideDataS
{
	/*
	 * Memory segment, or unused first memory space
	 */
	void * restrict FSpace;

	/*
	 * Effectively used memory space
	 * Replaced by the 'array' field of ZRVector
	 */
	// void * restrict USpace;
	//
	/*
	 * Unused end memory space
	 */
	void * restrict ESpace;

	/*
	 * First address of outside memory segment
	 */
	void * restrict OSpace;

	unsigned char *allocatedMemory;

	unsigned char initialArray[];
};

// ============================================================================

#define ZRVECTOR_DATA(V) ((ZRVector2SideData*)((V)->sdata))
#define ZRVECTOR_STRATEGY(V) ((ZRVector2SideStrategy*)((V)->strategy))

#define ZRVECTOR_FSPACE(V) ZRVECTOR_DATA(V)->FSpace
#define ZRVECTOR_USPACE(V) (V)->array
#define ZRVECTOR_ESPACE(V) ZRVECTOR_DATA(V)->ESpace
#define ZRVECTOR_OSPACE(V) ZRVECTOR_DATA(V)->OSpace

#define ZRVECTOR_FSPACE_SIZEOF(V) (ZRVECTOR_USPACE(V) - ZRVECTOR_FSPACE(V))
#define ZRVECTOR_USPACE_SIZEOF(V) (ZRVECTOR_ESPACE(V) - ZRVECTOR_USPACE(V))
#define ZRVECTOR_ESPACE_SIZEOF(V) (ZRVECTOR_OSPACE(V) - ZRVECTOR_ESPACE(V))
#define ZRVECTOR_TOTALSPACE_SIZEOF(V) (ZRVECTOR_OSPACE(V) - ZRVECTOR_FSPACE(V))
#define ZRVECTOR_FREESPACE_SIZEOF(V) (ZRVECTOR_FSPACE_SIZEOF(V) + ZRVECTOR_ESPACE_SIZEOF(V))

// ============================================================================

void ZRVector2SideStrategy_finitVec(ZRVector *vec);

void ZRVector2SideStrategy_finsert(ZRVector *vec, size_t pos, size_t nb);
void ZRVector2SideStrategy_fdelete(ZRVector *vec, size_t pos, size_t nb);
void ZRVector2SideStrategy_fdone(ZRVector *vec);

void ZRVector2SideStrategy_finsertGrow(ZRVector *vec, size_t pos, size_t nb);
void ZRVector2SideStrategy_fdeleteShrink(ZRVector *vec, size_t pos, size_t nb);

static inline void moreSize(ZRVector *vec, size_t nbObjMore);
static inline void setFUEOSpaces(ZRVector *vec, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj);

// ============================================================================

static inline bool ZRVector2SideStrategy_memoryIsAllocated(ZRVector *vec)
{
	return ((ZRVector2SideData*)vec->sdata)->allocatedMemory != NULL ;
}

// ============================================================================

#include "Vector2SideStrategy_help.c"
#include "Vector2SideStrategy_spaceStrategies.c"

// ============================================================================

size_t ZRVector2SideStrategy_sdataSize(ZRVector *vec)
{
	ZRVector2SideStrategy *twoSideStrategy = ZRVECTOR_STRATEGY(vec);
	return sizeof(ZRVector2SideData) + (twoSideStrategy->initialArraySize * vec->objSize);
}

size_t ZRVector2SideStrategy_size()
{
	return sizeof(ZRVector2SideStrategy);
}

void ZRVector2SideStrategy_init(ZRVectorStrategy *strategy, ZRAllocator *allocator, size_t initialArraySize, size_t initialMemorySize)
{
	*(ZRVector2SideStrategy*)strategy = (ZRVector2SideStrategy ) //
		{ //
		.strategy = { //
			.finitVec = ZRVector2SideStrategy_finitVec, //
			.fsdataSize __ = ZRVector2SideStrategy_sdataSize, //
			.fstrategySize = ZRVector2SideStrategy_size, //
			.finsert = ZRVector2SideStrategy_finsert, //
			.fdelete = ZRVector2SideStrategy_fdelete, //
			.fdone = ZRVector2SideStrategy_fdone //
			},//
		.allocator = allocator,  //
		.initialArraySize = initialArraySize, //
		.initialMemorySize = initialMemorySize, //
		.fmustGrow = mustGrowSimple, //
		.fincreaseSpace = increaseSpaceTwice, //
		.fmustShrink = mustShrink4, //
		.fdecreaseSpace = decreaseSpaceTwice, //
		0 };
	;
}

void ZRVector2SideStrategy_finitVec(ZRVector *vec)
{
	ZRVector2SideStrategy *twoSideStrategy = ZRVECTOR_STRATEGY(vec);
	ZRVector2SideData *sdata = (ZRVector2SideData*)vec->sdata;
	*sdata = (ZRVector2SideData ) { 0 };

	if (twoSideStrategy->initialArraySize == 0)
	{
		moreSize(vec, 0);
	}
	else
	{
		vec->array = sdata->initialArray;
		setFUEOSpaces(vec, vec->array, twoSideStrategy->initialArraySize, NULL, 0);
		vec->capacity = twoSideStrategy->initialArraySize;
	}
}

void ZRVector2SideStrategy_fixedMemory(ZRVectorStrategy *strategy)
{
	ZRVector2SideStrategy_growOnAdd(strategy, false);
	ZRVector2SideStrategy_shrinkOnDelete(strategy, false);
}

void ZRVector2SideStrategy_dynamicMemory(ZRVectorStrategy *strategy)
{
	ZRVector2SideStrategy_growOnAdd(strategy, true);
	ZRVector2SideStrategy_shrinkOnDelete(strategy, true);
}

void ZRVector2SideStrategy_growOnAdd(ZRVectorStrategy *strategy, bool v)
{
	ZRVector2SideStrategy *twoSideStrategy = (ZRVector2SideStrategy*)strategy;
	twoSideStrategy->strategy.finsert = v ? ZRVector2SideStrategy_finsertGrow : ZRVector2SideStrategy_finsert;
}

void ZRVector2SideStrategy_shrinkOnDelete(ZRVectorStrategy *strategy, bool v)
{
	ZRVector2SideStrategy *twoSideStrategy = (ZRVector2SideStrategy*)strategy;
	twoSideStrategy->strategy.fdelete = v ? ZRVector2SideStrategy_fdeleteShrink : ZRVector2SideStrategy_fdelete;
}

void ZRVector2SideStrategy_growStrategy( //
	ZRVectorStrategy *strategy, //
	bool _ (*mustGrow)(____ size_t totalSpace, size_t usedSpace, ZRVector*), //
	size_t (*increaseSpace)(size_t totalSpace, size_t usedSpace, ZRVector*) //
	)
{
	((ZRVector2SideStrategy*)strategy)->fmustGrow = mustGrow;
	((ZRVector2SideStrategy*)strategy)->fincreaseSpace = increaseSpace;
}

void ZRVector2SideStrategy_shrinkStrategy( //
	ZRVectorStrategy *strategy, //
	bool _ (*mustShrink)(__ size_t totalSpace, size_t usedSpace, ZRVector *vec), //
	size_t (*decreaseSpace)(size_t totalSpace, size_t usedSpace, ZRVector *vec) //
	)
{
	((ZRVector2SideStrategy*)strategy)->fmustShrink = mustShrink;
	((ZRVector2SideStrategy*)strategy)->fdecreaseSpace = decreaseSpace;
}

void ZRVector2SideStrategy_memoryTrim(ZRVector *vec)
{

}

// ============================================================================

static inline size_t getInitialMemorySize(ZRVector *vec)
{
	if (ZRVECTOR_STRATEGY(vec)->initialMemorySize > 0)
		return ZRVECTOR_STRATEGY(vec)->initialMemorySize;

	if (ZRVECTOR_STRATEGY(vec)->initialArraySize > 0)
		return ZRVECTOR_STRATEGY(vec)->initialArraySize;

	return INITIAL_SIZE;
}

static inline void setFUEOSpaces(ZRVector *vec, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj)
{
	assert(fspaceNbObj >= sourceNbObj);
	assert(fspaceNbObj >= 2);

	ZRVECTOR_FSPACE(vec) = fspace;
	ZRVECTOR_OSPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + (fspaceNbObj * vec->objSize);

	if (sourceNbObj == 0)
	{
		ZRVECTOR_USPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + ((fspaceNbObj * vec->objSize) / 2);
		ZRVECTOR_ESPACE(vec) = (char*)ZRVECTOR_USPACE(vec);
	}
	else
	{
		ZRVECTOR_USPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + ((fspaceNbObj - sourceNbObj) * vec->objSize) / 2;
		ZRVECTOR_ESPACE(vec) = (char*)ZRVECTOR_USPACE(vec) + (sourceNbObj * vec->objSize);

		if (sourceNbObj > 0)
			ZRMemoryOp_deplace(ZRVECTOR_USPACE(vec), source, sourceNbObj * vec->objSize);
	}
}

static inline void centerFUEOSpaces(ZRVector *vec, size_t nbMore)
{
	assert(nbMore > 0);
	size_t const newNbObj = vec->nbObj + nbMore;
	void * const lastUSpace = ZRVECTOR_USPACE(vec);

	size_t const uspaceOffset = (ZRVECTOR_TOTALSPACE_SIZEOF(vec) - (newNbObj * vec->objSize)) / 2;
	ZRVECTOR_USPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + uspaceOffset;

	// We dont add the new elements here, this is a virtual setting
	ZRVECTOR_ESPACE(vec) = (char*)ZRVECTOR_USPACE(vec) + (vec->nbObj * vec->objSize);

	ZRMemoryOp_deplace(ZRVECTOR_USPACE(vec), lastUSpace, vec->nbObj * vec->objSize);
}

static inline void moreSize(ZRVector *vec, size_t nbObjMore)
{
	size_t const totalSpace = ZRVECTOR_TOTALSPACE_SIZEOF(vec);
	size_t const moreNbBytes = nbObjMore * ZRVECTOR_OBJSIZE(vec);
	size_t const nextUsedSpace = ZRVECTOR_USPACE_SIZEOF(vec) + moreNbBytes;
	bool const isAllocated = ZRVector2SideStrategy_memoryIsAllocated(vec);

	size_t nextTotalSpace;

	if (!isAllocated)
	{
		size_t const initialSize = getInitialMemorySize(vec);
		nextTotalSpace = initialSize * vec->objSize;
	}
	else
	{
		nextTotalSpace = totalSpace;
	}
	size_t (* const fincreaseSpace)(size_t, size_t, ZRVector*) = ZRVECTOR_STRATEGY(vec)->fincreaseSpace;
	bool _ (* const fmustGrow)(____ size_t, size_t, ZRVector*) = ZRVECTOR_STRATEGY(vec)->fmustGrow;

	while (nextTotalSpace < nextUsedSpace)
		nextTotalSpace = fincreaseSpace(nextTotalSpace, nextUsedSpace, vec);
	while (fmustGrow(nextTotalSpace, nextUsedSpace, vec))
		nextTotalSpace = fincreaseSpace(nextTotalSpace, nextUsedSpace, vec);

	size_t const nextTotalNbObj = nextTotalSpace / vec->objSize;
	ZRVector2SideData * const sdata = ZRVECTOR_DATA(vec);

	if (!isAllocated)
	{
		sdata->allocatedMemory = ZRALLOC(ZRVECTOR_STRATEGY(vec)->allocator, nextTotalSpace);
		setFUEOSpaces(vec, sdata->allocatedMemory, nextTotalNbObj, sdata->initialArray, vec->nbObj);
	}
	else
	{
		void * const lastFSpace = ZRVECTOR_FSPACE(vec);
		size_t const offsetLastUSpace = ZRVECTOR_FSPACE_SIZEOF(vec);
		sdata->allocatedMemory = ZRREALLOC(ZRVECTOR_STRATEGY(vec)->allocator, sdata->allocatedMemory, nextTotalSpace);

		setFUEOSpaces(vec, sdata->allocatedMemory, nextTotalNbObj, (char*)sdata->allocatedMemory + offsetLastUSpace, vec->nbObj);
	}
	vec->capacity = ZRVECTOR_TOTALSPACE_SIZEOF(vec);
}

static inline void lessSize(ZRVector *vec)
{
	assert(ZRVector2SideStrategy_memoryIsAllocated(vec));
	size_t const totalSpace = ZRVECTOR_TOTALSPACE_SIZEOF(vec);
	size_t const usedSpace = ZRVECTOR_USPACE_SIZEOF(vec);
	size_t const initialMemorySize = getInitialMemorySize(vec);

	size_t nextTotalSpace = totalSpace;

	size_t (* const fdecreaseSpace)(size_t, size_t, ZRVector*) = ZRVECTOR_STRATEGY(vec)->fdecreaseSpace;
	bool _ (* const fmustShrink)(__ size_t, size_t, ZRVector*) = ZRVECTOR_STRATEGY(vec)->fmustShrink;

	while (fmustShrink(nextTotalSpace, usedSpace, vec))
		nextTotalSpace = fdecreaseSpace(nextTotalSpace, usedSpace, vec);

	// If we can store it into the initial array
	if ((nextTotalSpace / vec->objSize) <= ZRVECTOR_STRATEGY(vec)->initialArraySize)
	{
		// Nothing to do
	}
	// We cant get a memory space less than the initial memory size
	else if (nextTotalSpace < initialMemorySize)
	{
		nextTotalSpace = initialMemorySize;
	}
	size_t const nextTotalNbObj = nextTotalSpace / vec->objSize;
	ZRVector2SideData * const sdata = ZRVECTOR_DATA(vec);

	if (nextTotalNbObj <= ZRVECTOR_STRATEGY(vec)->initialArraySize)
	{
		ZRARRAYOP_CPY(sdata->initialArray, vec->objSize, vec->nbObj, sdata->allocatedMemory);
		ZRFREE(ZRVECTOR_STRATEGY(vec)->allocator, sdata->allocatedMemory);
		sdata->allocatedMemory = NULL;
	}
	else
	{
		setFUEOSpaces(vec, ZRVECTOR_FSPACE(vec), nextTotalNbObj, (char*)ZRVECTOR_USPACE(vec), vec->nbObj);
		sdata->allocatedMemory = ZRREALLOC(ZRVECTOR_STRATEGY(vec)->allocator, sdata->allocatedMemory, nextTotalSpace);

		if (sdata->allocatedMemory != ZRVECTOR_FSPACE(vec))
			setFUEOSpaces(vec, sdata->allocatedMemory, nextTotalNbObj, sdata->allocatedMemory + ZRVECTOR_FSPACE_SIZEOF(vec), vec->nbObj);
	}
	vec->capacity = ZRVECTOR_TOTALSPACE_SIZEOF(vec);
}

static inline bool mustGrow(ZRVector *vec, size_t nbObjMore)
{
	size_t const used = ZRVECTOR_USPACE_SIZEOF(vec);
	size_t const free = ZRVECTOR_FREESPACE_SIZEOF(vec);
	size_t const objNbBytes = nbObjMore * ZRVECTOR_OBJSIZE(vec);

	if (free < objNbBytes)
		return true;

	if (ZRVector2SideStrategy_memoryIsAllocated(vec))
		return ZRVECTOR_STRATEGY(vec)->fmustGrow(free, used + objNbBytes, vec);

	return false;
}

static inline bool mustShrink(ZRVector *vec)
{
	size_t const used = ZRVECTOR_USPACE_SIZEOF(vec);
	size_t const free = ZRVECTOR_FREESPACE_SIZEOF(vec);

	if (ZRVector2SideStrategy_memoryIsAllocated(vec))
		return ZRVECTOR_STRATEGY(vec)->fmustShrink(free, used, vec);

	return false;
}

static bool enoughSpaceForInsert(ZRVector *vec, size_t pos, size_t nbMore)
{
	size_t freeSpace;
	size_t const nbBytesMore = nbMore * vec->objSize;
	size_t const middleOffset = vec->nbObj / 2;
	bool const toTheRight = pos >= middleOffset;

	if (toTheRight)
		freeSpace = ZRVECTOR_ESPACE_SIZEOF(vec);
	else
		freeSpace = ZRVECTOR_FSPACE_SIZEOF(vec);

	return freeSpace >= nbBytesMore;
}

/**
 * Shift some object on the right or left to free space for new objects.
 *
 * This function assume that it exists enough space (according to CenterAdd) to do its work.
 */
static inline void operationAdd_shift(ZRVector *vec, size_t pos, size_t nbMore)
{
	assert(ZRVECTOR_FREESPACE_SIZEOF(vec) >= nbMore * vec->objSize);

	if (!enoughSpaceForInsert(vec, pos, nbMore))
		centerFUEOSpaces(vec, nbMore);

	size_t const nbObj = ZRVECTOR_NBOBJ(vec);
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);
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
		src = ZRVECTOR_GET(vec, pos);
		dest = (char*)src + nbBytesMore;
		ZRVECTOR_ESPACE(vec) = (char*)ZRVECTOR_ESPACE(vec) + nbBytesMore;
	}
	else
	{
		nbObjForDepl = pos;
		src = ZRVECTOR_GET(vec, 0);
		dest = (char*)src - nbBytesMore;
		ZRVECTOR_USPACE(vec) = dest;
	}

	if (nbObjForDepl == 0)
		return;

	ZRARRAYOP_DEPLACE(dest, objSize, nbObjForDepl, src);
}

static inline void operationDel_shift(ZRVector *vec, size_t pos, size_t nbLess)
{
	size_t const nbObj = ZRVECTOR_NBOBJ(vec);
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);
	size_t const nbBytesLess = nbLess * objSize;
	size_t const middleOffset = nbObj / 2;
	bool const toTheRight = pos >= middleOffset;

	void *dest;
	void *src;
	size_t nbObjForDepl;

	if (toTheRight)
	{
		nbObjForDepl = nbObj - pos - nbLess;
		dest = ZRVECTOR_GET(vec, pos);
		src = dest + nbBytesLess;
		ZRVECTOR_ESPACE(vec) = (char*)ZRVECTOR_ESPACE(vec) - nbBytesLess;
	}
	else
	{
		nbObjForDepl = pos;
		src = ZRVECTOR_GET(vec, 0);
		dest = src + nbBytesLess;
		ZRVECTOR_USPACE(vec) = dest;
	}

	if (nbObjForDepl > 0)
		ZRARRAYOP_DEPLACE(dest, objSize, nbObjForDepl, src);
}

// ============================================================================

void ZRVector2SideStrategy_finsert(ZRVector *vec, size_t pos, size_t nb)
{
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);
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
	assert(ZRVECTOR_FSPACE(vec) != NULL);
	assert(nb <= vec->nbObj);

	operationDel_shift(vec, pos, nb);
	vec->nbObj -= nb;
}

void ZRVector2SideStrategy_fdeleteShrink(ZRVector *vec, size_t pos, size_t nb)
{
	assert(ZRVECTOR_FSPACE(vec) != NULL);
	assert(nb <= vec->nbObj);

	operationDel_shift(vec, pos, nb);
	vec->nbObj -= nb;

	if (mustShrink(vec))
		lessSize(vec);
}

void ZRVector2SideStrategy_fdone(ZRVector *vec)
{
	if (ZRVector2SideStrategy_memoryIsAllocated(vec))
		ZRFREE(ZRVECTOR_STRATEGY(vec)->allocator, ZRVECTOR_DATA(vec)->allocatedMemory);
}
