/**
 * @author zuri
 * @date mardi 29 octobre 2019, 19:39:20 (UTC+0100)
 */

#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Bits/Bits.h>
#include <zrlib/base/MemoryPool/MPoolDynamicStrategy.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/ArrayOp.h>
#include <zrlib/base/ReserveOp_bits.h>
#include <zrlib/base/MemoryPool/MPoolReserve.h>

#include <stdalign.h>
#include <stddef.h>
#include <stdio.h>

// ============================================================================

typedef struct ZRMPoolDynamicStrategyS ZRMPoolDynamicStrategy;
typedef struct ZRAreaMetaDataS ZRAreaMetaData;
typedef struct ZRMPoolDSS ZRMPoolDS;
typedef struct ZRMPoolDS_bucketS ZRMPoolDS_bucket;

// ============================================================================

struct ZRMPoolDS_bucketS
{
	ZRMemoryPool *pool;

	size_t areaMetaDataOffset;
};

// ============================================================================

struct ZRMPoolDynamicStrategyS
{
	ZRMemoryPoolStrategy strategy;

	/**
	 * (Optional)
	 */
	ZRVector* (*fcreateBuckets)(ZRMemoryPool*);

	/**
	 * (Optional)
	 */
	void (*fdestroyBuckets)(ZRVector *buckets);
};

typedef enum
{
	MPoolDSInfos_base,
	MPoolDSInfos_strategy,
	MPoolDSInfos_struct,
	MPOOLDSINFOS_NB,
} MPoolDSInfos;

// ============================================================================

struct ZRAreaMetaDataS
{
	ZRMemoryPool *pool;
};

struct ZRMPoolDSS
{
	ZRMemoryPool pool;

	size_t nbFreeBuckets;
	size_t initialBucketSize;
	size_t maxFreeBuckets;

	ZRAllocator *allocator;

	ZRVector *buckets;

	unsigned staticStrategy :1;
};

// ============================================================================

#define ZRMPOOLDS(pool) ((ZRMPoolDS*)(pool))
#define ZRMPOOL_STRATEGY(pool) ((ZRMPoolDynamicStrategy*)((pool)->strategy))

#define DEFAULT_MAX_FREE_BUCKETS 10
#define INITIAL_BUCKETS_SPACE 1024
#define INITIAL_BUCKET_SIZE   1024

// ============================================================================
// Internal functions

static
void MPoolDSInfos_make(ZRObjAlignInfos *out, bool staticStrategy)
{
	out[MPoolDSInfos_base] = ZRTYPE_OBJALIGNINFOS(ZRMPoolDS);
	out[MPoolDSInfos_strategy] = staticStrategy ? ZRTYPE_OBJALIGNINFOS(ZRMPoolDynamicStrategy) : ZROBJALIGNINFOS_DEF0();
	out[MPoolDSInfos_struct] = (ZRObjAlignInfos ) { };
	ZRSTRUCT_MAKEOFFSETS(MPOOLDSINFOS_NB - 1, out);
}

/**
 * Add a bucket enough large to store $nbBlocks blocks.
 */
inline static ZRMPoolDS_bucket* addBucket(ZRMemoryPool *pool, size_t nbBlocks)
{
	ZRMPoolDS *dspool = ZRMPOOLDS(pool);
	size_t const initialBucketSize = dspool->initialBucketSize;
	size_t const bucket_i = ZRVECTOR_NBOBJ(dspool->buckets);
	ZRMPoolDS_bucket bucket;

	memset(&bucket, 0, sizeof(bucket));

	size_t nbBlocksToAlloc = initialBucketSize;

	while (nbBlocks > nbBlocksToAlloc)
		nbBlocksToAlloc += initialBucketSize;

	ZRObjAlignInfos areaMetaDataInfos = { 0, alignof(ZRAreaMetaData), sizeof(ZRAreaMetaData) };

	bucket.pool = ZRMPoolReserve_create(pool->blockSize, alignof(max_align_t), nbBlocksToAlloc, &areaMetaDataInfos, dspool->allocator, ZRMPoolReserveMode_chunk);
	bucket.areaMetaDataOffset = areaMetaDataInfos.offset;

	dspool->nbFreeBuckets++;
	ZRVECTOR_ADD(dspool->buckets, &bucket);
	return ZRVECTOR_GET(dspool->buckets, bucket_i);
}

