/**
 * @author zuri
 * @date mardi 29 octobre 2019, 19:39:20 (UTC+0100)
 */

#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Bits.h>
#include <zrlib/base/MemoryPool/MPoolDynamicStrategy.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/ArrayOp.h>

#include <stddef.h>
#include <stdio.h>

// ============================================================================

typedef struct ZRMPoolDynamicStrategyS ZRMPoolDynamicStrategy;
typedef struct ZRMPoolDynamicDataS ZRMPoolDynamicData;
typedef struct ZRMPoolDS_bucketS ZRMPoolDS_bucket;

// ============================================================================

struct ZRMPoolDS_bucketS
{
	size_t nbBlock;
	size_t nbAvailable;
	size_t nbZRBits;

	/**
	 * A sequence of bit which each inform of the availability of the block.
	 * 1 = empty, 0 = locked
	 */
	ZRBits *bits;

	/**
	 * The memory blocks and the bits.
	 *
	 * The first nbBlocks*pool->blockSize blocks are for the blocks, and the rest is the bits area.
	 */
	char blocks[];
};

// ============================================================================

struct ZRMPoolDynamicStrategyS
{
	ZRMemoryPoolStrategy strategy;

	ZRAllocator *allocator;

	size_t initialBucketSize;
};

// ============================================================================

struct ZRMPoolDynamicDataS
{
	size_t nbAvailables;

	size_t nbFreeBucket;

	ZRVector *buckets;
};

// ============================================================================

#define ZRMPOOL_DATA(pool) ((ZRMPoolDynamicData*)((pool)->sdata))
#define ZRMPOOL_STRATEGY(pool) ((ZRMPoolDynamicStrategy*)((pool)->strategy))

#define INITIAL_BUCKET_SPACE 1024
#define INITIAL_BITS_SPACE ((1024*64)/ZRBITS_NBOF)

#define BIT_BLOCKISEMPTY 1
#define BIT_BLOCKISLOCKD 0
#define BIT_FULLEMPTY ZRBITS_MASK_FULL
#define BIT_FULLLOCKD 0

#define ALIGNMENT 64

// ============================================================================
// Internal functions

/**
 * Add a bucket enough large to store $nbBlocks blocks.
 */
inline static ZRMPoolDS_bucket* addBucket(ZRMemoryPool *pool, size_t nbBlocks)
{
	ZRMPoolDynamicData *data = ZRMPOOL_DATA(pool);
	ZRMPoolDynamicStrategy *strategy = ZRMPOOL_STRATEGY(pool);
	size_t const initialBucketSize = strategy->initialBucketSize;
	size_t nbBlocksToAlloc = initialBucketSize;

	while (nbBlocks > nbBlocksToAlloc)
		nbBlocksToAlloc += initialBucketSize;

	size_t const blockSpace = nbBlocksToAlloc * pool->blockSize;
	size_t const nbZRBits = nbBlocksToAlloc / ZRBITS_NBOF + ((nbBlocksToAlloc % ZRBITS_NBOF) ? 1 : 0);
	size_t const alignment = sizeof(ZRBits);
	size_t const bitSpace = nbZRBits * sizeof(ZRBits) + alignment - 1;
	ZRMPoolDS_bucket *bucket = ZRALLOC(strategy->allocator, sizeof(ZRMPoolDS_bucket) + blockSpace + bitSpace);
	ZRVECTOR_ADD(data->buckets, &bucket);

	{
		char *bits = &bucket->blocks[blockSpace];
		size_t rest = (size_t)bits % alignment;

		if (rest > 0)
			bucket->bits = (ZRBits*)(bits + alignment - rest);
		else
			bucket->bits = (ZRBits*)(bits);
	}
	bucket->nbZRBits = nbZRBits;
	bucket->nbBlock = nbBlocksToAlloc;
	bucket->nbAvailable = nbBlocksToAlloc;
	pool->nbBlock += nbBlocksToAlloc;
	data->nbAvailables += nbBlocksToAlloc;
	data->nbFreeBucket++;

	ZRARRAYOP_FILL(bucket->bits, sizeof(ZRBits), nbZRBits, &((ZRBits ) { BIT_FULLEMPTY } ));
	return bucket;
}

// ============================================================================
// MemoryPool functions

size_t fsdataSize(ZRMemoryPool *pool)
{
	return 0;
}

size_t fstrategySize(void)
{
	return sizeof(ZRMPoolDynamicStrategy);
}

void finitPool(ZRMemoryPool *pool)
{
	ZRMPoolDynamicData * const data = ZRMPOOL_DATA(pool);
	data->nbFreeBucket = 0;
	data->nbAvailables = 0;
	data->buckets = ZRVector2SideStrategy_createDynamic(INITIAL_BUCKET_SPACE, sizeof(ZRMPoolDS_bucket*), ZRMPOOL_STRATEGY(pool)->allocator);
	addBucket(pool, 0);
}

