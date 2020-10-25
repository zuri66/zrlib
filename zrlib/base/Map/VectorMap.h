/**
 * @author zuri
 * @date dim. 24 mai 2020 16:35:38 CEST
 */

#ifndef ZRVECTORMAP_H
#define ZRVECTORMAP_H

#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Map/Map.h>
#include <zrlib/base/Vector/Vector.h>

enum ZRVectorMap_modeE
{
	ZRVectorMap_modeEq, ZRVectorMap_modeOrder
};

ZRObjInfos ZRVectorMapInfos_objInfos(void);

void ZRVectorMapInfos(void *infos, ZRObjInfos keyInfos, ZRObjInfos objInfos);
void ZRVectorMapInfos_staticStrategy(void *infos);
void ZRVectorMapInfos_allocator(void *infos, ZRAllocator *allocator);
void ZRVectorMapInfos_fucmp(void *infos, zrfucmp fucmp, enum ZRVectorMap_modeE mode);
void ZRVectorMapInfos_vector(void *infos, ZRVector *vector, bool destroyVector);
void ZRVectorMapInfos_staticVector(void *infos, void *vectorInfos,
	void (*fsetObjSize)(void*, ZRObjInfos),
	ZRObjInfos (*finfos_objSize)(void*),
	void (*finit)(ZRVector*, void*)
	);

ZRObjInfos ZRVectorMap_objInfos(void *infos);
/* Infos of the item store in the vector */
ZRObjInfos ZRVectorMap_itemObjInfos(void *infos);

void ZRVectorMap_init(ZRMap *map, void *infos);
ZRMap* ZRVectorMap_new(void *infos);

ZRMap* ZRVectorMap_create(
	ZRObjInfos keyInfos, ZRObjInfos objInfos,
	zrfucmp fucmp,
	ZRVector *vector,
	ZRAllocator *allocator,
	enum ZRVectorMap_modeE mode
	);

ZRVector* ZRVectorMap_vector(ZRMap *map);

#endif
