/**
 * @author zuri
 * @date lundi 28 octobre 2019, 20:07:09 (UTC+0100)
 */

#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <zrlib/config.h>
#include <zrlib/syntax_pad.h>

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
	size_t blockSize;
	size_t nbBlocks;

	/**
	 * The strategy for memory management and operations on the pool
	 */
	ZRMemoryPoolStrategy *strategy;
};

struct ZRMemoryPoolStrategyS
{
	size_t (*fstrategySize)(void);

	void (*finit)(ZRMemoryPool *pool);
	void (*fdone)(ZRMemoryPool *pool);
	void (*fdestroy)(ZRMemoryPool *pool);
	void (*fclean)(ZRMemoryPool *pool);

	void* (*freserve)(ZRMemoryPool *pool, size_t nb);
	void _(*frelease)(ZRMemoryPool *pool, void *firstBlock, size_t nb);

	size_t (*fareaNbBlocks)(ZRMemoryPool *pool, void *firstBlock);

	/**
	 * Pools of the same type must answer.
	 */
	ZRMemoryPool* (*fareaPool)(ZRMemoryPool *pool, void *firstBlock);
};

// ============================================================================

ZRMUSTINLINE
static inline void ZRMPOOL_INIT(ZRMemoryPool *pool, size_t blockSize, ZRMemoryPoolStrategy *strategy)
{
	*pool = (ZRMemoryPool ) { //
		.blockSize = blockSize, //
		.strategy = strategy, //
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
static inline ZRMemoryPool* ZRMPOOL_AREAPOOL(ZRMemoryPool *pool, void *firstBlock)
{
	return pool->strategy->fareaPool(pool, firstBlock);
}

ZRMUSTINLINE
static inline size_t ZRMPOOL_NBBLOCKS(ZRMemoryPool *pool)
{
	return pool->nbBlocks;
}

ZRMUSTINLINE
static inline size_t ZRMPOOL_BLOCKSIZE(ZRMemoryPool *pool)
{
	return pool->blockSize;
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
static inline void ZRMPOOL_RELEASE_NB(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	pool->strategy->frelease(pool, firstBlock, nb);
}

// ============================================================================

void ZRMPool_init(ZRMemoryPool *pool, size_t objSize, ZRMemoryPoolStrategy *strategy);
void ZRMPool_done(ZRMemoryPool *pool);
void ZRMPool_destroy(ZRMemoryPool *pool);
void ZRMPool_clean(ZRMemoryPool *pool);

size_t ZRMPool_nbBlocks(_ ZRMemoryPool *pool);
size_t ZRMPool_areaNbBlocks(ZRMemoryPool *pool, void *firstBlock);
size_t ZRMPool_blockSize(ZRMemoryPool *pool);

ZRMemoryPool* ZRMPool_areaPool(ZRMemoryPool *pool, void *firstBlock);

void* ZRMPool_reserve(__ ZRMemoryPool *pool);
void* ZRMPool_reserve_nb(ZRMemoryPool *pool, size_t nb);

void ZRMPool_releaseArea(__ ZRMemoryPool *pool, void *firstBlock);
void ZRMPool_release_nb(ZRMemoryPool *pool, void *firstBlock, size_t nb);

#endif
