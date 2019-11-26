/**
 * @author zuri
 * @date lundi 25 novembre 2019, 23:34:25 (UTC+0100)
 */

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
typedef struct ZRMPoolReserveDataS ZRMPoolReserveData;
typedef struct ZRMPoolReserveDataExtendS ZRMPoolReserveDataExtend;

// ============================================================================

struct ZRMPoolReserveStrategyS
{
	ZRMemoryPoolStrategy strategy;

	ZRAllocator *allocator;
};

struct ZRMPoolReserveDataS
{
	size_t const reserveSize;
	size_t const nbZRBits;
	char reserve[];
};

// ============================================================================

#define TYPEDEF_SDATA_BITS(nbZRBits, reserveSize) typedef STRUCT_SDATA_BITS(nbZRBits, reserveSize) ZRMPoolReserveDataExtend
#define STRUCT_SDATA_BITS(NBZRBITS, RESERVESIZE) \
struct \
{ \
	size_t reserveSize; \
	size_t nbZRBits; \
	char reserve[RESERVESIZE]; \
	ZRBits bits[NBZRBITS]; \
}

#define TYPEDEF_SDATA_BITS_AUTO(pool) \
	size_t const nbZRBits = ZRMPOOL_DATA(pool)->nbZRBits; \
	size_t const reserveSize = ZRMPOOL_DATA(pool)->reserveSize; \
	TYPEDEF_SDATA_BITS(nbZRBits, reserveSize)

// ============================================================================

#define ZRMPOOL_DATA(pool) ((ZRMPoolReserveData*)((pool)->sdata))
#define ZRMPOOL_DATA_EXTEND(pool) ((ZRMPoolReserveDataExtend*)((pool)->sdata))
#define ZRMPOOL_STRATEGY(pool) ((ZRMPoolReserveStrategy*)((pool)->strategy))

// ============================================================================
// MemoryPool functions

static size_t fsdataSize(ZRMemoryPool *pool)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	return sizeof(ZRMPoolReserveDataExtend);
}

static size_t fstrategySize(void)
{
	return sizeof(ZRMPoolReserveStrategy);
}

static void finitPool_bits(ZRMemoryPool *pool)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	ZRMPoolReserveDataExtend *const data = ZRMPOOL_DATA_EXTEND(pool);
	memset(data->bits, (int)ZRRESERVEOPBITS_FULLEMPTY, reserveSize);
}

static void* freserve_bits(ZRMemoryPool *pool, size_t nb)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	ZRMPoolReserveDataExtend *const data = ZRMPOOL_DATA_EXTEND(pool);
	size_t const offset = ZRReserveOpBits_reserveFirstAvailables(data->bits, nbZRBits, nb);
	return data->reserve + offset * pool->blockSize;
}

static void frelease_bits(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	TYPEDEF_SDATA_BITS_AUTO(pool);
	ZRMPoolReserveDataExtend *const data = ZRMPOOL_DATA_EXTEND(pool);

	assert((char*)firstBlock >= data->reserve);
	assert((char*)firstBlock < ((char* )firstBlock + nb * pool->blockSize));

	size_t const pos = (size_t)((char*)firstBlock - data->reserve);
	ZRReserveOpBits_releaseNb(data->bits, pos, nb);
}

/**
 * Clean the memory used by the pool.
 * The pool MUST NOT be used after this call.
 */
static void fdone(ZRMemoryPool *pool)
{
}

// ============================================================================

void ZRMPoolReserve_init(ZRMemoryPoolStrategy *strategy, ZRAllocator *allocator, bool bitStrategy)
{
	if (bitStrategy)
		*(ZRMPoolReserveStrategy*)strategy = (ZRMPoolReserveStrategy ) { //
			.strategy = (ZRMemoryPoolStrategy ) { //
				.fsdataSize = fsdataSize, //
				.fstrategySize = fstrategySize, //
				.finit = finitPool_bits, //
				.fdone = fdone, //
				.freserve = freserve_bits, //
				.frelease = frelease_bits, //
				},//
			.allocator = allocator, //
			};
}

// ============================================================================

#include "MPoolReserve_help.c"

// ============================================================================
// MPoolReserve functions


