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
};

// ============================================================================
// HELP
// ============================================================================

enum ZRMPoolReserveModeE
{
	ZRMPoolReserveMode_bits, ZRMPoolReserveMode_list, ZRMPoolReserveMode_chunk
};

ZRMemoryPool* ZRMPoolReserve_create(size_t blockSize, size_t alignment, size_t nbBlocks, ZRAllocator *allocator, enum ZRMPoolReserveModeE mode);
void ZRMPoolReserve_destroy(ZRMemoryPool *pool);

// ============================================================================

#endif
