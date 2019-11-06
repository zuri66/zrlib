/**
 * @author zuri
 * @date dimanche 3 novembre 2019, 23:43:59 (UTC+0100)
 */

ZRMemoryPool* ZRMPoolDS_create(size_t initialBucketSize, size_t objSize, ZRAllocator *allocator)
{
	ZRMemoryPoolStrategy *strategy = ZRALLOC(allocator, sizeof(ZRMPoolDynamicStrategy));
	ZRMPoolDS_init(strategy, allocator, initialBucketSize);
	ZRMemoryPool *pool = ZRALLOC(allocator, sizeof(ZRMemoryPool) + sizeof(ZRMPoolDynamicData));
	ZRMPool_init(pool, objSize, strategy);
	return pool;
}

ZRMemoryPool* ZRMPoolDS_destroy(ZRMemoryPool *pool)
{
	ZRMPool_done(pool);
	ZRAllocator *allocator = ZRMPOOL_STRATEGY(pool)->allocator;
	ZRFREE(allocator, pool->strategy);
	ZRFREE(allocator, pool);
}
