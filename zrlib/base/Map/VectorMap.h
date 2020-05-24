/**
 * @author zuri
 * @date dim. 24 mai 2020 16:35:38 CEST
 */

#ifndef ZRVECTORMAP_H
#define ZRVECTORMAP_H

#include <zrlib/config.h>

#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Map/Map.h>
#include <zrlib/base/Vector/Vector.h>

enum ZRVectorMap_modeE{
	ZRVectorMap_modeEq, ZRVectorMap_modeOrder
};

ZRMap* ZRVectorMap_create(
	size_t keySize, size_t keyAlignment,
	size_t objSize, size_t objAlignment,
	int (*fcmp)(void*, void*),
	ZRVector *vector,
	ZRAllocator *allocator,
	enum ZRVectorMap_modeE mode
	);

#endif
