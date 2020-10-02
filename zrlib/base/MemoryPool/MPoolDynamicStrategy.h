/**
 * @author zuri
 * @date mardi 29 octobre 2019, 19:39:20 (UTC+0100)
 */


#ifndef ZRMPOOLDYNAMICSTRATEGY_H
#define ZRMPOOLDYNAMICSTRATEGY_H

#include <zrlib/config.h>
#include <zrlib/base/struct.h>

#include <zrlib/base/MemoryPool/MemoryPool.h>
#include <zrlib/base/Allocator/Allocator.h>

// ============================================================================
// HELP
// ============================================================================

ZRObjInfos ZRMPoolDSInfos_objInfos(void);

void ZRMPoolDSInfos(void *infos_out, ZRObjInfos objInfos, ZRAllocator *allocator);

void ZRMPoolDSInfos_staticStrategy(__ void *infos);
void ZRMPoolDSInfos_initialBucketSize(void *infos, size_t initialBucketSize);
void ZRMPoolDSInfos_maxFreeBuckets(__ void *infos, size_t maxFreeBuckets);

void ZRMPoolDS_init(ZRMemoryPool *pool, void *initInfos_p);
ZRMemoryPool* ZRMPoolDS_new(void *initInfos_p);

ZRMemoryPool* ZRMPoolDS_create(______ size_t initialBucketSize, size_t maxFreeBuckets, size_t objSize, size_t objAlignment, ZRAllocator *allocator);
ZRMemoryPool* ZRMPoolDS_createBS(____ size_t initialBucketSize, ______________________ size_t objSize, size_t objAlignment, ZRAllocator *allocator);
ZRMemoryPool* ZRMPoolDS_createMaxFB(_ _________________________ size_t maxFreeBuckets, size_t objSize, size_t objAlignment, ZRAllocator *allocator);
ZRMemoryPool* ZRMPoolDS_createDefault(_________________________ ______________________ size_t objSize, size_t objAlignment, ZRAllocator *allocator);

// ============================================================================

#endif
