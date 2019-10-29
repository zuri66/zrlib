/**
 * @author zuri
 * @date mardi 29 octobre 2019, 19:39:20 (UTC+0100)
 */

#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Bits.h>
#include <zrlib/base/MemoryPool/MPoolDynamicStrategy.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <stddef.h>

// ============================================================================

typedef struct ZRMPoolDynamicStrategyS ZRMPoolDynamicStrategy;
typedef struct ZRMPoolDynamicDataS ZRMPoolDynamicData;
typedef struct ZRMPoolDS_bucketS ZRMPoolDS_bucket;

// ============================================================================

struct ZRMPoolDS_bucketS
{
	size_t nbBlock;
	size_t nbAvailable;
	ZRBits *bits;
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

	ZRVector *buckets;

	/**
	 * A sequence of bit which each inform of the availability of the block.
	 * 1 = empty, 0 = locked
	 */
	ZRVector *bits;
};

// ============================================================================

#define ZRMPOOL_DATA(pool) ((ZRMPoolDynamicData*)((pool)->sdata))
#define ZRMPOOL_STRATEGY(pool) ((ZRMPoolDynamicStrategy*)((pool)->strategy))

#define INITIAL_BUCKET_SPACE 1024
#define INITIAL_BITS_SPACE ((1024*64)/ZRBITS_NBOF)

#define BIT_BLOCKISEMPTY 1
#define BIT_BLOCKISLOCKD 0

// ============================================================================
// Internal functions

/**
 * Add a bucket enough large to store $nbBlocks blocks.
 */
inline static void addBucket(ZRMemoryPool *pool, size_t nbBlocks)
{
	ZRMPoolDynamicData *data = ZRMPOOL_DATA(pool);
	ZRMPoolDynamicStrategy *strategy = ZRMPOOL_STRATEGY(pool);
	size_t const initialBucketSize = strategy->initialBucketSize;
	size_t nbBlocksToAlloc = initialBucketSize;

	while (nbBlocks > nbBlocksToAlloc)
		nbBlocksToAlloc += initialBucketSize;

	ZRMPoolDS_bucket *bucket = ZRALLOC(strategy->allocator, sizeof(*bucket) + nbBlocksToAlloc);
	ZRVECTOR_ADD(data->buckets, &bucket);

	bucket->nbBlock = nbBlocksToAlloc;
	bucket->nbAvailable = nbBlocksToAlloc;
	pool->nbBlock += nbBlocksToAlloc;
	data->nbAvailables += nbBlocksToAlloc;

	size_t const nbZRBits = nbBlocksToAlloc / ZRBITS_NBOF;
	size_t const offsetBits = data->bits->nbObj;

	ZRVECTOR_FILL(data->bits, offsetBits, nbZRBits, &((ZRBits ) { BIT_BLOCKISEMPTY } ));
	bucket->bits = ZRVECTOR_GET(data->bits, offsetBits);
}

// ============================================================================
// MemoryPool functions

size_t fstrategySize(void)
{
	return sizeof(ZRMPoolDynamicStrategy);
}

void finitPool(ZRMemoryPool *pool)
{
	ZRMPoolDynamicData * const data = ZRMPOOL_DATA(pool);
	data->buckets = ZRVector2SideStrategy_createDynamic(INITIAL_BUCKET_SPACE, sizeof(ZRMPoolDS_bucket*), ZRMPOOL_STRATEGY(pool)->allocator);
	data->bits = ZRVector2SideStrategy_createDynamic(INITIAL_BITS_SPACE, sizeof(ZRBits), ZRMPOOL_STRATEGY(pool)->allocator);
	addBucket(pool, 0);
}

void* fget(ZRMemoryPool *pool, size_t nb)
{

}

void _frelease(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{

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
		ZRMPoolDS_bucket * const bucket = ZRVECTOR_GET(buckets, i);
		ZRFREE(allocator, bucket->blocks);
	}
	ZRVector2SideStrategy_destroy(buckets);
	ZRVector2SideStrategy_destroy(data->bits);
}
