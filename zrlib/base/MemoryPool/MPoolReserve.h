/**
 * @author zuri
 * @date lundi 25 novembre 2019, 23:34:25 (UTC+0100)
 */

#ifndef MPOOLRESERVE_H
#define MPOOLRESERVE_H

#include <zrlib/syntax_pad.h>
#include <zrlib/base/MemoryPool/MemoryPool.h>
#include <zrlib/base/Allocator/Allocator.h>

#include <stdbool.h>

// ============================================================================

typedef struct ZRMPoolReserveStrategyS ZRMPoolReserveStrategy;

// ============================================================================

struct ZRMPoolReserveStrategyS
{
	ZRMemoryPoolStrategy strategy;

	ZRAllocator *allocator;

	bool (*favailablePos)(ZRMemoryPool *pool, size_t pos, size_t nb);
	void* (*freservePos)(ZRMemoryPool *pool, size_t pos, size_t nb);
};

// ============================================================================

static inline bool ZRMPOOLRESERVE_AVAILABLEPOS_NB(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	return ((ZRMPoolReserveStrategy*)(pool->strategy))->favailablePos(pool, pos, nb);
}

static inline bool ZRMPOOLRESERVE_AVAILABLEPOS(ZRMemoryPool *pool, size_t pos)
{
	return ZRMPOOLRESERVE_AVAILABLEPOS_NB(pool, pos, 1);
}

static inline void* ZRMPOOLRESERVE_RESERVEPOS_NB(ZRMemoryPool *pool, size_t pos, size_t nb, bool checkAvailability)
{
	if (checkAvailability && !ZRMPOOLRESERVE_AVAILABLEPOS_NB(pool, pos, nb))
		return NULL;

	return ((ZRMPoolReserveStrategy*)(pool->strategy))->freservePos(pool, pos, nb);
}

static inline void* ZRMPOOLRESERVE_RESERVEPOS(ZRMemoryPool *pool, size_t pos, bool checkAvailability)
{
	return ZRMPOOLRESERVE_RESERVEPOS_NB(pool, pos, 1, checkAvailability);
}

// ============================================================================

bool ZRMPoolReserve_availablePos_nb(ZRMemoryPool *pool, size_t pos, size_t nb);
bool ZRMPoolReserve_availablePos(ZRMemoryPool *pool, size_t pos);
void* ZRMPoolReserve_reservePos_nb(ZRMemoryPool *pool, size_t pos, size_t nb, bool checkAvailability);
void* ZRMPoolReserve_reservePos(ZRMemoryPool *pool, size_t pos, bool checkAvailability);

// ============================================================================
// HELP
// ============================================================================

ZRMemoryPool* ZRMPoolReserve_create(size_t blockSize, size_t alignment, size_t nbBlocks, ZRAllocator *allocator, bool bitStrategy);
void ZRMPoolReserve_destroy(ZRMemoryPool *pool);

// ============================================================================

#endif
