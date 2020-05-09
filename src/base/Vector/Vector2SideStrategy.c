/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

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

	bool _ (*fmustGrow)(____ size_t totalSpace, size_t usedSpace, ZRVector*);
	size_t (*fincreaseSpace)(size_t totalSpace, size_t usedSpace, ZRVector*);

	bool _ (*fmustShrink)(__ size_t totalSpace, size_t usedSpace, ZRVector*);
	size_t (*fdecreaseSpace)(size_t totalSpace, size_t usedSpace, ZRVector*);
};

// ============================================================================

#define ZRVECTOR_INFOS_NB 3
typedef enum
{
	ZRVectorInfos_base, ZRVectorInfos_objs, ZRVectorInfos_struct
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
};

// ============================================================================

#define ZRDATA_INITIALARRAY(V) ((V)->initialArray)
#define ZRVECTOR_INITIALARRAY(V) ZRDATA_INITIALARRAY(ZRVECTOR_2SS(V))

static void vectorInfos(ZRObjAlignInfos *out, size_t initialSpace, size_t objSize, size_t objAlignment)
{
	out[0] = (ZRObjAlignInfos ) { 0, alignof(ZR2SSVector), sizeof(ZR2SSVector) };
	out[1] = (ZRObjAlignInfos ) { 0, objAlignment, objSize * initialSpace };
	out[2] = (ZRObjAlignInfos ) { };
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

size_t ZRVector2SideStrategy_size()
{
	return sizeof(ZRVector2SideStrategy);
}

/**
 * vec must be initialized to zero before.
 */
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

void ZRVector2SideStrategy_fmemoryTrim(ZRVector *vec)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);

	if (!ZRVector2SideStrategy_memoryIsAllocated(svector))
		return;

	size_t const objSize = vec->objSize;
	size_t const nbObj = vec->nbObj;
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

static inline void moreSize(ZRVector *vec, size_t nbObjMore)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	size_t const objAlignment = vec->objAlignment;
	size_t const objSize = vec->objSize;
	size_t const nbObj = vec->nbObj;

	size_t const newNbObj = nbObj + nbObjMore;
	size_t const totalSpace = ZR2SS_TOTALSPACE_SIZEOF(svector);
	size_t const moreNbBytes = nbObjMore * objSize;
	size_t const nextUsedSpace = ZR2SS_USPACE_SIZEOF(svector) + moreNbBytes;
	bool const isAllocated = ZRVector2SideStrategy_memoryIsAllocated(svector);

	size_t nextTotalSpace;

	if (!isAllocated)
	{
		size_t const initialSize = getInitialMemoryNbObjs(svector);
		nextTotalSpace = initialSize * objSize;
	}
	else
	{
		nextTotalSpace = totalSpace;
	}
	size_t (*const fincreaseSpace)(size_t, size_t, ZRVector*) = ZR2SS_STRATEGY(svector)->fincreaseSpace;
	bool _ (*const fmustGrow)(____ size_t, size_t, ZRVector*) = ZR2SS_STRATEGY(svector)->fmustGrow;

	while (nextTotalSpace < nextUsedSpace)
		nextTotalSpace = fincreaseSpace(nextTotalSpace, nextUsedSpace, ZR2SS_VECTOR(svector));
	while (fmustGrow(nextTotalSpace, nextUsedSpace, ZR2SS_VECTOR(svector)))
		nextTotalSpace = fincreaseSpace(nextTotalSpace, nextUsedSpace, ZR2SS_VECTOR(svector));

	size_t const nextTotalNbObj = nextTotalSpace / objSize;

	if (!isAllocated)
	{
		svector->allocatedMemory = ZRAALLOC(svector->allocator, objAlignment, nextTotalSpace);
		setFUEOSpaces(svector, svector->allocatedMemory, nextTotalNbObj, svector->initialArray, nbObj);
	}
	else
	{
		void *const lastMemory = svector->allocatedMemory;
		void *const lastUSpace = lastMemory + ZR2SS_FSPACE_SIZEOF(svector);

		svector->allocatedMemory = ZRAALLOC(svector->allocator, objAlignment, nextTotalSpace);
		memcpy(svector->allocatedMemory, lastMemory, totalSpace);

		setFUEOSpaces(svector, svector->allocatedMemory, nextTotalNbObj, lastUSpace, nbObj);
		ZRFREE(svector->allocator, lastMemory);
	}
	ZR2SS_VECTOR(svector)->capacity = ZR2SS_TOTALSPACE_SIZEOF(svector);
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
	size_t const initialMemorySize = getInitialMemoryNbObjs(svector);

	size_t nextTotalSpace = totalSpace;

	size_t (*const fdecreaseSpace)(size_t, size_t, ZRVector*) = ZR2SS_STRATEGY(svector)->fdecreaseSpace;
	bool _ (*const fmustShrink)(__ size_t, size_t, ZRVector*) = ZR2SS_STRATEGY(svector)->fmustShrink;

	while (fmustShrink(nextTotalSpace, usedSpace, ZR2SS_VECTOR(svector)))
		nextTotalSpace = fdecreaseSpace(nextTotalSpace, usedSpace, ZR2SS_VECTOR(svector));

// If we can store it into the initial array
	if ((nextTotalSpace / objSize) <= ZRVECTOR_2SS(svector)->initialArraySize)
	{
		// Nothing to do
	}
// We cant get a memory space less than the initial memory size
	else if (nextTotalSpace < initialMemorySize)
	{
		nextTotalSpace = initialMemorySize;
	}
	size_t const nextTotalNbObj = nextTotalSpace / objSize;
	ZR2SSVector *const sdata = ZRVECTOR_2SS(svector);

	if (nextTotalNbObj <= ZRVECTOR_2SS(svector)->initialArraySize)
	{
		ZRARRAYOP_CPY(ZRVECTOR_2SS(svector)->initialArray, objSize, nbObj, sdata->allocatedMemory);
		ZRFREE(ZRVECTOR_2SS(svector)->allocator, sdata->allocatedMemory);
		sdata->allocatedMemory = NULL;
	}
	else
	{
		void *const lastMemory = sdata->allocatedMemory;
		void *const lastUSpace = lastMemory + ZR2SS_FSPACE_SIZEOF(svector);

		sdata->allocatedMemory = ZRAALLOC(svector->allocator, objAlignment, nextTotalSpace);
		setFUEOSpaces(svector, sdata->allocatedMemory, nextTotalNbObj, lastUSpace, nbObj);

		ZRFREE(svector->allocator, lastMemory);
	}
	ZR2SS_VECTOR(svector)->capacity = ZR2SS_TOTALSPACE_SIZEOF(svector);
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
		return ZR2SS_STRATEGY(svector)->fmustGrow(free, used + objNbBytes, ZR2SS_VECTOR(svector));

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

void ZRVector2SideStrategy_fdone(ZRVector *vec)
{
	if (ZRVector2SideStrategy_memoryIsAllocated(ZRVECTOR_2SS(vec)))
		ZRFREE(ZRVECTOR_2SS(vec)->allocator, ZRVECTOR_2SS(vec)->allocatedMemory);
}

#include "Vector2SideStrategy_help.c"
