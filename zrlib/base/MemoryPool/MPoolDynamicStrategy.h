/**
 * @author zuri
 * @date mardi 29 octobre 2019, 19:39:20 (UTC+0100)
 */


#ifndef ZRMPOOLDYNAMICSTRATEGY_H
#define ZRMPOOLDYNAMICSTRATEGY_H

#include <zrlib/config.h>

#include <zrlib/syntax_pad.h>
#include <zrlib/base/MemoryPool/MemoryPool.h>
#include <zrlib/base/Allocator/Allocator.h>

// ============================================================================
// HELP
// ============================================================================

ZRMemoryPool* ZRMPoolDS_create(______ size_t initialBucketSize, size_t maxFreeBuckets, size_t objSize, ZRAllocator *allocator);
ZRMemoryPool* ZRMPoolDS_createBS(____ size_t initialBucketSize, ______________________ size_t objSize, ZRAllocator *allocator);
ZRMemoryPool* ZRMPoolDS_createMaxFB(_ _________________________ size_t maxFreeBuckets, size_t objSize, ZRAllocator *allocator);
ZRMemoryPool* ZRMPoolDS_createDefault(_________________________ ______________________ size_t objSize, ZRAllocator *allocator);

void ZRMPoolDS_destroy(ZRMemoryPool *pool);

// ============================================================================

#endif
