/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <assert.h>
#include <stdbool.h>

#define INITIAL_SIZE 512

typedef struct ZRVector_2SideStrategyS ZRVector_2SideStrategy;
typedef struct ZRVector_2SideDataS ZRVector_2SideData;

// ============================================================================

struct ZRVector_2SideStrategyS
{
	ZRVectorStrategy strategy;

	/*
	 * Allocator for the vector's array.
	 */
	ZRAllocator *allocator;

	size_t initialMemorySize;

	size_t initialArraySize;
};

// ============================================================================

struct ZRVector_2SideDataS
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

	unsigned char initialArray[0];
};

// ============================================================================

#define ZRVECTOR_DATA(V) ((ZRVector_2SideData*)((V)->sdata))
#define ZRVECTOR_STRATEGY(V) ((ZRVector_2SideStrategy*)((V)->strategy))

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

void ZRVector_2SideStrategy_finitVec(ZRVector *vec);

void ZRVector_2SideStrategy_finsert(ZRVector *vec, size_t pos, size_t nb);
void ZRVector_2SideStrategy_fdelete(ZRVector *vec, size_t pos, size_t nb);
void ZRVector_2SideStrategy_fclean(ZRVector *vec);

void ZRVector_2SideStrategy_finsertGrow(ZRVector *vec, size_t pos, size_t nb);
void ZRVector_2SideStrategy_fdeleteShrink(ZRVector *vec, size_t pos, size_t nb);

static inline void moreSize(ZRVector *vec, size_t nbObjMore);
static inline void setFUEOSpaces(ZRVector *vec, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj);

// ============================================================================

static inline bool ZRVector_2SideStrategy_memoryIsAllocated(ZRVector *vec)
{
	return ((ZRVector_2SideData*)vec->sdata)->allocatedMemory != NULL ;
}

// ============================================================================

#include "Vector2SideStrategy_help.c"

// ============================================================================

size_t ZRVector_2SideStrategy_sdataSize(ZRVector *vec)
{
	ZRVector_2SideStrategy *twoSideStrategy = ZRVECTOR_STRATEGY(vec);
	return sizeof(ZRVector_2SideData) + (twoSideStrategy->initialArraySize * vec->objSize);
}

size_t ZRVector_2SideStrategy_size()
{
	return sizeof(ZRVector_2SideStrategy);
}

void ZRVector_2SideStrategy_modeInitialMemorySize(ZRVectorStrategy *strategy, size_t initialMemorySize)
{
	ZRVector_2SideStrategy *twoSideStrategy = (ZRVector_2SideStrategy*)strategy;

	if (initialMemorySize < 0)
		twoSideStrategy->initialMemorySize = INITIAL_SIZE;
	else
		twoSideStrategy->initialMemorySize = initialMemorySize;
}

void ZRVector_2SideStrategy_init(ZRVectorStrategy *strategy, ZRAllocator *allocator, size_t initialArraySize)
{
	*(ZRVector_2SideStrategy*)strategy = (ZRVector_2SideStrategy ) //
		{ //
		.strategy = { //
			.finitVec = ZRVector_2SideStrategy_finitVec, //
			.fsdataSize __ = ZRVector_2SideStrategy_sdataSize, //
			.fstrategySize = ZRVector_2SideStrategy_size, //
			.finsert = ZRVector_2SideStrategy_finsert, //
			.fdelete = ZRVector_2SideStrategy_fdelete, //
			.fclean = ZRVector_2SideStrategy_fclean //
			},//
		.allocator = allocator,  //
		.initialArraySize = initialArraySize //
		};
	;
}

void ZRVector_2SideStrategy_finitVec(ZRVector *vec)
{
	ZRVector_2SideStrategy *twoSideStrategy = ZRVECTOR_STRATEGY(vec);
	ZRVector_2SideData *sdata = (ZRVector_2SideData*)vec->sdata;

	if (twoSideStrategy->initialArraySize == 0)
	{
		moreSize(vec, 0);
	}
	else
	{
		vec->array = sdata->initialArray;
		setFUEOSpaces(vec, vec->array, twoSideStrategy->initialArraySize, NULL, 0);
	}
}

