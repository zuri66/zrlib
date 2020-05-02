/**
 * @author zuri
 * @date lundi 28 octobre 2019, 20:45:13 (UTC+0100)
 */

#include <zrlib/base/MemoryPool/MemoryPool.h>

void ZRMPool_init(ZRMemoryPool *pool, size_t blockSize, ZRMemoryPoolStrategy *strategy)
{
	ZRMPOOL_INIT(pool, blockSize, strategy);
}

void ZRMPool_done(ZRMemoryPool *pool)
{
	ZRMPOOL_DONE(pool);
}

void ZRMPool_destroy(ZRMemoryPool *pool)
{
	ZRMPOOL_DESTROY(pool);
}

size_t ZRMPool_areaNbBlocks(ZRMemoryPool *pool, void *firstBlock)
{
	return ZRMPOOL_AREANBBLOCKS(pool, firstBlock);
}

void* ZRMPool_userAreaMetaData(ZRMemoryPool *pool, void *firstBlock)
{
	return ZRMPOOL_USERAREAMETADATA(pool, firstBlock);
}

size_t ZRMPool_nbBlocks(_ ZRMemoryPool *pool)
{
	return ZRMPOOL_NBBLOCKS(pool);
}

size_t ZRMPool_blockSize(ZRMemoryPool *pool)
{
	return ZRMPOOL_BLOCKSIZE(pool);
}

void* ZRMPool_reserve(ZRMemoryPool *pool)
{
	return ZRMPOOL_RESERVE(pool);
}

void* ZRMPool_reserve_nb(ZRMemoryPool *pool, size_t nb)
{
	return ZRMPOOL_RESERVE_NB(pool, nb);
}

void ZRMPool_releaseArea(ZRMemoryPool *pool, void *firstBlock)
{
	ZRMPOOL_RELEASEAREA(pool, firstBlock);
}

void* ZRMPool_release_nb(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	return ZRMPOOL_RELEASE_NB(pool, firstBlock, nb);
}

void ZRMPool_clean(ZRMemoryPool *pool)
{
	ZRMPOOL_CLEAN(pool);
}
