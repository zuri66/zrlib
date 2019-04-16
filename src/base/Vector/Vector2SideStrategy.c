/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <assert.h>
#include <stdbool.h>

#define INITIAL_SIZE 64*3

typedef struct ZRVector_2SideDataS ZRVector_2SideData;

// ============================================================================

struct ZRVector_2SideDataS
{
	/*
	 * Memory segment, or unused first memory space
	 */
	void * restrict FSpace;

	/*
	 * Effectively used memory space
	 * Replaced by the array field of ZRVector
	 */
	// void * restrict USpace;
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
#define ZRVECTOR_TOTALSPACE_SIZEOF(V)  (ZRVECTOR_OSPACE(V) - ZRVECTOR_FSPACE(V))
#define ZRVECTOR_FREESPACE_SIZEOF(V) (ZRVECTOR_FSPACE_SIZEOF(V) + ZRVECTOR_ESPACE_SIZEOF(V))

// ============================================================================

ZRVector_2SideStrategy ZRVector_2SideStrategy_object(ZRAllocator *allocator)
{
	ZRVector_2SideStrategy ret = { //
		.strategy = { //
			.fdataSize = ZRVector_2SideStrategy_dataSize, //
			.finsert = ZRVector_2SideStrategy_insert, //
			.fdelete = ZRVector_2SideStrategy_delete //
			},//
		.allocator = allocator  //
		};
	return ret;
}

void ZRVector_2SideStrategy_init(ZRVector_2SideStrategy *strategy, ZRAllocator *allocator)
{
	*strategy = ZRVector_2SideStrategy_object(allocator);
}

size_t ZRVector_2SideStrategy_dataSize(void)
{
	return sizeof(ZRVector_2SideData);
}

static inline void firstAlloc(ZRVector *vec)
{
	size_t const initialSize = INITIAL_SIZE;
	size_t const objSize = vec->objSize;
	size_t const nbBytes = objSize * initialSize;

	ZRVECTOR_FSPACE(vec) = ZRALLOC(ZRVECTOR_STRATEGY(vec)->allocator, nbBytes);
	ZRVECTOR_USPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + (nbBytes / 3);
	ZRVECTOR_ESPACE(vec) = (char*)ZRVECTOR_USPACE(vec) + objSize;
	ZRVECTOR_OSPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + nbBytes;
}

static inline void moreSize(ZRVector *vec)
{
	size_t const nbBytes = ZRVECTOR_TOTALSPACE_SIZEOF(vec) << 1;

	ZRVECTOR_FSPACE(vec) = ZRREALLOC(ZRVECTOR_STRATEGY(vec)->allocator, ZRVECTOR_DATA(vec)->FSpace, nbBytes);
	ZRVECTOR_OSPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + nbBytes;
}

static bool mustResize(ZRVector *vec)
{
	size_t const used = ZRVECTOR_USPACE_SIZEOF(vec);
	size_t const free = ZRVECTOR_FREESPACE_SIZEOF(vec);
	return free > used;
}

void ZRVector_2SideStrategy_insert(ZRVector *vec, size_t pos)
{
	if (ZRVECTOR_FSPACE(vec) == NULL)
	{
		firstAlloc(vec);
		return;
	}
	size_t const middleOffset = vec->nbObj >> 1;
	bool const toTheLeft = pos <= middleOffset;
	size_t const shiftBytes = (ZRVECTOR_FREESPACE_SIZEOF(vec) >> 1) - (middleOffset * vec->objSize);

	if (toTheLeft)
	{
		if (ZRVECTOR_FSPACE(vec) == ZRVECTOR_USPACE(vec))
		{
			if (mustResize(vec))
				moreSize(vec);

			ZRMemoryOp_shift(ZRVECTOR_FSPACE(vec), ZRVECTOR_OSPACE(vec), shiftBytes, true);
			ZRVECTOR_FSPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) + shiftBytes;
			ZRVECTOR_USPACE(vec) = (char*)ZRVECTOR_USPACE(vec) + shiftBytes;
		}
	}
	else
	{
		if (ZRVECTOR_ESPACE(vec) == ZRVECTOR_USPACE(vec) + vec->nbObj)
		{
			if (mustResize(vec))
				moreSize(vec);

			ZRMemoryOp_shift(ZRVECTOR_FSPACE(vec), ZRVECTOR_OSPACE(vec), shiftBytes, false);
			ZRVECTOR_FSPACE(vec) = (char*)ZRVECTOR_FSPACE(vec) - shiftBytes;
			ZRVECTOR_USPACE(vec) = (char*)ZRVECTOR_USPACE(vec) - shiftBytes;
		}
	}
}

void ZRVector_2SideStrategy_delete(ZRVector *vec, size_t pos)
{
	assert(ZRVECTOR_FSPACE(vec) != NULL);
}
