/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#ifndef ZRVECTOR2SIDESTRATEGY_H
#define ZRVECTOR2SIDESTRATEGY_H

#include <zrlib/base/ResizeOp.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/struct.h>
#include <zrlib/base/Vector/Vector.h>

#include <stdbool.h>

// ============================================================================
// STRATEGY

void ZRVector2SideStrategy_growStrategy(_ ZRVector *vec, zrfmustGrow fmustGrow, ___ zrfincreaseSpace fincreaseSpace);
void ZRVector2SideStrategy_shrinkStrategy(ZRVector *vec, zrfmustShrink fmustShrink, zrfdecreaseSpace fdecreaseSpace);

// ============================================================================
// HELPERS

/**
 * The ObjInfos of the information object for initialization of the vector.
 */
ZRObjInfos ZRVector2SideStrategyInfos_objInfos(void);

void ZRVector2SideStrategyInfos(_______ void *infos_out, size_t initialArrayNbObj, size_t initialMemoryNbObj, size_t objSize, size_t objAlignment, ZRAllocator *allocator, bool fixed);
void ZRVector2SideStrategyInfos_fixed(_ void *infos_out, size_t initialArrayNbObj, size_t initialMemoryNbObj, size_t objSize, size_t objAlignment, ZRAllocator *allocator);
void ZRVector2SideStrategyInfos_dynamic(void *infos_out, size_t initialArrayNbObj, size_t initialMemoryNbObj, size_t objSize, size_t objAlignment, ZRAllocator *allocator);

/**
 * The strategy object is store inside the vector object
 */
void ZRVector2SideStrategyInfos_staticStrategy(void *infos_out);

/**
 * The ObjInfos of the vector object.
 */
ZRObjInfos ZRVector2SideStrategy_objInfos(void *infos);

void ZRVector2SideStrategy_init(ZRVector *vector, void *infos);
ZRVector* ZRVector2SideStrategy_new(void *infos);

ZRVector* ZRVector2SideStrategy_createFixed(_ size_t initialNbObj, size_t objSize, size_t objAlignment, ZRAllocator *allocator);
ZRVector* ZRVector2SideStrategy_createDynamic(size_t initialNbObj, size_t objSize, size_t objAlignment, ZRAllocator *allocator);

ZRVector* ZRVector2SideStrategy_createFixedM(_ size_t initialArrayNbObj, size_t initialMemoryNbObj, size_t objSize, size_t objAlignment, ZRAllocator *allocator);
ZRVector* ZRVector2SideStrategy_createDynamicM(size_t initialArrayNbObj, size_t initialMemoryNbObj, size_t objSize, size_t objAlignment, ZRAllocator *allocator);


#endif
