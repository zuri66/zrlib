/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <assert.h>
#include <stdbool.h>

#define INITIAL_SIZE 64*3

static inline void firstAlloc(ZRVector *vec)
{
	size_t const initialSize = INITIAL_SIZE;
	size_t const objSize = vec->objSize;
	size_t const nbBytes = objSize * initialSize;

	vec->FSpace = vec->allocator->alloc(nbBytes);
	vec->USpace = (char*)vec->FSpace + nbBytes / 3;
	vec->ESpace = (char*)vec->USpace + objSize;
	vec->OSpace = (char*)vec->FSpace + nbBytes;
}

static inline void moreSize(ZRVector *vec)
{
	size_t const nbBytes = ZRVECTOR_TOTALSPACE_SIZEOF(vec) << 1;

	vec->FSpace = vec->allocator->realloc(vec->FSpace, nbBytes);
	vec->OSpace = (char*)vec->FSpace + nbBytes;
}

static bool mustResize(ZRVector *vec)
{
	size_t const used = ZRVECTOR_USPACE_SIZEOF(vec);
	size_t const free = ZRVECTOR_FREESPACE_SIZEOF(vec);
	return free > used;
}

void ZRVector_2SideStrategy_insert(ZRVector *vec, size_t pos)
{
	if (vec->FSpace == NULL)
	{
		firstAlloc(vec);
		return;
	}
	size_t const middleOffset = vec->nbObj >> 1;
	bool const toTheLeft = pos <= middleOffset;
	size_t const shiftBytes = (ZRVECTOR_FREESPACE_SIZEOF(vec) >> 1) - (middleOffset * vec->objSize);

	if (toTheLeft)
	{
		if (vec->FSpace == vec->USpace)
		{
			if (mustResize(vec))
				moreSize(vec);

			ZRMemoryOp_shift(vec->FSpace, vec->OSpace, shiftBytes, true);
			vec->FSpace = (char*)vec->FSpace + shiftBytes;
			vec->USpace = (char*)vec->USpace + shiftBytes;
		}
	}
	else
	{
		if (vec->ESpace == vec->USpace + vec->nbObj)
		{
			if (mustResize(vec))
				moreSize(vec);

			ZRMemoryOp_shift(vec->FSpace, vec->OSpace, shiftBytes, false);
			vec->FSpace = (char*)vec->FSpace - shiftBytes;
			vec->USpace = (char*)vec->USpace - shiftBytes;
		}
	}
}

void ZRVector_2SideStrategy_delete(ZRVector *vec, size_t pos)
{
	assert(vec->FSpace != NULL);
}
