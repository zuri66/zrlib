/**
 * @author zuri
 * @date samedi 26 octobre 2019, 18:24:01 (UTC+0200)
 */

#include <stdalign.h>

void ZRVector2SideStrategy_init(ZRVectorStrategy *strategy)
{
	*(ZRVector2SideStrategy*)strategy = (ZRVector2SideStrategy ) //
		{ //
		.strategy = { //
			.finitVec = ZRVector2SideStrategy_finitVec, //
			.fstrategySize = ZRVector2SideStrategy_size, //
			.finsert = ZRVector2SideStrategy_finsert, //
			.fdelete = ZRVector2SideStrategy_fdelete, //
			.fchangeObjSize = ZRVector2SideStrategy_fchangeObjSize, //
			.fmemoryTrim = ZRVector2SideStrategy_fmemoryTrim, //
			.fdone = ZRVector2SideStrategy_fdone //
			},//
		.fmustGrow = mustGrowSimple, //
		.fincreaseSpace = increaseSpaceTwice, //
		.fmustShrink = mustShrink4, //
		.fdecreaseSpace = decreaseSpaceTwice, //
		};
}

/**
 * Delete a vector created by one of the creation helper functions
 */
void ZRVector2SideStrategy_destroy(ZRVector *vec)
{
	ZRAllocator *const allocator = ZRVECTOR_2SS(vec)->allocator;
	ZRVECTOR_DONE(vec);
	ZRFREE(allocator, vec->strategy);
	ZRFREE(allocator, vec);
}

ZRVector* ZRVector2SideStrategy_alloc(size_t initialNbObjs, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZRObjAlignInfos infos[ZRVECTOR_INFOS_NB];

	if(initialNbObjs < 2)
		initialNbObjs = 2;

	vectorInfos(infos, initialNbObjs, objSize, objAlignment);

	size_t const vecSize = infos[ZRVectorInfos_struct].size;
	ZR2SSVector *const ret = ZRALLOC(allocator, vecSize);

	memset(ret, 0, sizeof(ZR2SSVector));
	ret->allocator = allocator;
	ret->initialArrayOffset = infos[ZRVectorInfos_objs].offset;
	ret->initialArray = (char*)ret + infos[ZRVectorInfos_objs].offset;
	ret->initialArraySize = infos[ZRVectorInfos_objs].size;
	ret->initialMemoryNbObjs = initialNbObjs;
	return ZR2SS_VECTOR(ret);
}

/**
 * Create a fixed vector with an initial space.
 */
ZRVector* ZRVector2SideStrategy_createFixed(size_t initialNbObjs, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZRVectorStrategy *strategy = ZRALLOC(allocator, sizeof(ZRVector2SideStrategy));
	ZRVector2SideStrategy_init(strategy);
	strategy->fdestroy = ZRVector2SideStrategy_destroy;
	ZRVector *vec = ZRVector2SideStrategy_alloc(initialNbObjs, objSize, objAlignment, allocator);
	ZRVECTOR_INIT(vec, objSize, objAlignment, strategy);
	return vec;
}

/**
 * Create a dynamic vector with an initial space.
 */
ZRVector* ZRVector2SideStrategy_createDynamic(size_t initialNbObjs, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZRVector *vec = ZRVector2SideStrategy_createFixed(initialNbObjs, objSize, objAlignment, allocator);
	ZRVector2SideStrategy_dynamicMemory(vec->strategy);
	return vec;
}

ZRVector* ZRVector2SideStrategy_createFixedM(size_t initialArraySpace, size_t initialMemorySpace, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZRVectorStrategy *strategy = ZRALLOC(allocator, sizeof(ZRVector2SideStrategy));
	ZRVector2SideStrategy_init(strategy);
	strategy->fdestroy = ZRVector2SideStrategy_destroy;
	ZRVector *vec = ZRVector2SideStrategy_alloc(initialArraySpace, objSize, objAlignment, allocator);
	ZRVECTOR_INIT(vec, objSize, objAlignment, strategy);
	return vec;
}

ZRVector* ZRVector2SideStrategy_createDynamicM(size_t initialArraySpace, size_t initialMemorySpace, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZRVector *vec = ZRVector2SideStrategy_createFixedM(initialArraySpace, initialMemorySpace, objSize, objAlignment, allocator);
	ZRVector2SideStrategy_dynamicMemory(vec->strategy);
	return vec;
}
