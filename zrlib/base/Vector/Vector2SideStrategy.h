/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#ifndef VECTOR2SIDESTRATEGY_H
#define VECTOR2SIDESTRATEGY_H

#include <zrlib/config.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Vector/Vector.h>

#include <stdbool.h>

// ============================================================================
// STRATEGY

size_t ZRVector2SideStrategy_sdataSize(ZRVector *vec);
size_t ZRVector2SideStrategy_size();

void ZRVector2SideStrategy_init(ZRVectorStrategy *strategy, ZRAllocator *allocator, size_t initialArraySize, size_t initialMemorySize);

void ZRVector2SideStrategy_fixedMemory(______ ZRVectorStrategy *strategy);
void ZRVector2SideStrategy_dynamicMemory(____ ZRVectorStrategy *strategy);

void ZRVector2SideStrategy_growOnAdd(____ ZRVectorStrategy *strategy, bool v);
void ZRVector2SideStrategy_shrinkOnDelete(ZRVectorStrategy *strategy, bool v);

void ZRVector2SideStrategy_growStrategy( //
	ZRVectorStrategy *strategy, //
	bool _ (*mustGrow)(____ size_t totalSpace, size_t usedSpace, ZRVector *vec), //
	size_t (*increaseSpace)(size_t totalSpace, size_t usedSpace, ZRVector *vec) //
	);

void ZRVector2SideStrategy_shrinkStrategy( //
	ZRVectorStrategy *strategy, //
	bool _ (*mustShrink)(__ size_t totalSpace, size_t usedSpace, ZRVector *vec), //
	size_t (*decreaseSpace)(size_t totalSpace, size_t usedSpace, ZRVector *vec) //
	);

// ============================================================================
// VECTOR

void ZRVector2SideStrategy_memoryTrim(ZRVector *vec);

// ============================================================================
// HELPERS

size_t __ ZRVector2SideStrategy_vectorSize(size_t initialSpace, size_t objSize);
ZRVector* ZRVector2SideStrategy_alloc(_______ size_t initialSpace, size_t objSize, ZRAllocator *allocator);
ZRVector* ZRVector2SideStrategy_createFixed(_ size_t initialSpace, size_t objSize, ZRAllocator *allocator);
ZRVector* ZRVector2SideStrategy_createDynamic(size_t initialSpace, size_t objSize, ZRAllocator *allocator);

ZRVector* ZRVector2SideStrategy_createFixedM(_ size_t initialArraySpace, size_t initialMemorySpace, size_t objSize, ZRAllocator *allocator);
ZRVector* ZRVector2SideStrategy_createDynamicM(size_t initialArraySpace, size_t initialMemorySpace, size_t objSize, ZRAllocator *allocator);

void ZRVector2SideStrategy_destroy(ZRVector *vec);

// ============================================================================
// SPACE STRATEGIES

bool mustGrowSimple(size_t totalSpace, size_t usedSpace, ZRVector *vec);
bool mustGrowTwice(size_t totalSpace, size_t usedSpace, ZRVector *vec);
size_t increaseSpaceTwice(size_t totalSpace, size_t usedSpace, ZRVector *vec);

bool mustShrink4(size_t total, size_t used, ZRVector *vec);
size_t decreaseSpaceTwice(size_t totalSpace, size_t usedSpace, ZRVector *vec);

// ============================================================================

#endif