static inline void bucket_clean(ZRMPoolDS_bucket *bucket)
{
	ZRMPOOL_CLEAN(bucket->pool);
}

static inline void bucket_done(ZRMPoolDS_bucket *bucket)
{
	ZRMPOOL_DESTROY(bucket->pool);
}

static inline void bucket_done_p(void *bucket)
{
	bucket_done(bucket);
}

// ============================================================================
// MemoryPool functions

void finitPool(ZRMemoryPool *pool)
{
	size_t const psize = sizeof(void*) * 2;
	ZRMPoolDS *const dspool = ZRMPOOLDS(pool);
	dspool->nbFreeBuckets = 0;
	dspool->buckets = ZRMPOOL_STRATEGY(pool)->fcreateBuckets(pool);
	addBucket(pool, 0);
}

void fclean(ZRMemoryPool *pool)
{
	ZRMPoolDS *const dspool = ZRMPOOLDS(pool);
	size_t const nbBuckets = ZRVECTOR_NBOBJ(dspool->buckets);

	for (int i = 0; i < nbBuckets; i++)
		bucket_clean(ZRVECTOR_GET(dspool->buckets, i));
}

static size_t fareaNbBlocks(ZRMemoryPool *pool, void *firstBlock)
{
	ZRMPoolDS *const dspool = ZRMPOOLDS(pool);
	ZRMPoolDS_bucket *bucket = ZRVECTOR_GET(dspool->buckets, 0);

	ZRAreaMetaData areaMetaData;
	memcpy(&areaMetaData, ZRMPOOL_USERAREAMETADATA(bucket->pool, firstBlock), sizeof(ZRAreaMetaData));

	ZRMemoryPool *blockPool = areaMetaData.pool;
	return ZRMPOOL_AREANBBLOCKS(blockPool, firstBlock);
}

static void* fuserAreaMetaData(ZRMemoryPool *pool, void *firstBlock)
{
	ZRMPoolDS *const dspool = ZRMPOOLDS(pool);
	ZRMPoolDS_bucket *bucket = ZRVECTOR_GET(dspool->buckets, 0);
	return ZRMPOOL_USERAREAMETADATA(bucket->pool, firstBlock);
}

static inline void* freserveInBucket(ZRMemoryPool *pool, size_t nb, ZRMPoolDS_bucket *bucket)
{
	ZRMPoolDS *const dspool = ZRMPOOLDS(pool);
	size_t const lastPoolBlocks = bucket->pool->nbBlocks;
	void *ret = ZRMPOOL_RESERVE_NB(bucket->pool, nb);

	pool->nbBlocks += bucket->pool->nbBlocks - lastPoolBlocks;

	if (ret == NULL)
		return NULL ;

	if (lastPoolBlocks == 0)
		dspool->nbFreeBuckets--;

	ZRAreaMetaData areaHead = { .pool = bucket->pool };
	memcpy(ZRMPOOL_USERAREAMETADATA(bucket->pool, ret), &areaHead, sizeof(ZRAreaMetaData));
	return ret;
}

void* freserve(ZRMemoryPool *pool, size_t nb)
{
	ZRMPoolDS *const dspool = ZRMPOOLDS(pool);
	size_t const nbBuckets = ZRVECTOR_NBOBJ(dspool->buckets);

	for (int i = 0; i < nbBuckets; i++)
	{
		ZRMPoolDS_bucket *bucket = ZRVECTOR_GET(dspool->buckets, i);
		void *ret = freserveInBucket(pool, nb, bucket);

		if (ret == NULL)
			continue;

		return ret;
	}
	return freserveInBucket(pool, nb, addBucket(pool, nb));
}

static inline void* freleaseInBucket(ZRMPoolDS *dspool, ZRMPoolDS_bucket *bucket, size_t bucket_i, void *firstBlock, size_t nb)
{
	size_t const lastPoolBlocks = bucket->pool->nbBlocks;
	void *newFirstBlock = ZRMPOOL_RELEASE_NB(bucket->pool, firstBlock, nb);

	dspool->pool.nbBlocks -= lastPoolBlocks - bucket->pool->nbBlocks;

	if (bucket->pool->nbBlocks == 0)
	{
		// Clean the bucket
		if (dspool->nbFreeBuckets == dspool->maxFreeBuckets)
		{
			bucket_done(bucket);
			ZRVECTOR_DELETE(dspool->buckets, bucket_i);
			return NULL ;
		}
		dspool->nbFreeBuckets++;
	}
	return newFirstBlock;
}

