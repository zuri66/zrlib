/**
 * @author zuri
 * @date mer. 02 sept. 2020 15:48:02 CEST
 */

#ifndef ZRRESIZE_OP_H
#define ZRRESIZE_OP_H

#include <zrlib/config.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Array.h>

#include <assert.h>

typedef size_t (*zrflimit)(size_t totalSpace, void *userData);
typedef size_t (*zrfincrease)(size_t totalSpace, void *userData);
typedef size_t (*zrfdecrease)(size_t totalSpace, void *userData);

typedef struct
{
	zrflimit fupLimit;
	zrfincrease fincrease;
} ZRResizeGrowStrategy;

typedef struct
{
	zrflimit fdownLimit;
	zrfincrease fdecrease;
} ZRResizeShrinkStrategy;

typedef struct
{
	ZRResizeGrowStrategy growStrategy;
	ZRResizeShrinkStrategy shrinkStrategy;

	size_t upLimit;
	size_t downLimit;
	size_t initialNb;
} ZRResizeData;

// ============================================================================

static inline size_t ZRRESIZE_MORESIZE(
	size_t totalSpace, size_t usedSpace,
	zrflimit flimit, zrfincrease fincrease,
	void *userData
	)
{
	size_t nextTotalSpace = totalSpace;

	while (nextTotalSpace < usedSpace)
		nextTotalSpace = fincrease(nextTotalSpace, userData);
	while (flimit(nextTotalSpace, userData) <= usedSpace)
		nextTotalSpace = fincrease(nextTotalSpace, userData);

	return nextTotalSpace;
}

static inline size_t ZRRESIZE_LESSSIZE(
	size_t totalSpace, size_t usedSpace,
	zrflimit flimit, zrfdecrease fdecrease,
	void *userData
	)
{
	size_t nextTotalSpace = totalSpace;

	while (flimit(nextTotalSpace, userData) > usedSpace)
		nextTotalSpace = fdecrease(nextTotalSpace, userData);

	return nextTotalSpace;
}

ZRMUSTINLINE
static inline bool ZRRESIZE_MUSTGROW(size_t totalSpace, size_t usedSpace, ZRResizeData *rdata)
{
	if (totalSpace < usedSpace)
		return true;

	return usedSpace >= rdata->upLimit;
}

ZRMUSTINLINE
static inline bool ZRRESIZE_MUSTSHRINK(size_t totalSpace, size_t usedSpace, ZRResizeData *rdata)
{
	return usedSpace < rdata->downLimit;
}

static inline ZRArrayAndNb ZRRESIZE_MAKEMORESIZE(
	size_t totalNb, size_t usedNb, size_t objSize, size_t alignment,
	bool isAllocated, ZRAllocator *allocator,
	ZRResizeData *rdata, void *userData
	)
{
	size_t nextTotalNb;
	nextTotalNb = (isAllocated) ? totalNb : rdata->initialNb;
	nextTotalNb = ZRRESIZE_MORESIZE(nextTotalNb, usedNb, rdata->growStrategy.fupLimit, rdata->growStrategy.fincrease, userData);
	size_t nextTotalSpace = nextTotalNb * objSize;

	rdata->upLimit = rdata->growStrategy.fupLimit(nextTotalNb, rdata);
	rdata->downLimit = rdata->shrinkStrategy.fdownLimit(
		rdata->shrinkStrategy.fdecrease(nextTotalNb, rdata),
		rdata);

	if (!isAllocated)
		return ZRARRAYN_DEF(ZRAALLOC(allocator, alignment, nextTotalSpace), nextTotalNb);
	else
	{
		assert(nextTotalNb > totalNb);
		void *const newMemory = ZRAALLOC(allocator, alignment, nextTotalSpace);
		return ZRARRAYN_DEF(newMemory, nextTotalNb);
	}
}

ZRMUSTINLINE
static inline ZRArrayAndNb ZRRESIZE_MAKELESSSIZE(
	size_t totalNb, size_t usedNb, size_t objSize, size_t alignment,
	void *staticArray, size_t staticArrayNb, ZRAllocator *allocator,
	ZRResizeData *rdata, void *userData
	)
{
	size_t nextTotalNb = ZRRESIZE_LESSSIZE(totalNb, usedNb, rdata->shrinkStrategy.fdownLimit, rdata->shrinkStrategy.fdecrease, userData);

// If we can store it into the initial array
	if (nextTotalNb <= staticArrayNb)
	{
		// Nothing to do
	}
// We can't get a memory space less than the initial memory size
	else if (nextTotalNb < rdata->initialNb)
		nextTotalNb = rdata->initialNb;

	rdata->upLimit = rdata->growStrategy.fupLimit(nextTotalNb, rdata);
	rdata->downLimit = rdata->shrinkStrategy.fdownLimit(
		rdata->shrinkStrategy.fdecrease(nextTotalNb, rdata),
		rdata);

	if (nextTotalNb <= staticArrayNb)
		return ZRARRAYN_DEF(staticArray, staticArrayNb);
	else
	{
		assert(nextTotalNb < totalNb);
		size_t nextTotalSpace = nextTotalNb * objSize;
		void *const newMemory = ZRAALLOC(allocator, alignment, nextTotalSpace);
		return ZRARRAYN_DEF(newMemory, nextTotalNb);
	}
}

// ============================================================================
// SPACE STRATEGIES

size_t ZRResizeOp_limit_25(size_t totalSpace, void *data);
size_t ZRResizeOp_limit_50(size_t totalSpace, void *data);
size_t ZRResizeOp_limit_55(size_t totalSpace, void *data);
size_t ZRResizeOp_limit_60(size_t totalSpace, void *data);
size_t ZRResizeOp_limit_70(size_t totalSpace, void *data);
size_t ZRResizeOp_limit_75(size_t totalSpace, void *data);
size_t ZRResizeOp_limit_80(size_t totalSpace, void *data);
size_t ZRResizeOp_limit_85(size_t totalSpace, void *data);
size_t ZRResizeOp_limit_90(size_t totalSpace, void *data);
size_t ZRResizeOp_limit_95(size_t totalSpace, void *data);
size_t ZRResizeOp_limit_100(size_t totalSpace, void *data);

size_t ZRResizeOp_increase_25(size_t totalSpace, void *vec_p);
size_t ZRResizeOp_increase_50(size_t totalSpace, void *vec_p);
size_t ZRResizeOp_increase_75(size_t totalSpace, void *vec_p);
size_t ZRResizeOp_increase_100(size_t totalSpace, void *vec_p);

#endif
