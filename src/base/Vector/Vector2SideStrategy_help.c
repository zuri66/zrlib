/**
 * @author zuri
 * @date samedi 26 octobre 2019, 18:24:01 (UTC+0200)
 */

#include <stdalign.h>

ZRVector* ZRVector2SideStrategy_alloc(size_t initialSpace, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZRObjAlignInfos infos[ZRVECTOR_INFOS_NB];
	vectorInfos(infos, initialSpace, objSize, objAlignment);
	size_t i = sizeof(infos);
	size_t const vecSize = infos[ZRVectorInfos_struct].size;
	ZRVector *ret = ZRALLOC(allocator, vecSize);
	*ZRVECTOR_2SS(ret) = (ZR2SSVector ) { 0 };
	memcpy(ZRVECTOR_2SS(ret)->infos, infos, sizeof(infos));
	return ret;
}

ZRVector* ZRVector2SideStrategy_createFixed(size_t initialSpace, size_t objSize, ZRAllocator *allocator)
{
	ZRVectorStrategy *strategy = ZRALLOC(allocator, sizeof(ZRVector2SideStrategy));
	ZRVector2SideStrategy_init(strategy, allocator, initialSpace, 0);
	strategy->fdestroy = ZRVector2SideStrategy_destroy;
	ZRVector *vec = ZRVector2SideStrategy_alloc(initialSpace, objSize, alignof(max_align_t), allocator);
	ZRVector_init(vec, objSize, alignof(max_align_t), strategy);
	return vec;
}

ZRVector* ZRVector2SideStrategy_createDynamic(size_t initialSpace, size_t objSize, ZRAllocator *allocator)
{
	ZRVector *vec = ZRVector2SideStrategy_createFixed(initialSpace, objSize, allocator);
	ZRVector2SideStrategy_dynamicMemory(vec->strategy);
	return vec;
}

ZRVector* ZRVector2SideStrategy_createFixedM(size_t initialArraySpace, size_t initialMemorySpace, size_t objSize, ZRAllocator *allocator)
{
	ZRVectorStrategy *strategy = ZRALLOC(allocator, sizeof(ZRVector2SideStrategy));
	ZRVector2SideStrategy_init(strategy, allocator, initialArraySpace, initialMemorySpace);
	strategy->fdestroy = ZRVector2SideStrategy_destroy;
	ZRVector *vec = ZRVector2SideStrategy_alloc(initialArraySpace, objSize, alignof(max_align_t), allocator);
	ZRVector_init(vec, objSize, alignof(max_align_t), strategy);
	return vec;
}

ZRVector* ZRVector2SideStrategy_createDynamicM(size_t initialArraySpace, size_t initialMemorySpace, size_t objSize, ZRAllocator *allocator)
{
	ZRVector *vec = ZRVector2SideStrategy_createFixedM(initialArraySpace, initialMemorySpace, objSize, allocator);
	ZRVector2SideStrategy_dynamicMemory(vec->strategy);
	return vec;
}

/**
 * Delete a vector created by one of the creation helper functions
 */
void ZRVector2SideStrategy_destroy(ZRVector *vec)
{
	ZRAllocator *const allocator = ZRVECTOR_STRATEGY(vec)->allocator;
	ZRVECTOR_DONE(vec);
	ZRFREE(allocator, vec->strategy);
	ZRFREE(allocator, vec);
}