void* frelease(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolDS *const dspool = ZRMPOOLDS(pool);
	ZRMPoolDS_bucket *bucket;

	bucket = ZRVECTOR_GET(dspool->buckets, 0);

	ZRAreaMetaData areaMetaData;
	memcpy(&areaMetaData, ZRMPOOL_USERAREAMETADATA(bucket->pool, firstBlock), sizeof(ZRAreaMetaData));

	ZRMemoryPool *const blockPool = areaMetaData.pool;
	size_t i, c;

// Search the bucket in the vector
	for (i = 0, c = ZRVECTOR_NBOBJ(dspool->buckets); i < c; i++)
	{
		bucket = ZRVECTOR_GET(dspool->buckets, i);

		if (blockPool == bucket->pool)
			goto end;
	}
	fprintf(stderr, "Dynamic pool %p can't find the pool of block %p\n", pool, firstBlock);
	exit(1);
	return NULL ;
	end:
	return freleaseInBucket(dspool, bucket, i, firstBlock, nb);
}

/**
 * Clean the memory used by the pool.
 * The pool MUST NOT be used after this call.
 */
void fdone(ZRMemoryPool *pool)
{
	ZRMPoolDS *const dspool = ZRMPOOLDS(pool);
	ZRMPOOL_STRATEGY(pool)->fdestroyBuckets(dspool->buckets);

	if (dspool->staticStrategy == 0)
		ZRFREE(dspool->allocator, pool->strategy);
}

// ============================================================================

static ZRVector* fcreateBuckets(ZRMemoryPool *pool)
{
	ZRMPoolDS *const dspool = ZRMPOOLDS(pool);
	alignas(max_align_t) char buffer[2048];
	ZRVector2SideStrategyInfos(buffer,
	INITIAL_BUCKETS_SPACE, INITIAL_BUCKETS_SPACE,
		ZRTYPE_SIZE_ALIGNMENT(ZRMPoolDS_bucket), dspool->allocator, false
		);

	if (dspool->staticStrategy)
		ZRVector2SideStrategyInfos_staticStrategy(buffer);

	return ZRVector2SideStrategy_new(buffer);
}

static void fdestroyBuckets(ZRVector *buckets)
{
	ZRARRAYOP_WALK(ZRARRAY_OON(buckets->array), bucket_done_p);
	ZRVector_destroy(buckets);
}

void ZRMPoolDSStrategy_init(ZRMemoryPoolStrategy *strategy)
{
	*(ZRMPoolDynamicStrategy*)strategy = (ZRMPoolDynamicStrategy ) { //
		.strategy = (ZRMemoryPoolStrategy ) { //
			.finit = finitPool, //
			.fdone = fdone, //
			.fclean = fclean, //
			.fareaNbBlocks = fareaNbBlocks, //
			.fuserAreaMetaData = fuserAreaMetaData, //
			.freserve = freserve, //
			.frelease = frelease, //
			},//
		.fcreateBuckets = fcreateBuckets, //
		.fdestroyBuckets = fdestroyBuckets, //
		};
}

void ZRMPoolDS_destroy(ZRMemoryPool *pool)
{
	ZRMPoolDS *const dspool = ZRMPOOLDS(pool);
	ZRAllocator *allocator = dspool->allocator;
	ZRMPOOL_DONE(pool);
	ZRFREE(allocator, pool);
}

// ============================================================================

typedef struct
{
	ZRObjAlignInfos infos[MPOOLDSINFOS_NB];
	size_t initialBucketSize;
	size_t maxFreeBuckets;
	ZRObjInfos objInfos;
	ZRAllocator *allocator;
	unsigned staticStrategy :1;
} ZRMPoolDSInitInfos;

ZRObjInfos ZRMPoolDSInfos_objInfos(void)
{
	ZRObjInfos ret = { ZRTYPE_ALIGNMENT_SIZE(ZRMPoolDSInitInfos) };
	return ret;
}

ZRMUSTINLINE
static inline void ZRMPoolDSInfos_validate(ZRMPoolDSInitInfos *initInfos)
{
	MPoolDSInfos_make(initInfos->infos, (bool)initInfos->staticStrategy);
}

