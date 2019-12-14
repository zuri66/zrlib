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

// ============================================================================

#define ZRMPOOL_STRATEGY(pool) ((ZRMPoolReserveStrategy*)((pool)->strategy))

// ============================================================================
// LIST

typedef struct ZRMPoolReserveDataListHeadS ZRMPoolReserveDataListHead;

struct ZRMPoolReserveDataListHeadS
{
	size_t const nbBlocks;
};

#define ZRMPOOL_DATA_LIST_HEAD(pool) ((ZRMPoolReserveDataListHead*)((pool)->sdata))
#define ZRMPOOL_DATA_LIST(pool) ((ZRMPoolReserveDataList*)((pool)->sdata))

#define TYPEDEF_SDATA_LIST(BLOCKSIZE, NBBLOCKS) \
	typedef STRUCT_SDATA_LIST(BLOCKSIZE, NBBLOCKS) ZRMPoolReserveDataList
#define STRUCT_SDATA_LIST(BLOCKSIZE, NBBLOCKS) \
struct \
{ \
	size_t nbBlocks; \
	ZRReserveNextUnused nextUnused[NBBLOCKS]; \
	char reserve[(NBBLOCKS) * (BLOCKSIZE)]; \
}

#define TYPEDEF_SDATA_LIST_AUTO(pool) TYPEDEF_SDATA_LIST_ALL(pool->blockSize, ZRMPOOL_DATA_LIST_HEAD(pool)->nbBlocks)
#define TYPEDEF_SDATA_LIST_ALL(BLOCKSIZE, NBBLOCKS) \
	TYPEDEF_SDATA_LIST(BLOCKSIZE, NBBLOCKS)

static size_t fsdataSize_list(ZRMemoryPool *pool)
{
	TYPEDEF_SDATA_LIST_AUTO(pool);
	return sizeof(ZRMPoolReserveDataList);
}

static void finitPool_list(ZRMemoryPool *pool)
{
	TYPEDEF_SDATA_LIST_AUTO(pool);
	ZRMPoolReserveDataList *const data = ZRMPOOL_DATA_LIST(pool);
	memset((char*)data->nextUnused, (int)0, data->nbBlocks * sizeof(ZRReserveNextUnused));
	memset((char*)data->reserve, (int)0, data->nbBlocks * pool->blockSize);
}

static void* freserve_list(ZRMemoryPool *pool, size_t nb)
{
	TYPEDEF_SDATA_LIST_AUTO(pool);
	ZRMPoolReserveDataList *const data = ZRMPOOL_DATA_LIST(pool);

	if (nb > data->nbBlocks - pool->nbBlocks)
		return NULL ;

	size_t const offset = ZRReserveOpList_reserveFirstAvailables(data->nextUnused, sizeof(ZRReserveNextUnused), data->nbBlocks, 0, nb);

	if (offset == SIZE_MAX)
		return NULL ;

	pool->nbBlocks += nb;
	return &data->reserve[offset * pool->blockSize];
}

static void frelease_list(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	TYPEDEF_SDATA_LIST_AUTO(pool);
	ZRMPoolReserveDataList *const data = ZRMPOOL_DATA_LIST(pool);

	assert(nb <= data->nbBlocks);
	assert((char* )firstBlock >= (char* )data->reserve);
	assert((char* )firstBlock < (char* )&data->reserve[data->nbBlocks * pool->blockSize]);

	size_t const pos = (size_t)((char*)firstBlock - (char*)data->reserve) / pool->blockSize;
	ZRReserveOpList_releaseNb(data->nextUnused, sizeof(ZRReserveNextUnused), data->nbBlocks, 0, pos, nb);
	pool->nbBlocks -= nb;
}

static bool favailablePos_list(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	TYPEDEF_SDATA_LIST_AUTO(pool);
	ZRMPoolReserveDataList *const data = ZRMPOOL_DATA_LIST(pool);
	return ZRRESERVEOPLIST_AVAILABLES(data->nextUnused, sizeof(ZRReserveNextUnused), 0, pos, nb);
}

static void* freservePos_list(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	TYPEDEF_SDATA_LIST_AUTO(pool);
	ZRMPoolReserveDataList *const data = ZRMPOOL_DATA_LIST(pool);
	ZRRESERVEOPLIST_RESERVENB(data->nextUnused, sizeof(ZRReserveNextUnused), data->nbBlocks, 0, pos, nb);
	pool->nbBlocks += nb;
	return &data->reserve[pos * pool->blockSize];
}

// ============================================================================
// BITS

typedef struct ZRMPoolReserveDataBitsHeadS ZRMPoolReserveDataBitsHead;
typedef struct ZRMPoolReserveDataBitsS ZRMPoolReserveDataBits;

struct ZRMPoolReserveDataBitsHeadS
{
	size_t const reserveSize;
	size_t const nbZRBits;
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

	if (offset == SIZE_MAX)
		return NULL ;

	pool->nbBlocks += nb;
	return &data->reserve[offset * pool->blockSize];
}

static void frelease_bits(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	ZRMPoolReserveDataBits *const data = ZRMPOOL_DATA_BITS(pool);

	assert((char* )firstBlock >= data->reserve);
	assert((char* )firstBlock < &data->reserve[data->reserveSize]);

	size_t const pos = (size_t)((char*)firstBlock - data->reserve) / pool->blockSize;
	ZRReserveOpBits_releaseNb(data->bits, pos, nb);
	pool->nbBlocks -= nb;
}

static bool favailablePos_bits(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	ZRMPoolReserveDataBits *const data = ZRMPOOL_DATA_BITS(pool);
	return ZRRESERVEOPBITS_AVAILABLES(data->bits, pos, nb);
}

static void* freservePos_bits(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	ZRMPoolReserveDataBits *const data = ZRMPOOL_DATA_BITS(pool);
	ZRRESERVEOPBITS_RESERVENB(data->bits, pos, nb);
	pool->nbBlocks += nb;
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
			.favailablePos = favailablePos_list, //
			.freservePos = freservePos_list, //
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
			.favailablePos = favailablePos_bits, //
			.freservePos = freservePos_bits, //
			};
}

// ============================================================================

#include "MPoolReserve_help.c"

bool ZRMPoolReserve_availablePos_nb(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	return ZRMPOOLRESERVE_AVAILABLEPOS_NB(pool, pos, nb);
}

bool ZRMPoolReserve_availablePos(ZRMemoryPool *pool, size_t pos)
{
	return ZRMPOOLRESERVE_AVAILABLEPOS(pool, pos);
}

void* ZRMPoolReserve_reservePos_nb(ZRMemoryPool *pool, size_t pos, size_t nb, bool checkAvailability)
{
	return ZRMPOOLRESERVE_RESERVEPOS_NB(pool, pos, nb, checkAvailability);
}

void* ZRMPoolReserve_reservePos(ZRMemoryPool *pool, size_t pos, bool checkAvailability)
{
	return ZRMPOOLRESERVE_RESERVEPOS(pool, pos, checkAvailability);
}
