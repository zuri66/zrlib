/**
 * @author zuri
 * @date lundi 28 octobre 2019, 20:45:13 (UTC+0100)
 */

#include <zrlib/base/MemoryPool/MemoryPool.h>

void ZRMPool_init(ZRMemoryPool *pool, size_t blockSize, ZRMemoryPoolStrategy *strategy)
{
	ZRMPOOL_INIT(pool, blockSize, strategy);
}

void ZRMPool_clean(ZRMemoryPool *pool)
{
	ZRMPOOL_CLEAN(pool);
}

size_t ZRMPool_nbBlock(_ ZRMemoryPool *pool)
{
	return ZRMPOOL_NBBLOCK(pool);
}

size_t ZRMPool_blockSize(ZRMemoryPool *pool)
{
	return ZRMPOOL_BLOCKSIZE(pool);
}

void* ZRMPool_get(ZRMemoryPool *pool)
{
	return ZRMPOOL_GET(pool);
}

void* ZRMPool_get_nb(ZRMemoryPool *pool, size_t nb)
{
	return ZRMPOOL_GET_NB(pool, nb);
}

void ZRMPool_release(ZRMemoryPool *pool, void *block)
{
	ZRMPOOL_RELEASE(pool, block);
}

void ZRMPool_release_nb(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPOOL_RELEASE_NB(pool, firstBlock, nb);
}
