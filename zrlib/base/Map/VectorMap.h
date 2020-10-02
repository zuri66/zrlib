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

void ZRVectorMapInfos(void *infos, ZRObjInfos keyInfos, ZRObjInfos objInfos, ZRAllocator *allocator);
void ZRVectorMapInfos_staticStrategy(void *infos);
void ZRVectorMapInfos_fucmp(void *infos, zrfucmp fucmp, enum ZRVectorMap_modeE mode);

ZRObjInfos ZRVectorMap_objInfos(void *infos);
void ZRVectorMap_init(ZRMap *map, void *infos);
ZRMap* ZRVectorMap_new(void *infos);


ZRMap* ZRVectorMap_create(
	size_t keySize, size_t keyAlignment,
	size_t objSize, size_t objAlignment,
	zrfucmp fucmp,
	ZRVector *vector,
	ZRAllocator *allocator,
	enum ZRVectorMap_modeE mode
	);

ZRVector* ZRVectorMap_vector(ZRMap *map);

#endif
