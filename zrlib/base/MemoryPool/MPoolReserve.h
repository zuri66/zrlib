/**
 * @author zuri
 * @date lundi 25 novembre 2019, 23:34:25 (UTC+0100)
 */

#ifndef MPOOLRESERVE_H
#define MPOOLRESERVE_H

#include <zrlib/syntax_pad.h>
#include <zrlib/base/MemoryPool/MemoryPool.h>
#include <zrlib/base/Allocator/Allocator.h>

// ============================================================================

static inline void* ZRMPOOLRESERVE_RESERVEPOS(ZRMemoryPool *pool, size_t pos)
{

}

static inline void* ZRMPOOLRESERVE_RESERVEPOS_NB(ZRMemoryPool *pool, size_t pos, size_t nb)
{

}

// ============================================================================
// HELP
// ============================================================================

ZRMemoryPool* ZRMPoolReserve_create(size_t objSize, size_t nbBlocks, ZRAllocator *allocator);
void ZRMPoolReserve_destroy(ZRMemoryPool *pool);

// ============================================================================

#endif
