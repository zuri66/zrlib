/**
 * @author zuri
 * @date lundi 28 octobre 2019, 20:45:13 (UTC+0100)
 */

#include <zrlib/base/MemoryPool/MemoryPool.h>

void ZRMPool_init(ZRMemoryPool *pool, ZRObjInfos blockInfos, ZRMemoryPoolStrategy *strategy)
{
	ZRMPOOL_INIT(pool, blockInfos, strategy);
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

void* ZRMPool_areaMetaData(ZRMemoryPool *pool, void *firstBlock)
{
	return ZRMPOOL_AREAMETADATA(pool, firstBlock);
}

size_t ZRMPool_nbBlocks(ZRMemoryPool *pool)
{
	return ZRMPOOL_NBBLOCKS(pool);
}

ZRObjInfos ZRMPool_blockInfos(ZRMemoryPool *pool)
{
	return ZRMPOOL_BLOCKINFOS(pool);
}

size_t ZRMPool_blockAlignment(ZRMemoryPool *pool)
{
	return ZRMPOOL_BLOCKALIGNMENT(pool);
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
