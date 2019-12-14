/**
 * @author zuri
 * @date mardi 26 novembre 2019, 00:23:51 (UTC+0100)
 */

ZRMemoryPool* ZRMPoolReserve_create(size_t blockSize, size_t nbBlocks, ZRAllocator *allocator, bool bitStrategy)
{
	ZRMemoryPool *pool;
	ZRMemoryPoolStrategy *strategy = ZRALLOC(allocator, sizeof(ZRMPoolReserveStrategy));

	if (bitStrategy)
	{
		size_t const nbZRBits = ZRBITS_NBOF / nbBlocks + (ZRBITS_NBOF % nbBlocks) ? 1 : 0;
		size_t const reserveSize = nbBlocks * blockSize;

		TYPEDEF_SDATA_BITS(nbZRBits, reserveSize);
		ZRMPoolReserve_init(strategy, allocator, true);

		size_t poolSize = sizeof(ZRMemoryPool) + sizeof(ZRMPoolReserveDataBits);
		pool = ZRALLOC(allocator, poolSize);

		ZRMPoolReserveDataBits *data = ZRMPOOL_DATA_BITS(pool);
		data->nbZRBits = nbZRBits;
		data->reserveSize = reserveSize;
	}
	else
	{
		TYPEDEF_SDATA_LIST_ALL(blockSize, nbBlocks);
		ZRMPoolReserve_init(strategy, allocator, false);

		size_t poolSize = sizeof(ZRMemoryPool) + sizeof(ZRMPoolReserveDataList);
		pool = ZRALLOC(allocator, poolSize);

		ZRMPoolReserveDataList *data = ZRMPOOL_DATA_LIST(pool);
		data->nbBlocks = nbBlocks;
	}
	ZRMPool_init(pool, blockSize, strategy);
	return pool;
}

void ZRMPoolReserve_destroy(ZRMemoryPool *pool)
{
	ZRMPool_done(pool);
	ZRAllocator *allocator = ZRMPOOL_STRATEGY(pool)->allocator;
	ZRFREE(allocator, pool->strategy);
	ZRFREE(allocator, pool);
}
