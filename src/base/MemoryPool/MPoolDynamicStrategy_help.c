/**
 * @author zuri
 * @date dimanche 3 novembre 2019, 23:43:59 (UTC+0100)
 */

ZRMemoryPool* ZRMPoolDS_create(size_t initialBucketSize, size_t maxFreeBuckets, size_t objSize, ZRAllocator *allocator)
{
	ZRMemoryPoolStrategy *strategy = ZRALLOC(allocator, sizeof(ZRMPoolDynamicStrategy));
	ZRMPoolDS_init(strategy, allocator, initialBucketSize, maxFreeBuckets);
	strategy->fdestroy = ZRMPoolDS_destroy;
	ZRMemoryPool *pool = ZRALLOC(allocator, sizeof(ZRMPoolDS));
	ZRMPool_init(pool, objSize, strategy);
	return pool;
}

ZRMemoryPool* ZRMPoolDS_createBS(size_t initialBucketSize, size_t objSize, ZRAllocator *allocator)
{
	return ZRMPoolDS_create(initialBucketSize, DEFAULT_MAX_FREE_BUCKETS, objSize, allocator);
}

ZRMemoryPool* ZRMPoolDS_createMaxFB(size_t maxFreeBuckets, size_t objSize, ZRAllocator *allocator)
{
	return ZRMPoolDS_create(INITIAL_BUCKET_SIZE, maxFreeBuckets, objSize, allocator);
}

ZRMemoryPool* ZRMPoolDS_createDefault(size_t objSize, ZRAllocator *allocator)
{
	return ZRMPoolDS_create(INITIAL_BUCKET_SIZE, DEFAULT_MAX_FREE_BUCKETS, objSize, allocator);
}

void ZRMPoolDS_destroy(ZRMemoryPool *pool)
{
	ZRAllocator *allocator = ZRMPOOL_STRATEGY(pool)->allocator;
	ZRMPOOL_DONE(pool);
	ZRFREE(allocator, pool->strategy);
	ZRFREE(allocator, pool);
}