void* freserveInBucket(ZRMemoryPool *pool, size_t nb, ZRMPoolDS_bucket *bucket)
{
	if (nb > bucket->nbAvailable)
		return NULL ;

	ZRMPoolDynamicData * const data = ZRMPOOL_DATA(pool);
	ZRBits *bits;
	size_t pos;

	ZRBits_searchFixedPattern(bucket->bits, 0, bucket->nbZRBits, nb, &bits, &pos);

	if (bits == NULL)
		return NULL ;

	ZRBits set[bucket->nbZRBits];
	memset(set, BIT_FULLLOCKD, bucket->nbZRBits * sizeof(ZRBits));
	ZRBits_copy(set, 0, nb, bits, pos);

	if (bucket->nbAvailable == bucket->nbBlock)
		data->nbFreeBucket--;

	bucket->nbAvailable -= nb;
	pos += (bits - bucket->bits) * ZRBITS_NBOF;
	return ZRARRAYOP_GET(bucket->blocks, pool->blockSize, pos);
}

void* freserve(ZRMemoryPool *pool, size_t nb)
{
	ZRMPoolDynamicData * const data = ZRMPOOL_DATA(pool);
	size_t const nbBuckets = data->buckets->nbObj;

	for (int i = 0; i < nbBuckets; i++)
	{
		ZRMPoolDS_bucket *bucket = *(ZRMPoolDS_bucket **)ZRVECTOR_GET(data->buckets, i);
		void *ret = freserveInBucket(pool, nb, bucket);

		if (ret == NULL)
			continue;

		return ret;
	}
	exit(1);
	return freserveInBucket(pool, nb, addBucket(pool, nb));
}

static inline bool freleaseInBucket(ZRMemoryPool *pool, ZRMPoolDS_bucket *bucket, void *firstBlock, size_t nb)
{
	ZRMPoolDynamicData * const data = ZRMPOOL_DATA(pool);

	if (bucket->blocks > (char*)firstBlock || (char*)firstBlock >= &bucket->blocks[pool->blockSize * bucket->nbBlock])
		return false;

	size_t blockPos = ((char*)firstBlock - bucket->blocks) / pool->blockSize;
	ZRBits set[bucket->nbZRBits];
	memset(set, BIT_FULLEMPTY, bucket->nbZRBits * sizeof(ZRBits));
	ZRBits_copy(set, 0, nb, bucket->bits, blockPos);
	bucket->nbAvailable += nb;

	if (bucket->nbAvailable == bucket->nbBlock)
		data->nbFreeBucket++;
}

void frelease(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolDynamicData * const data = ZRMPOOL_DATA(pool);
	size_t const nb_i = data->buckets->nbObj;

	for (size_t i = 0; i < nb_i; i++)
	{
		if (freleaseInBucket(pool, *(ZRMPoolDS_bucket**)ZRVECTOR_GET(data->buckets, i), firstBlock, nb))
			return;
	}
	fprintf(stderr, "The block %p does not belong to the pool %p", firstBlock, pool);
	exit(1);
}

/**
 * Clean the memory used by the pool.
 * The pool MUST NOT be used after this call.
 */
void fdone(ZRMemoryPool *pool)
{
	ZRMPoolDynamicData * const data = ZRMPOOL_DATA(pool);
	ZRAllocator * const allocator = ZRMPOOL_STRATEGY(pool)->allocator;
	ZRVector * const buckets = data->buckets;
	size_t const nbBuckets = ZRVECTOR_NBOBJ(buckets);

	for (size_t i = 0; i < nbBuckets; i++)
	{
		ZRMPoolDS_bucket * const bucket = *(ZRMPoolDS_bucket **)ZRVECTOR_GET(buckets, i);
		ZRFREE(allocator, bucket);
	}
	ZRVector2SideStrategy_destroy(buckets);
}

// ============================================================================

void ZRMPoolDS_init(ZRMemoryPoolStrategy *strategy, ZRAllocator *allocator, size_t initialBucketSize)
{
	*(ZRMPoolDynamicStrategy*)strategy = (ZRMPoolDynamicStrategy ) { //
		.strategy = (ZRMemoryPoolStrategy ) { //
			.fsdataSize = fsdataSize, //
			.fstrategySize = fstrategySize, //
			.finit = finitPool, //
			.fdone = fdone, //
			.freserve = freserve, //
			.frelease = frelease, //
			},//
		.allocator = allocator, //
		.initialBucketSize = initialBucketSize, //
		};
}

// ============================================================================

#include "MPoolDynamicStrategy_help.c"
