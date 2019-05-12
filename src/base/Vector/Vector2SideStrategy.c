/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <assert.h>
#include <stdbool.h>

#define INITIAL_SIZE 64*3

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
};

// ============================================================================

struct ZRVector_2SideDataS
{
	size_t initialNbObjSpace;

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

size_t ZRVector_2SideStrategy_size(void)
{
	return sizeof(ZRVector_2SideStrategy);
}

ZRVector_2SideStrategy ZRVector_2SideStrategy_object(ZRAllocator *allocator)
{
	ZRVector_2SideStrategy ret = { //
		.strategy = { //
			.finitVec = ZRVector_2SideStrategy_initVec, //
			.finitSData = ZRVector_2SideData_init, //
			.fdataSize = ZRVector_2SideData_size, //
			.finsert = ZRVector_2SideStrategy_insert, //
			.fdelete = ZRVector_2SideStrategy_delete, //
			.fclean = ZRVector_2SideStrategy_clean //
			},//
		.allocator = allocator  //
		};
	return ret;
}

void ZRVector_2SideStrategy_init(ZRVectorStrategy *strategy, ZRAllocator *allocator)
{
	*(ZRVector_2SideStrategy*)strategy = ZRVector_2SideStrategy_object(allocator);
}

static inline void firstAlloc(ZRVector *vec);

void ZRVector_2SideStrategy_initVec(ZRVector *vec)
{
	firstAlloc(vec);
}

// ============================================================================

size_t ZRVector_2SideData_size(void)
{
	return sizeof(ZRVector_2SideData);
}

void ZRVector_2SideData_init(char *sdata, ZRVectorStrategy *strategy)
{
	ZRVector_2SideData_initialSpace(sdata, strategy, 0);
}

void ZRVector_2SideData_initialSpace(char *_sdata, ZRVectorStrategy *strategy, size_t initialSpace)
{
	ZRVector_2SideData *sdata = (ZRVector_2SideData*)_sdata;

	if (initialSpace == 0)
		sdata->initialNbObjSpace = INITIAL_SIZE;
	else
		sdata->initialNbObjSpace = initialSpace;
}

// ============================================================================

static size_t getOffsetOfUSpace(size_t free, size_t used)
{
	return (free / 2) - (used / 2);
}

static inline bool _mustGrow(size_t free, size_t used)
{
	return (free / 2) < used;
}

/**
 * Shift the user vector.
 *
 * Do not check the overbound : this is a private function.
 */
static inline void shiftUSpace(ZRVector *vec, size_t shift, bool toTheRight)
{
	void *dest;
	void *src;

	if (toTheRight)
	{
		dest = (char*)ZRVECTOR_USPACE(vec) + (shift * ZRVECTOR_OBJSIZE(vec));
		src = ZRVECTOR_USPACE(vec);
	}
	else
	{
		dest = (char*)ZRVECTOR_USPACE(vec) - (shift * ZRVECTOR_OBJSIZE(vec));
		src = ZRVECTOR_USPACE(vec);
	}
	ZRArrayOp_deplace(dest, ZRVECTOR_OBJSIZE(vec), ZRVECTOR_NBOBJ(vec), src);
	ZRVECTOR_USPACE(vec) = dest;
	ZRVECTOR_ESPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + (ZRVECTOR_NBOBJ(vec) * ZRVECTOR_OBJSIZE(vec));
}

static inline void firstAlloc(ZRVector *vec)
{
	size_t const initialSize = ZRVECTOR_DATA(vec)->initialNbObjSpace;
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);
	size_t const nbBytes = objSize * initialSize;

	ZRVECTOR_FSPACE(vec) = ZRALLOC(ZRVECTOR_STRATEGY(vec)->allocator, nbBytes);
	ZRVECTOR_USPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + ((initialSize / 3) * objSize);
	ZRVECTOR_ESPACE(vec) = (char*)ZRVECTOR_USPACE(vec);
	ZRVECTOR_OSPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + nbBytes;
}

