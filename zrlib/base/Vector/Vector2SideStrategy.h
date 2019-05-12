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

size_t ZRVector_2SideStrategy_size(void);
size_t ZRVector_2SideData_size(void);

void ZRVector_2SideStrategy_initVec(ZRVector *vec);

void ZRVector_2SideStrategy_init(_______ ZRVectorStrategy *strategy, ZRAllocator *allocator);
void ZRVector_2SideStrategy_initialSpace(ZRVectorStrategy *strategy, size_t initialSpace);

void ZRVector_2SideData_init(_______ char *sdata, ZRVectorStrategy *strategy);
void ZRVector_2SideData_initialSpace(char *sdata, ZRVectorStrategy *strategy, size_t initialSpace);

void ZRVector_2SideStrategy_insert(ZRVector *vec, size_t pos, size_t nb);
void ZRVector_2SideStrategy_delete(ZRVector *vec, size_t pos, size_t nb);
void ZRVector_2SideStrategy_clean(ZRVector *vec);

#endif