void ZRVector_2SideStrategy_fixedMemory(ZRVectorStrategy *strategy)
{
	ZRVector_2SideStrategy_growOnAdd(strategy, false);
	ZRVector_2SideStrategy_shrinkOnDelete(strategy, false);
}

void ZRVector_2SideStrategy_dynamicMemory(ZRVectorStrategy *strategy)
{
	ZRVector_2SideStrategy_growOnAdd(strategy, true);
	ZRVector_2SideStrategy_shrinkOnDelete(strategy, true);
}

void ZRVector_2SideStrategy_growOnAdd(ZRVectorStrategy *strategy, bool v)
{
	ZRVector_2SideStrategy *twoSideStrategy = (ZRVector_2SideStrategy*)strategy;
	twoSideStrategy->strategy.finsert = v ? ZRVector_2SideStrategy_finsertGrow : ZRVector_2SideStrategy_finsert;
}

void ZRVector_2SideStrategy_shrinkOnDelete(ZRVectorStrategy *strategy, bool v)
{
	ZRVector_2SideStrategy *twoSideStrategy = (ZRVector_2SideStrategy*)strategy;
	twoSideStrategy->strategy.fdelete = v ? ZRVector_2SideStrategy_fdeleteShrink : ZRVector_2SideStrategy_fdelete;
}

void ZRVector_2SideStrategy_memoryTrim(ZRVector *vec)
{

}

// ============================================================================

static inline bool _mustGrow(size_t free, size_t used)
{
	return (free / 2) < used;
}

static inline bool _mustShrink(size_t free, size_t used)
{
	return (free / 5) > used;
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
	void *lastUSpace = ZRVECTOR_USPACE(vec);

	size_t const uspaceOffset = (ZRVECTOR_TOTALSPACE_SIZEOF(vec) - (newNbObj * vec->objSize)) / 2;
	ZRVECTOR_USPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + uspaceOffset;

	// We dont add the new elements here, this is a virtual setting
	ZRVECTOR_ESPACE(vec) = (char*)ZRVECTOR_USPACE(vec) + (vec->nbObj * vec->objSize);

	ZRMemoryOp_deplace(ZRVECTOR_USPACE(vec), lastUSpace, vec->nbObj * vec->objSize);
}

static inline void moreSize(ZRVector *vec, size_t nbObjMore)
{
	size_t const totalSpace = ZRVECTOR_TOTALSPACE_SIZEOF(vec);
	size_t const objNbBytes = nbObjMore * ZRVECTOR_OBJSIZE(vec);
	size_t const nextUsedSpace = ZRVECTOR_USPACE_SIZEOF(vec) + objNbBytes;
	bool const isAllocated = ZRVector_2SideStrategy_memoryIsAllocated(vec);

	size_t nextFreeSpace;
	size_t nextTotalSpace;

	if (!isAllocated)
	{
		assert(ZRVECTOR_STRATEGY(vec)->initialMemorySize + ZRVECTOR_STRATEGY(vec)->initialArraySize > 0);

		size_t const initialSize = ZRVECTOR_STRATEGY(vec)->initialMemorySize > 0 //
			? ZRVECTOR_STRATEGY(vec)->initialMemorySize //
			: ZRVECTOR_STRATEGY(vec)->initialArraySize //
			;
		nextTotalSpace = initialSize * vec->objSize;
		assert(nextTotalSpace >= (vec->nbObj * vec->objSize));
		nextFreeSpace = nextTotalSpace - (vec->nbObj * vec->objSize);

		// Take into account the space of the new objects
		if (nextFreeSpace < objNbBytes)
			nextTotalSpace += objNbBytes;
		else
			nextFreeSpace -= objNbBytes;
	}
	else
	{
		nextTotalSpace = totalSpace + objNbBytes;
		nextFreeSpace = ZRVECTOR_FREESPACE_SIZEOF(vec);
	}

	while (_mustGrow(nextFreeSpace, nextUsedSpace))
	{
		nextFreeSpace += nextTotalSpace;
		nextTotalSpace *= 2;
	}

	if (!isAllocated)
	{
		ZRVector_2SideData *sdata = (ZRVector_2SideData*)vec->sdata;
		sdata->allocatedMemory = ZRALLOC(ZRVECTOR_STRATEGY(vec)->allocator, nextTotalSpace);
		setFUEOSpaces(vec, sdata->allocatedMemory, nextTotalSpace / vec->objSize, sdata->initialArray, vec->nbObj);
	}
	else
	{
		void *lastFSpace = ZRVECTOR_FSPACE(vec);
		size_t offsetLastUSpace = ZRVECTOR_FSPACE_SIZEOF(vec);
		ZRVECTOR_FSPACE(vec) = ZRREALLOC(ZRVECTOR_STRATEGY(vec)->allocator, ZRVECTOR_FSPACE(vec), nextTotalSpace);
		setFUEOSpaces(vec, ZRVECTOR_FSPACE(vec), nextTotalSpace / vec->objSize, (char*)ZRVECTOR_FSPACE(vec) + offsetLastUSpace, vec->nbObj);
	}
}

