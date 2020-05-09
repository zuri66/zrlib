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

	ZRAllocator *allocator;

	size_t initialBucketSize;

	size_t maxFreeBuckets;

	/**
	 * (Optional)
	 */
	ZRVector* (*fcreateBuckets)(ZRMemoryPool*);

	/**
	 * (Optional)
	 */
	void (*fdestroyBuckets)(ZRVector *buckets);
};

// ============================================================================

struct ZRAreaMetaDataS
{
	ZRMemoryPool *pool;
};

struct ZRMPoolDSS
{
	ZRMemoryPool pool;

	size_t nbFreeBuckets;

	ZRVector *buckets;
};

// ============================================================================

#define ZRMPOOLDS(pool) ((ZRMPoolDS*)(pool))
#define ZRMPOOL_STRATEGY(pool) ((ZRMPoolDynamicStrategy*)((pool)->strategy))

#define DEFAULT_MAX_FREE_BUCKETS 10
#define INITIAL_BUCKETS_SPACE 1024
#define INITIAL_BUCKET_SIZE   1024
#define INITIAL_BITS_SPACE ((1024*64)/ZRBITS_NBOF)
#define ZRGUARD_P ((void*)0XDEAD)

// ============================================================================
// Internal functions

/**
 * Add a bucket enough large to store $nbBlocks blocks.
 */
inline static ZRMPoolDS_bucket* addBucket(ZRMemoryPool *pool, size_t nbBlocks)
{
	ZRMPoolDS *dspool = ZRMPOOLDS(pool);
	ZRMPoolDynamicStrategy *strategy = ZRMPOOL_STRATEGY(pool);
	size_t const initialBucketSize = strategy->initialBucketSize;
	size_t const bucket_i = ZRVECTOR_NBOBJ(dspool->buckets);
	ZRMPoolDS_bucket bucket;

	memset(&bucket, 0, sizeof(bucket));

	size_t nbBlocksToAlloc = initialBucketSize;

	while (nbBlocks > nbBlocksToAlloc)
		nbBlocksToAlloc += initialBucketSize;

	ZRObjAlignInfos areaMetaDataInfos = { 0, alignof(ZRAreaMetaData), sizeof(ZRAreaMetaData) };

	bucket.pool = ZRMPoolReserve_create(pool->blockSize, alignof(max_align_t), nbBlocksToAlloc, &areaMetaDataInfos, strategy->allocator, ZRMPoolReserveMode_chunk);
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

size_t fstrategySize(void)
{
	return sizeof(ZRMPoolDynamicStrategy);
}

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
	size_t const nbBuckets = dspool->buckets->nbObj;

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
	size_t const nbBuckets = dspool->buckets->nbObj;

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
		if (dspool->nbFreeBuckets == ZRMPOOL_STRATEGY(&dspool->pool)->maxFreeBuckets)
		{
			bucket_done(bucket);
			ZRVECTOR_DELETE(dspool->buckets, bucket_i);
			return NULL;
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
	for (i = 0, c = dspool->buckets->nbObj; i < c; i++)
	{
		bucket = ZRVECTOR_GET(dspool->buckets, i);

		if (blockPool == bucket->pool)
			goto end;
	}
	fprintf(stderr, "Dynamic pool %p can't find the pool of block %p\n", pool, firstBlock);
	exit(1);
	return NULL;
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
}

// ============================================================================

static ZRVector* fcreateBuckets(ZRMemoryPool *pool)
{
	return ZRVector2SideStrategy_createDynamic(INITIAL_BUCKETS_SPACE, ZRTYPE_SIZE_ALIGNMENT(ZRMPoolDS_bucket), ZRMPOOL_STRATEGY(pool)->allocator);
}

static void fdestroyBuckets(ZRVector *buckets)
{
	ZRARRAYOP_WALK(buckets->array, buckets->objSize, buckets->nbObj, bucket_done_p);
	ZRVector_destroy(buckets);
}

void ZRMPoolDS_init(ZRMemoryPoolStrategy *strategy, ZRAllocator *allocator, size_t initialBucketSize, size_t maxFreeBuckets)
{
	*(ZRMPoolDynamicStrategy*)strategy = (ZRMPoolDynamicStrategy ) { //
		.strategy = (ZRMemoryPoolStrategy ) { //
			.fstrategySize = fstrategySize, //
			.finit = finitPool, //
			.fdone = fdone, //
			.fclean = fclean, //
			.fareaNbBlocks = fareaNbBlocks, //
			.fuserAreaMetaData = fuserAreaMetaData, //
			.freserve = freserve, //
			.frelease = frelease, //
			},//
		.allocator = allocator, //
		.initialBucketSize = initialBucketSize, //
		.maxFreeBuckets = maxFreeBuckets, //
		.fcreateBuckets = fcreateBuckets, //
		.fdestroyBuckets = fdestroyBuckets, //
		};
}

// ============================================================================

#include "MPoolDynamicStrategy_help.c"
