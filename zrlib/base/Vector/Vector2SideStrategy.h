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

/* ========================================================================= */
/* STRATEGY */

void ZRVector2SideStrategy_growStrategy(_ ZRVector *vec, zrflimit fupLimit, _ zrfincrease fincrease);
void ZRVector2SideStrategy_shrinkStrategy(ZRVector *vec, zrflimit fdownLimit, zrfdecrease fdecrease);

/* ========================================================================= */

/**
 * The ObjInfos of the information object for initialization of the vector.
 */
ZRObjInfos ZRVector2SideStrategyInfos_objInfos(void);

void ZRVector2SideStrategyInfos(void *infos_out, ZRObjInfos objInfos);
void ZRVector2SideStrategyInfos_allocator(void *infos_out, ZRAllocator *allocator);
void ZRVector2SideStrategyInfos_initialArraySize(void *infos_out, size_t size);
void ZRVector2SideStrategyInfos_initialMemorySize(void *infos_out, size_t size);
void ZRVector2SideStrategyInfos_fixedArray(void *infos_out);
void ZRVector2SideStrategyInfos_oneSide(void *infos_out);

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


/* HELPERS */

#define ZRVector2SideStrategy_(prefix, ...) ZRCONCAT(ZRVector2SideStrategy_ ## prefix ## _, ZRMACRO_NARGS(__VA_ARGS__))(__VA_ARGS__)
#define ZRVector2SideStrategy_createFixed(...)   ZRVector2SideStrategy_(createFixed, __VA_ARGS__)
#define ZRVector2SideStrategy_createDynamic(...) ZRVector2SideStrategy_(createDynamic, __VA_ARGS__)

ZRVector* ZRVector2SideStrategy_createFixed_1(ZRObjInfos objInfos);
ZRVector* ZRVector2SideStrategy_createFixed_2(ZRObjInfos objInfos, size_t capacity);
ZRVector* ZRVector2SideStrategy_createFixed_3(ZRObjInfos objInfos, size_t capacity, ZRAllocator *allocator);

ZRVector* ZRVector2SideStrategy_createDynamic_1(ZRObjInfos objInfos);
ZRVector* ZRVector2SideStrategy_createDynamic_2(ZRObjInfos objInfos, size_t capacity);
ZRVector* ZRVector2SideStrategy_createDynamic_3(ZRObjInfos objInfos, size_t capacity, ZRAllocator *allocator);

#endif
