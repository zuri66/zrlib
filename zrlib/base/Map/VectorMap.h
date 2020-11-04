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

ZRObjInfos ZRVectorMapIInfosObjInfos(void);

void ZRVectorMapIInfos(void *iinfos, ZRObjInfos keyInfos, ZRObjInfos objInfos);
void ZRVectorMapIInfos_keyInfos(void *iinfos, ZRObjInfos keyInfos);
void ZRVectorMapIInfos_objInfos(void *iinfos, ZRObjInfos objInfos);
void ZRVectorMapIInfos_staticStrategy(void *iinfos);
void ZRVectorMapIInfos_allocator(void *iinfos, ZRAllocator *allocator);
void ZRVectorMapIInfos_fucmp(void *iinfos, zrfucmp fucmp, enum ZRVectorMap_modeE mode);
void ZRVectorMapIInfos_vector(void *iinfos, ZRVector *vector, bool destroyVector);
void ZRVectorMapIInfos_staticVector(void *iinfos, void *vectorInfos,
	void (*fsetObjSize)(void*, ZRObjInfos),
	ZRObjInfos (*fiinfosObjSize)(void*iinfos),
	void (*finit)(ZRVector*, void*)
	);

ZRObjInfos ZRVectorMap_objInfos(void *iinfos);
/* Infos of the item store in the vector */
ZRObjInfos ZRVectorMap_itemObjInfos(void *iinfos);

void ZRVectorMap_init(ZRMap *map, void *iinfos);
ZRMap* ZRVectorMap_new(void *iinfos);

ZRMap* ZRVectorMap_create(
	ZRObjInfos keyInfos, ZRObjInfos objInfos,
	zrfucmp fucmp,
	ZRVector *vector,
	ZRAllocator *allocator,
	enum ZRVectorMap_modeE mode
	);

ZRVector* ZRVectorMap_vector(ZRMap *map);

#endif
