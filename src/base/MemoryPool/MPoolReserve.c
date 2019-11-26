/**
 * @author zuri
 * @date lundi 25 novembre 2019, 23:34:25 (UTC+0100)
 */

#include <zrlib/base/ReserveOp_list.h>
#include <zrlib/base/MemoryPool/MPoolReserve.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/ReserveOp_bits.h>
#include <zrlib/base/ReserveOp_list.h>

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

// ============================================================================

typedef struct ZRMPoolReserveStrategyS ZRMPoolReserveStrategy;

// ============================================================================

struct ZRMPoolReserveStrategyS
{
	ZRMemoryPoolStrategy strategy;

	ZRAllocator *allocator;
};

// ============================================================================

#define ZRMPOOL_STRATEGY(pool) ((ZRMPoolReserveStrategy*)((pool)->strategy))

// ============================================================================
// LIST

typedef struct ZRMPoolReserveDataListHeadS ZRMPoolReserveDataListHead;

struct ZRMPoolReserveDataListHeadS
{
	size_t const reserveSize;
	size_t const nbBlocks;
};

#define ZRMPOOL_DATA_LIST_HEAD(pool) ((ZRMPoolReserveDataListHead*)((pool)->sdata))
#define ZRMPOOL_DATA_LIST(pool) ((ZRMPoolReserveDataList*)((pool)->sdata))

#define TYPEDEF_SDATA_LIST_BUCKET(BLOCKSIZE) typedef STRUCT_SDATA_LIST_BUCKET(BLOCKSIZE) ZRMPoolReserveBucketList
#define STRUCT_SDATA_LIST_BUCKET(BLOCKSIZE) \
struct{ \
	ZRReserveNextUnused nextUnused; \
	char block[BLOCKSIZE]; \
}

#define TYPEDEF_SDATA_LIST(NBBLOCKS) \
	typedef STRUCT_SDATA_LIST(NBBLOCKS) ZRMPoolReserveDataList
#define STRUCT_SDATA_LIST(NBBLOCKS) \
struct \
{ \
	size_t reserveSize; \
	size_t nbBlocks; \
	ZRMPoolReserveBucketList reserve[NBBLOCKS]; \
}

#define TYPEDEF_SDATA_LIST_AUTO(pool) TYPEDEF_SDATA_LIST_ALL(pool->blockSize, ZRMPOOL_DATA_LIST_HEAD(pool)->nbBlocks)
#define TYPEDEF_SDATA_LIST_ALL(BLOCKSIZE, NBBLOCKS) \
	TYPEDEF_SDATA_LIST_BUCKET(BLOCKSIZE); \
	TYPEDEF_SDATA_LIST(NBBLOCKS)

static size_t fsdataSize_list(ZRMemoryPool *pool)
{
	TYPEDEF_SDATA_LIST_AUTO(pool);
	return sizeof(ZRMPoolReserveDataList);
}

static void finitPool_list(ZRMemoryPool *pool)
{
	TYPEDEF_SDATA_LIST_AUTO(pool);
	ZRMPoolReserveDataList *const data = pool->sdata;
	memset((char*)data, (int)0, data->reserveSize);
}

static void* freserve_list(ZRMemoryPool *pool, size_t nb)
{
	TYPEDEF_SDATA_LIST_AUTO(pool);
	ZRMPoolReserveDataList *const data = ZRMPOOL_DATA_LIST(pool);
	size_t const offset = ZRReserveOpList_reserveFirstAvailables(data->reserve, sizeof(*data->reserve), data->nbBlocks, offsetof(ZRMPoolReserveBucketList, nextUnused), nb);
	pool->nbBlock += nb;
	return &data->reserve[offset];
}

static void frelease_list(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	TYPEDEF_SDATA_LIST_AUTO(pool);
	ZRMPoolReserveDataList *const data = ZRMPOOL_DATA_LIST(pool);

	assert((char*)firstBlock >= (char*)data->reserve);
	assert((char*)firstBlock < ((char*)firstBlock + nb * pool->blockSize));

	size_t const pos = (size_t)((char*)firstBlock - (char*)data->reserve);
	ZRReserveOpList_releaseNb(data->reserve, sizeof(*data->reserve), data->nbBlocks, offsetof(ZRMPoolReserveBucketList, nextUnused), pos, nb);
	pool->nbBlock -= nb;
}