static inline void lessSize(ZRVector *vec, size_t nbObjLess)
{

}

static inline bool mustGrow(ZRVector *vec, size_t nbObjMore)
{
	size_t const used = ZRVECTOR_USPACE_SIZEOF(vec);
	size_t const free = ZRVECTOR_FREESPACE_SIZEOF(vec);
	size_t const objNbBytes = nbObjMore * ZRVECTOR_OBJSIZE(vec);

	if (ZRVector_2SideStrategy_memoryIsAllocated(vec))
		return _mustGrow(free, used + objNbBytes);

	return free == 0;
}

static inline bool mustShrink(ZRVector *vec, size_t nbObjLess)
{
	size_t const used = ZRVECTOR_USPACE_SIZEOF(vec);
	size_t const free = ZRVECTOR_FREESPACE_SIZEOF(vec);
	size_t const objNbBytes = nbObjLess * ZRVECTOR_OBJSIZE(vec);

	if (ZRVector_2SideStrategy_memoryIsAllocated(vec))
		return _mustShrink(free, used - objNbBytes);

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

}

// ============================================================================

void ZRVector_2SideStrategy_finsert(ZRVector *vec, size_t pos, size_t nb)
{
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);
	operationAdd_shift(vec, pos, nb);
	vec->nbObj += nb;
}

void ZRVector_2SideStrategy_finsertGrow(ZRVector *vec, size_t pos, size_t nb)
{
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);

	if (mustGrow(vec, nb))
		moreSize(vec, nb);

	operationAdd_shift(vec, pos, nb);
	vec->nbObj += nb;
}

void ZRVector_2SideStrategy_fdelete(ZRVector *vec, size_t pos, size_t nb)
{
	assert(ZRVECTOR_FSPACE(vec) != NULL);
	operationDel_shift(vec, pos, nb);
	vec->nbObj -= nb;
}

void ZRVector_2SideStrategy_fdeleteShrink(ZRVector *vec, size_t pos, size_t nb)
{
	assert(ZRVECTOR_FSPACE(vec) != NULL);

	if (mustShrink(vec, nb))
		lessSize(vec, nb);

	operationDel_shift(vec, pos, nb);
	vec->nbObj -= nb;
}

void ZRVector_2SideStrategy_fclean(ZRVector *vec)
{
	if (ZRVector_2SideStrategy_memoryIsAllocated(vec))
		ZRFREE(ZRVECTOR_STRATEGY(vec)->allocator, ZRVECTOR_DATA(vec)->allocatedMemory);
}
