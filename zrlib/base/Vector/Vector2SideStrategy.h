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

size_t ZRVector_2SideStrategy_sdataSize(ZRVector *vec);
size_t ZRVector_2SideStrategy_size();


void ZRVector_2SideStrategy_init(ZRVectorStrategy *strategy, ZRAllocator *allocator, size_t initialArraySize);

void ZRVector_2SideStrategy_modeInitialMemorySize(ZRVectorStrategy *strategy, size_t initialMemorySize);

void ZRVector_2SideStrategy_fixedMemory(______ ZRVectorStrategy *strategy);
void ZRVector_2SideStrategy_dynamicMemory(____ ZRVectorStrategy *strategy);

void ZRVector_2SideStrategy_growOnAdd(____ ZRVectorStrategy *strategy, bool v);
void ZRVector_2SideStrategy_shrinkOnDelete(ZRVectorStrategy *strategy, bool v);

void ZRVector_2SideStrategy_memoryTrim(ZRVector *vec);

void ZRVector_2SideData_init(char *sdata, ZRVectorStrategy *strategy);

// ============================================================================
// HELPERS

ZRVector* ZRVector2SideStrategy_alloc(_______ size_t initialSpace, size_t objSize, ZRAllocator *allocator);
ZRVector* ZRVector2SideStrategy_createFixed(_ size_t initialSpace, size_t objSize, ZRAllocator *allocator);
ZRVector* ZRVector2SideStrategy_createDynamic(size_t initialSpace, size_t objSize, ZRAllocator *allocator);

ZRVector* ZRVector2SideStrategy_createFixedM(_ size_t initialArraySpace, size_t initialMemorySpace, size_t objSize, ZRAllocator *allocator);
ZRVector* ZRVector2SideStrategy_createDynamicM(size_t initialArraySpace, size_t initialMemorySpace, size_t objSize, ZRAllocator *allocator);

// ============================================================================

#endif