static inline void moreSize(ZRVector *vec, size_t nbObjMore)
{
	size_t const totalSpace = ZRVECTOR_TOTALSPACE_SIZEOF(vec);
	size_t const objNbBytes = nbObjMore * ZRVECTOR_OBJSIZE(vec);
	size_t const nextUsedSpace = ZRVECTOR_USPACE_SIZEOF(vec) + objNbBytes;

	size_t nextFreeSpace = ZRVECTOR_FREESPACE_SIZEOF(vec);
	size_t nextTotalSpace = totalSpace;

// Adjust the nextFreeSpace if necessary to avoid negative number
	while (nextFreeSpace < objNbBytes)
	{
		nextTotalSpace += nextFreeSpace;
		nextFreeSpace *= 2;
	}
	nextFreeSpace -= objNbBytes;

	while (_mustGrow(nextFreeSpace, nextUsedSpace))
	{
		nextTotalSpace *= 2;
		nextFreeSpace = nextTotalSpace - nextUsedSpace;
	}
	size_t const fsize = ZRVECTOR_FSPACE_SIZEOF(vec);
	void *lastFSpace = ZRVECTOR_FSPACE(vec);

	ZRVECTOR_FSPACE(vec) = ZRREALLOC(ZRVECTOR_STRATEGY(vec)->allocator, ZRVECTOR_FSPACE(vec), nextTotalSpace);
	ZRVECTOR_OSPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + nextTotalSpace;

	if (ZRVECTOR_FSPACE(vec) == lastFSpace)
		return;

	ZRVECTOR_USPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + fsize;
	ZRVECTOR_ESPACE(vec) = (char*)ZRVECTOR_USPACE(vec) + (ZRVECTOR_NBOBJ(vec) * ZRVECTOR_OBJSIZE(vec));
}


static inline bool mustGrow(ZRVector *vec, size_t nbObjMore)
{
	size_t const used = ZRVECTOR_USPACE_SIZEOF(vec);
	size_t const free = ZRVECTOR_FREESPACE_SIZEOF(vec);
	size_t const objNbBytes = nbObjMore * ZRVECTOR_OBJSIZE(vec);

	return _mustGrow(free, used + objNbBytes);
}

/**
 * Shift, if necessary, the memory to make enough space for adding new objects.
 *
 * This function assume that it exists enough space to do its work.
 */
static inline void operationAdd_center(ZRVector *vec, size_t nbMore, bool toTheRight)
{
	size_t shift;
	size_t const nbObj = ZRVECTOR_NBOBJ(vec);
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);
	size_t const nbBytesMore = nbMore * objSize;
	size_t const nextUsedSpace = ZRVECTOR_USPACE_SIZEOF(vec) + nbBytesMore;
	size_t const nextFreeSpace = ZRVECTOR_FREESPACE_SIZEOF(vec) - nbBytesMore;
	size_t const offset = getOffsetOfUSpace(nextFreeSpace, nextUsedSpace);

	if (toTheRight)
	{
		size_t const rightSpace = ZRVECTOR_ESPACE_SIZEOF(vec);

		// Enough space
		if (rightSpace >= nbBytesMore)
			return;

		shift = 0;
	}
	else
	{
		size_t const leftSpace = ZRVECTOR_FSPACE_SIZEOF(vec);

		// Enough space
		if (leftSpace >= nbBytesMore)
			return;

		size_t const foffset = leftSpace / objSize;
		shift = offset - foffset;
	}
	shiftUSpace(vec, shift, !toTheRight);
}

/**
 * Shift some object on the right or left to free space for new objects.
 *
 * This function assume that it exists enough space (according to CenterAdd) to do its work.
 */
static inline void operationAdd_shift(ZRVector *vec, size_t pos, size_t nbMore)
{
	size_t const nbObj = ZRVECTOR_NBOBJ(vec);
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);
	size_t const nbBytesMore = nbMore * objSize;
	size_t const middleOffset = nbObj / 2;

// The insertion will be act on the right part ?
	bool const toTheRight = pos >= middleOffset;

	operationAdd_center(vec, nbMore, toTheRight);

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

// ============================================================================

void ZRVector_2SideStrategy_insert(ZRVector *vec, size_t pos, size_t nb)
{
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);

	if (mustGrow(vec, nb))
		moreSize(vec, nb);

	operationAdd_shift(vec, pos, nb);
	vec->nbObj += nb;
}

void ZRVector_2SideStrategy_delete(ZRVector *vec, size_t pos, size_t nb)
{
	assert(ZRVECTOR_FSPACE(vec) != NULL);
}

void ZRVector_2SideStrategy_clean(ZRVector *vec)
{
	ZRFREE(ZRVECTOR_STRATEGY(vec)->allocator, ZRVECTOR_FSPACE(vec));
}
