/**
 * @author zuri
 * @date samedi 26 octobre 2019, 18:24:01 (UTC+0200)
 */

size_t ZRVector2SideStrategy_vectorSize(size_t initialSpace, size_t objSize)
{
	return sizeof(ZRVector) + sizeof(ZRVector2SideData) + (initialSpace * objSize);
}

ZRVector* ZRVector2SideStrategy_alloc(size_t initialSpace, size_t objSize, ZRAllocator *allocator)
{
	size_t const vecSize = ZRVector2SideStrategy_vectorSize(initialSpace, objSize);
	ZRVector *ret = ZRALLOC(allocator, vecSize);
	return ret;
}

ZRVector* ZRVector2SideStrategy_createFixed(size_t initialSpace, size_t objSize, ZRAllocator *allocator)
{
	ZRVectorStrategy *strategy = ZRALLOC(allocator, sizeof(ZRVector2SideStrategy));
	ZRVector2SideStrategy_init(strategy, allocator, initialSpace, 0);
	strategy->fdestroy = ZRVector2SideStrategy_destroy;
	ZRVector *vec = ZRVector2SideStrategy_alloc(initialSpace, objSize, allocator);
	ZRVector_init(vec, objSize, strategy);
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
	ZRVector *vec = ZRVector2SideStrategy_alloc(initialArraySpace, objSize, allocator);
	ZRVector_init(vec, objSize, strategy);
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
	ZRVector_done(vec);
	ZRFREE(allocator, vec->strategy);
	ZRFREE(allocator, vec);
}
