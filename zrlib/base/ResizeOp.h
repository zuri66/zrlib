/**
 * @author zuri
 * @date mer. 02 sept. 2020 15:48:02 CEST
 */

#ifndef ZRRESIZE_OP_H
#define ZRRESIZE_OP_H

#include <zrlib/config.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/ArrayOp.h>

#include <assert.h>

typedef bool (*zrfmustGrow)(size_t totalSpace, size_t usedSpace, void *userData);
typedef size_t (*zrfincreaseSpace)(size_t totalSpace, size_t usedSpace, void *userData);

typedef bool (*zrfmustShrink)(size_t totalSpace, size_t usedSpace, void *userData);
typedef size_t (*zrfdecreaseSpace)(size_t totalSpace, size_t usedSpace, void *userData);

static inline size_t ZRRESIZE_MORESIZE(
	size_t totalSpace, size_t usedSpace,
	zrfmustGrow fmustGrow, zrfincreaseSpace fincreaseSpace,
	void *userData
	)
{
	size_t nextTotalSpace = totalSpace;

	while (nextTotalSpace < usedSpace)
		nextTotalSpace = fincreaseSpace(nextTotalSpace, usedSpace, userData);
	while (fmustGrow(nextTotalSpace, usedSpace, userData))
		nextTotalSpace = fincreaseSpace(nextTotalSpace, usedSpace, userData);

	return nextTotalSpace;
}

static inline size_t ZRRESIZE_LESSSIZE(
	size_t totalSpace, size_t usedSpace,
	zrfmustGrow fmustShrink, zrfincreaseSpace fdecreaseSpace,
	void *userData
	)
{
	size_t nextTotalSpace = totalSpace;

	while (fmustShrink(nextTotalSpace, usedSpace, userData))
		nextTotalSpace = fdecreaseSpace(nextTotalSpace, usedSpace, userData);

	return nextTotalSpace;
}

static inline ZRArray2 ZRRESIZE_MAKEMORESIZE(
	size_t totalSpace, size_t usedSpace, size_t initialSpace,
	size_t alignment, void *allocatedMemory, ZRAllocator *allocator,
	zrfmustGrow fmustGrow, zrfincreaseSpace fincreaseSpace, void *userData
	)
{
	bool const isAllocated = allocatedMemory != NULL;
	size_t nextTotalSpace;

	if (!isAllocated)
		nextTotalSpace = initialSpace;
	else
		nextTotalSpace = totalSpace;

	nextTotalSpace = ZRRESIZE_MORESIZE(nextTotalSpace, usedSpace, fmustGrow, fincreaseSpace, userData);

	if (!isAllocated)
	{
		return ZRARRAY2_DEF(ZRAALLOC(allocator, alignment, nextTotalSpace), nextTotalSpace);
	}
	else
	{
		assert(nextTotalSpace > totalSpace);
		void *const newMemory = ZRAALLOC(allocator, alignment, nextTotalSpace);
		return ZRARRAY2_DEF(newMemory, nextTotalSpace);
	}
}

ZRMUSTINLINE
static inline ZRArray2 ZRRESIZE_MAKELESSSIZE(
	size_t totalSpace, size_t usedSpace, size_t initialSpace,
	size_t alignment, void *allocatedMemory, void *staticArray, size_t staticArraySpace, ZRAllocator *allocator,
	zrfmustShrink fmustShrink, zrfdecreaseSpace fdecreaseSpace, void *userData
	)
{
	size_t nextTotalSpace = ZRRESIZE_LESSSIZE(totalSpace, usedSpace, fmustShrink, fdecreaseSpace, userData);

// If we can store it into the initial array
	if (nextTotalSpace <= staticArraySpace)
	{
		// Nothing to do
	}
// We can't get a memory space less than the initial memory size
	else if (nextTotalSpace < initialSpace)
		nextTotalSpace = initialSpace;

	if (nextTotalSpace <= staticArraySpace)
		return ZRARRAY2_DEF(staticArray, staticArraySpace);
	else
	{
		assert(nextTotalSpace < totalSpace);
		void *const newMemory = ZRAALLOC(allocator, alignment, nextTotalSpace);
		return ZRARRAY2_DEF(newMemory, nextTotalSpace);
	}
}

// ============================================================================
// SPACE STRATEGIES

bool ZRResizeOp_mustGrowSimple(size_t totalSpace, size_t usedSpace, void *data);
bool ZRResizeOp_mustGrowTwice(size_t totalSpace, size_t usedSpace, void *data);
size_t ZRResizeOp_increaseSpaceTwice(size_t totalSpace, size_t usedSpace, void *vdata);

bool ZRResizeOp_mustShrink4(size_t total, size_t used, void *vec_p);
size_t ZRResizeOp_decreaseSpaceTwice(size_t totalSpace, size_t usedSpace, void *data);

// ============================================================================

#endif