void ZRMPoolDSInfos(void *infos_out, ZRObjInfos objInfos, ZRAllocator *allocator)
{
	ZRMPoolDSInitInfos *initInfos = (ZRMPoolDSInitInfos*)infos_out;
	*initInfos = (ZRMPoolDSInitInfos ) { //
		.initialBucketSize = INITIAL_BUCKET_SIZE,
		.maxFreeBuckets = DEFAULT_MAX_FREE_BUCKETS,
		.objInfos = objInfos,
		.allocator = allocator,
		};
	ZRMPoolDSInfos_validate(initInfos);
}

void ZRMPoolDSInfos_initialBucketSize(void *infos, size_t initialBucketSize)
{
	ZRMPoolDSInitInfos *initInfos = (ZRMPoolDSInitInfos*)infos;
	initInfos->initialBucketSize = initialBucketSize;
}

void ZRMPoolDSInfos_maxFreeBuckets(void *infos, size_t maxFreeBuckets)
{
	ZRMPoolDSInitInfos *initInfos = (ZRMPoolDSInitInfos*)infos;
	initInfos->maxFreeBuckets = maxFreeBuckets;
}

void ZRMPoolDSInfos_staticStrategy(void *infos_out)
{
	ZRMPoolDSInitInfos *initInfos = (ZRMPoolDSInitInfos*)infos_out;
	initInfos->staticStrategy = 1;
	ZRMPoolDSInfos_validate(initInfos);
}

void ZRMPoolDS_init(ZRMemoryPool *pool, void *initInfos_p)
{
	ZRMPoolDS *dspool = ZRMPOOLDS(pool);
	ZRMPoolDSInitInfos *initInfos = (ZRMPoolDSInitInfos*)initInfos_p;
	ZRMemoryPoolStrategy *strategy;

	if (initInfos->staticStrategy)
		strategy = ZRARRAYOP_GET(dspool, 1, initInfos->infos[MPoolDSInfos_strategy].offset);
	else
		strategy = ZRALLOC(initInfos->allocator, sizeof(ZRMPoolDynamicStrategy));

	*dspool = (ZRMPoolDS ) { //
		.pool = (ZRMemoryPool ) { //
			.strategy = strategy,
			},
		.initialBucketSize = initInfos->initialBucketSize,
		.maxFreeBuckets = initInfos->maxFreeBuckets,
		.allocator = initInfos->allocator,
		.staticStrategy = initInfos->staticStrategy,
		};
	ZRMPoolDSStrategy_init(strategy);

	ZRMPool_init(pool, initInfos->objInfos.size, initInfos->objInfos.alignment, strategy);
}

ZRMemoryPool* ZRMPoolDS_new(void *initInfos_p)
{
	ZRMPoolDSInitInfos *initInfos = (ZRMPoolDSInitInfos*)initInfos_p;
	ZRMemoryPool *pool = ZRALLOC(initInfos->allocator, initInfos->infos[MPoolDSInfos_struct].size);
	ZRMPoolDS_init(pool, initInfos);
	pool->strategy->fdestroy = ZRMPoolDS_destroy;
	return pool;
}

ZRMemoryPool* ZRMPoolDS_create(size_t initialBucketSize, size_t maxFreeBuckets, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	ZRMPoolDSInitInfos initInfos;
	ZRMPoolDSInfos(&initInfos, ZROBJINFOS_DEF(objAlignment, objSize), allocator);
	ZRMPoolDSInfos_maxFreeBuckets(&initInfos, maxFreeBuckets);
	ZRMPoolDSInfos_initialBucketSize(&initInfos, initialBucketSize);
	return ZRMPoolDS_new(&initInfos);
}

ZRMemoryPool* ZRMPoolDS_createBS(size_t initialBucketSize, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	return ZRMPoolDS_create(initialBucketSize, DEFAULT_MAX_FREE_BUCKETS, objSize, objAlignment, allocator);
}

ZRMemoryPool* ZRMPoolDS_createMaxFB(size_t maxFreeBuckets, size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	return ZRMPoolDS_create(INITIAL_BUCKET_SIZE, maxFreeBuckets, objSize, objAlignment, allocator);
}

ZRMemoryPool* ZRMPoolDS_createDefault(size_t objSize, size_t objAlignment, ZRAllocator *allocator)
{
	return ZRMPoolDS_create(INITIAL_BUCKET_SIZE, DEFAULT_MAX_FREE_BUCKETS, objSize, objAlignment, allocator);
}
