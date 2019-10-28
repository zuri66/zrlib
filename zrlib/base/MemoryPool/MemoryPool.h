/**
 * @author zuri
 * @date lundi 28 octobre 2019, 20:07:09 (UTC+0100)
 */

#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <zrlib/config.h>
#include <zrlib/syntax_pad.h>

#include <stddef.h>
#include <string.h>

// ============================================================================

typedef struct ZRMemoryPoolS ZRMemoryPool;
typedef struct ZRMemoryPoolStrategyS ZRMemoryPoolStrategy;

// ============================================================================

struct ZRMemoryPoolS
{
	size_t blockSize;
	size_t nbBlock;

	/**
	 * The strategy for memory management and operations on the pool
	 */
	ZRMemoryPoolStrategy *strategy;

	/*
	 * Data for Strategy purpose.
	 */
	char sdata[];
};

struct ZRMemoryPoolStrategyS
{
	size_t (*fsdataSize)(ZRMemoryPool *pool);
	size_t (*fstrategySize)();

	/**
	 * (optional)
	 */
	void (*finitPool)(ZRMemoryPool *pool);

	/**
	 * (optional)
	 */
	void (*finitSData)(char *sdata, ZRMemoryPoolStrategy *strategy);

	void* (*fget)(___ ZRMemoryPool *pool, size_t nb);
	void _(*frelease)(ZRMemoryPool *pool, void *firstBlock, size_t nb);

	/**
	 * Clean the memory used by the pool.
	 * The pool MUST NOT be used after this call.
	 */
	void (*fclean)(ZRMemoryPool *pool);
};

// ============================================================================

static inline void ZRMPOOL_INIT(ZRMemoryPool *pool, size_t blockSize, ZRMemoryPoolStrategy *strategy)
{
	*pool = (ZRMemoryPool ) { //
		.blockSize = blockSize, //
		.strategy = strategy //
		};

	if (strategy->finitSData == NULL)
		memset(pool->sdata, 0, strategy->fsdataSize(pool));
	else
		strategy->finitSData(pool->sdata, strategy);
}

static inline void ZRMPOOL_CLEAN(ZRMemoryPool *pool)
{
	pool->strategy->fclean(pool);
}

static inline size_t ZRMPOOL_NBBLOCK(ZRMemoryPool *pool)
{
	return pool->nbBlock;
}

static inline size_t ZRMPOOL_BLOCKSIZE(ZRMemoryPool *pool)
{
	return pool->blockSize;
}

static inline void* ZRMPOOL_GET(ZRMemoryPool *pool)
{
	return pool->strategy->fget(pool, 1);
}

static inline void* ZRMPOOL_GET_NB(ZRMemoryPool *pool, size_t nb)
{
	return pool->strategy->fget(pool, nb);
}

static inline void ZRMPOOL_RELEASE(ZRMemoryPool *pool, void *block)
{
	pool->strategy->frelease(pool, block, 1);
}

static inline void ZRMPOOL_RELEASE_NB(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	pool->strategy->frelease(pool, firstBlock, nb);
}

// ============================================================================

void ZRMPool_init(ZRMemoryPool *pool, size_t objSize, ZRMemoryPoolStrategy *strategy);
void ZRMPool_clean(ZRMemoryPool *pool);

size_t ZRMPool_nbBlock(_ ZRMemoryPool *pool);
size_t ZRMPool_blockSize(ZRMemoryPool *pool);

void* ZRMPool_get(__ ZRMemoryPool *pool);
void* ZRMPool_get_nb(ZRMemoryPool *pool, size_t nb);

void ZRMPool_release(__ ZRMemoryPool *pool, void *block);
void ZRMPool_release_nb(ZRMemoryPool *pool, void *firstBlock, size_t nb);

#endif
