/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#ifndef VECTOR2SIDESTRATEGY_H
#define VECTOR2SIDESTRATEGY_H

#include <zrlib/config.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Vector/Vector.h>

// ============================================================================

typedef struct ZRVector_2SideStrategyS ZRVector_2SideStrategy;

// ============================================================================

struct ZRVector_2SideStrategyS{
	ZRVectorStrategy strategy;

	/*
	 * Allocator for the vector's array.
	 */
	ZRAllocator *allocator;
};

// ============================================================================

ZRVector_2SideStrategy ZRVector_2SideStrategy_object(ZRAllocator *allocator);
void ZRVector_2SideStrategy_init(ZRVector_2SideStrategy *strategy, ZRAllocator *allocator);

size_t ZRVector_2SideStrategy_dataSize(void);
void ZRVector_2SideStrategy_insert(ZRVector *vec, size_t pos);
void ZRVector_2SideStrategy_delete(ZRVector *vec, size_t pos);

#endif
