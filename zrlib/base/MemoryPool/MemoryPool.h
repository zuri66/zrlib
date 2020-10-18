/**
 * @author zuri
 * @date lundi 28 octobre 2019, 20:07:09 (UTC+0100)
 */

#ifndef ZRMEMORYPOOL_H
#define ZRMEMORYPOOL_H

#include <zrlib/config.h>
#include <zrlib/base/struct.h>

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// ============================================================================

typedef struct ZRMemoryPoolS ZRMemoryPool;
typedef struct ZRMemoryPoolStrategyS ZRMemoryPoolStrategy;

// ============================================================================

struct ZRMemoryPoolS
{
	ZRObjInfos blockInfos;
	size_t nbBlocks;

	/**
	 * The strategy for memory management and operations on the pool
	 */
	ZRMemoryPoolStrategy *strategy;
};

#define ZRMPOOL_BLOCKINFOS(P) (P)->blockInfos
#define ZRMPOOL_BLOCKSIZE(P) (P)->blockInfos.size
#define ZRMPOOL_BLOCKALIGNMENT(P) (P)->blockInfos.alignment
#define ZRMPOOL_NBBLOCKS(P) (P)->nbBlocks

struct ZRMemoryPoolStrategyS
{
	void (*finit)(ZRMemoryPool *pool);
	void (*fdone)(ZRMemoryPool *pool);
	void (*fdestroy)(ZRMemoryPool *pool);
	void (*fclean)(ZRMemoryPool *pool);

	void* (*freserve)(ZRMemoryPool *pool, size_t nb);
	void* (*frelease)(ZRMemoryPool *pool, void *firstBlock, size_t nb);

	size_t (*fareaNbBlocks)(ZRMemoryPool *pool, void *firstBlock);

	/*
	 * Get a pointer to the begin of user MetaData.
	 * The pointer may not be aligned to the alignof(T), T the type attempted by the user.
	 */
	void* (*fareaMetaData)(ZRMemoryPool *pool, void *firstBlock);
};

// ============================================================================

ZRMUSTINLINE
static inline void ZRMPOOL_INIT(ZRMemoryPool *pool, ZRObjInfos blockInfos, ZRMemoryPoolStrategy *strategy)
{
	*pool = (struct ZRMemoryPoolS)
	{
		.blockInfos = blockInfos,
		.strategy = strategy,
	};
	strategy->finit(pool);
}

ZRMUSTINLINE
static inline void ZRMPOOL_DONE(ZRMemoryPool *pool)
{
	pool->strategy->fdone(pool);
}

ZRMUSTINLINE
static inline void ZRMPOOL_DESTROY(ZRMemoryPool *pool)
{
	pool->strategy->fdestroy(pool);
}

ZRMUSTINLINE
static inline void ZRMPOOL_CLEAN(ZRMemoryPool *pool)
{
	pool->strategy->fclean(pool);
}

ZRMUSTINLINE
static inline size_t ZRMPOOL_AREANBBLOCKS(ZRMemoryPool *pool, void *firstBlock)
{
	return pool->strategy->fareaNbBlocks(pool, firstBlock);
}

ZRMUSTINLINE
static inline void* ZRMPOOL_AREAMETADATA(ZRMemoryPool *pool, void *firstBlock)
{
	return pool->strategy->fareaMetaData(pool, firstBlock);
}

ZRMUSTINLINE
static inline void* ZRMPOOL_RESERVE(ZRMemoryPool *pool)
{
	return pool->strategy->freserve(pool, 1);
}

ZRMUSTINLINE
static inline void* ZRMPOOL_RESERVE_NB(ZRMemoryPool *pool, size_t nb)
{
	return pool->strategy->freserve(pool, nb);
}

ZRMUSTINLINE
static inline void ZRMPOOL_RELEASEAREA(ZRMemoryPool *pool, void *block)
{
	pool->strategy->frelease(pool, block, SIZE_MAX);
}

ZRMUSTINLINE
static inline void* ZRMPOOL_RELEASE_NB(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	return pool->strategy->frelease(pool, firstBlock, nb);
}

// ============================================================================

void ZRMPool_init(ZRMemoryPool *pool, ZRObjInfos blockInfos, ZRMemoryPoolStrategy *strategy);
void ZRMPool_done(ZRMemoryPool *pool);
void ZRMPool_destroy(ZRMemoryPool *pool);
void ZRMPool_clean(ZRMemoryPool *pool);

size_t ZRMPool_nbBlocks(ZRMemoryPool *pool);
size_t ZRMPool_areaNbBlocks(ZRMemoryPool *pool, void *firstBlock);
void* ZRMPool_areaMetaData(ZRMemoryPool *pool, void *firstBlock);

ZRObjInfos ZRMPool_blockInfos(ZRMemoryPool *pool);
size_t ZRMPool_blockSize(ZRMemoryPool *pool);
size_t ZRMPool_blockAlignment(ZRMemoryPool *pool);

void* ZRMPool_reserve(__ ZRMemoryPool *pool);
void* ZRMPool_reserve_nb(ZRMemoryPool *pool, size_t nb);

void ZRMPool_release(ZRMemoryPool *pool, void *firstBlock);
void* ZRMPool_release_nb(ZRMemoryPool *pool, void *firstBlock, size_t nb);

#endif
