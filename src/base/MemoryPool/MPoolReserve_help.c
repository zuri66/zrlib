/**
 * @author zuri
 * @date mardi 26 novembre 2019, 00:23:51 (UTC+0100)
 */

ZRMemoryPool* ZRMPoolReserve_create(size_t objSize, size_t nbBlocks, ZRAllocator *allocator)
{
	ZRMemoryPoolStrategy *strategy = ZRALLOC(allocator, sizeof(ZRMPoolReserveStrategy));
	size_t const nbZRBits = ZRBITS_NBOF / nbBlocks + (ZRBITS_NBOF % nbBlocks) ? 1 : 0;
	size_t const reserveSize = nbBlocks * objSize;
	TYPEDEF_SDATA_BITS(nbZRBits, reserveSize);
	ZRMPoolReserve_init(strategy, allocator, true);
	ZRMemoryPool *pool = ZRALLOC(allocator, sizeof(ZRMemoryPool) + sizeof(ZRMPoolReserveData));

	ZRMPOOL_DATA_EXTEND(pool)->nbZRBits = nbZRBits;
	ZRMPOOL_DATA_EXTEND(pool)->reserveSize = reserveSize;

	ZRMPool_init(pool, objSize, strategy);
	return pool;
}

void ZRMPoolReserve_destroy(ZRMemoryPool *pool)
{
	ZRMPool_done(pool);
	ZRAllocator *allocator = ZRMPOOL_STRATEGY(pool)->allocator;
	ZRFREE(allocator, pool->strategy);
}