// ============================================================================
// BITS

typedef struct ZRMPoolReserveDataBitsHeadS ZRMPoolReserveDataBitsHead;
typedef struct ZRMPoolReserveDataBitsS ZRMPoolReserveDataBits;

struct ZRMPoolReserveDataBitsHeadS
{
	size_t const reserveSize;
	size_t const nbZRBits;
	char reserve[];
};

#define ZRMPOOL_DATA_BITS_HEAD(pool) ((ZRMPoolReserveDataBitsHead*)((pool)->sdata))
#define ZRMPOOL_DATA_BITS(pool) ((ZRMPoolReserveDataBits*)((pool)->sdata))

#define TYPEDEF_SDATA_BITS(NBZRBITS, RESERVESIZE) typedef STRUCT_SDATA_BITS(NBZRBITS, RESERVESIZE) ZRMPoolReserveDataBits
#define STRUCT_SDATA_BITS(NBZRBITS, RESERVESIZE) \
struct \
{ \
	size_t reserveSize; \
	size_t nbZRBits; \
	char reserve[RESERVESIZE]; \
	ZRBits bits[NBZRBITS]; \
}

#define TYPEDEF_SDATA_BITS_AUTO(pool) \
	size_t const nbZRBits = ZRMPOOL_DATA_BITS_HEAD(pool)->nbZRBits; \
	size_t const reserveSize = ZRMPOOL_DATA_BITS_HEAD(pool)->reserveSize; \
	TYPEDEF_SDATA_BITS(nbZRBits, reserveSize)

static size_t fsdataSize_bits(ZRMemoryPool *pool)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	return sizeof(ZRMPoolReserveDataBits);
}

static void finitPool_bits(ZRMemoryPool *pool)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	ZRMPoolReserveDataBits *const data = ZRMPOOL_DATA_BITS(pool);
	memset(data->bits, (int)ZRRESERVEOPBITS_FULLEMPTY, nbZRBits * sizeof(ZRBits));
}

static void* freserve_bits(ZRMemoryPool *pool, size_t nb)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	ZRMPoolReserveDataBits *const data = ZRMPOOL_DATA_BITS(pool);
	size_t const offset = ZRReserveOpBits_reserveFirstAvailables(data->bits, nbZRBits, nb);
	pool->nbBlock += nb;
	return data->reserve + offset * pool->blockSize;
}

static void frelease_bits(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	ZRMPoolReserveDataBits *const data = ZRMPOOL_DATA_BITS(pool);

	assert((char*)firstBlock >= data->reserve);
	assert((char*)firstBlock < ((char* )firstBlock + nb * pool->blockSize));

	size_t const pos = (size_t)((char*)firstBlock - data->reserve);
	ZRReserveOpBits_releaseNb(data->bits, pos, nb);
	pool->nbBlock -= nb;
}

// ============================================================================

static size_t fstrategySize(void)
{
	return sizeof(ZRMPoolReserveStrategy);
}

/**
 * Clean the memory used by the pool.
 * The pool MUST NOT be used after this call.
 */
static void fdone(ZRMemoryPool *pool)
{
}

void ZRMPoolReserve_init(ZRMemoryPoolStrategy *strategy, ZRAllocator *allocator, bool bitStrategy)
{
	if (bitStrategy)
		*(ZRMPoolReserveStrategy*)strategy = (ZRMPoolReserveStrategy ) { //
			.strategy = (ZRMemoryPoolStrategy ) { //
				.fsdataSize = fsdataSize_bits, //
				.fstrategySize = fstrategySize, //
				.finit = finitPool_bits, //
				.fdone = fdone, //
				.freserve = freserve_bits, //
				.frelease = frelease_bits, //
				},//
			.allocator = allocator, //
			};
	else
		*(ZRMPoolReserveStrategy*)strategy = (ZRMPoolReserveStrategy ) { //
			.strategy = (ZRMemoryPoolStrategy ) { //
				.fsdataSize = fsdataSize_list, //
				.fstrategySize = fstrategySize, //
				.finit = finitPool_list, //
				.fdone = fdone, //
				.freserve = freserve_list, //
				.frelease = frelease_list, //
				},//
			.allocator = allocator, //
			};
}

// ============================================================================

#include "MPoolReserve_help.c"
