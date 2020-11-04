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

ZRObjInfos ZRMPoolDSIInfosObjInfos(void);

void ZRMPoolDSIInfos(void *iinfos, ZRObjInfos objInfos);
void ZRMPoolDSIInfos_staticStrategy(__ void *iinfos);
void ZRMPoolDSIInfos_allocator(_______ void *iinfos, ZRAllocator *allocator);
void ZRMPoolDSIInfos_initialBucketSize(void *iinfos, size_t initialBucketSize);
void ZRMPoolDSIInfos_maxFreeBuckets(__ void *iinfos, size_t maxFreeBuckets);

ZRObjInfos ZRMPoolDS_objInfos(void *iinfos);
void ZRMPoolDS_init(ZRMemoryPool *pool, void *iinfos);
ZRMemoryPool* ZRMPoolDS_new(void *iinfos);

ZRMemoryPool* ZRMPoolDS_create(______ size_t initialBucketSize, size_t maxFreeBuckets, ZRObjInfos objInfos, ZRAllocator *allocator);
ZRMemoryPool* ZRMPoolDS_createBS(____ size_t initialBucketSize, ______________________ ZRObjInfos objInfos, ZRAllocator *allocator);
ZRMemoryPool* ZRMPoolDS_createMaxFB(_ _________________________ size_t maxFreeBuckets, ZRObjInfos objInfos, ZRAllocator *allocator);
ZRMemoryPool* ZRMPoolDS_createDefault(_________________________ ______________________ ZRObjInfos objInfos, ZRAllocator *allocator);

// ============================================================================

#endif
