/**
 * @author zuri
 * @date lundi 25 novembre 2019, 23:34:25 (UTC+0100)
 */

#include <zrlib/base/struct.h>
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

#define ZRMPOOL_STRATEGY(POOL) ((ZRMPoolReserveStrategy*)((POOL)->strategy))

// ============================================================================
// LIST

typedef struct ZRMPoolRListS ZRMPoolRList;

#define ZRMPOOLRLIST_INFOS_NB 4

typedef enum
{
	ZRMPoolRListInfos_base, ZRMPoolRListInfos_nextUnused, ZRMPoolRListInfos_reserve, ZRMPoolRListInfos_struct
} ZRMPoolRListInfos;

struct ZRMPoolRListS
{
	ZRMemoryPool pool;
	ZRObjAlignInfos infos[ZRMPOOLRLIST_INFOS_NB];
	size_t nbBlocks;
//	ZRReserveNextUnused nextUnused[NBBLOCKS];
// char reserve[(NBBLOCKS) * (BLOCKSIZE)];
};

#define ZRMPOOLRLIST(POOL) ((ZRMPoolRList*)POOL)
#define ZRMPOOLRLIST_GET(POOL,FIELD) ((char*)POOL + ZRMPOOLRLIST(POOL)->infos[FIELD].offset)
#define ZRMPOOLRLIST_NEXTUNUSED(POOL) ((ZRReserveNextUnused*)ZRMPOOLRLIST_GET(POOL,ZRMPoolRListInfos_nextUnused))
#define ZRMPOOLRLIST_RESERVE(POOL) ZRMPOOLRLIST_GET(POOL,ZRMPoolRListInfos_reserve)

static void MPoolRListInfos(ZRObjAlignInfos *out, size_t blockSize, size_t blockAlignment, size_t nbObj)
{
	out[0] = (ZRObjAlignInfos ) { 0, alignof(ZRMPoolRList), sizeof(ZRMPoolRList) };
	out[1] = (ZRObjAlignInfos ) { 0, alignof(ZRReserveNextUnused), sizeof(ZRReserveNextUnused) * nbObj };
	out[2] = (ZRObjAlignInfos ) { 0, blockAlignment, blockSize * nbObj };
	out[3] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsets(ZRMPOOLRLIST_INFOS_NB - 1, out);
}

static void finitPool_list(ZRMemoryPool *pool)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	memset(ZRMPOOLRLIST_RESERVE(rlpool), __ (int)0, rlpool->nbBlocks * sizeof(ZRReserveNextUnused));
	memset(ZRMPOOLRLIST_NEXTUNUSED(rlpool), (int)0, rlpool->nbBlocks * pool->blockSize);
}

static void* freserve_list(ZRMemoryPool *pool, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);

	if (nb > rlpool->nbBlocks - pool->nbBlocks)
		return NULL ;

	size_t const offset = ZRReserveOpList_reserveFirstAvailables(ZRMPOOLRLIST_NEXTUNUSED(rlpool), sizeof(ZRReserveNextUnused), rlpool->nbBlocks, 0, nb);

	if (offset == SIZE_MAX)
		return NULL ;

	pool->nbBlocks += nb;
	return &ZRMPOOLRLIST_RESERVE(rlpool)[offset * pool->blockSize];
}

static void frelease_list(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);

	assert(nb <= rlpool->nbBlocks);
//	assert((char* )firstBlock >= (char* )data->reserve);
//	assert((char* )firstBlock < (char* )&data->reserve[data->nbBlocks * pool->blockSize]);

	size_t const pos = (size_t)((char*)firstBlock - (char*)ZRMPOOLRLIST_RESERVE(rlpool)) / pool->blockSize;
	ZRReserveOpList_releaseNb(ZRMPOOLRLIST_NEXTUNUSED(rlpool), sizeof(ZRReserveNextUnused), rlpool->nbBlocks, 0, pos, nb);
	pool->nbBlocks -= nb;
}

static bool favailablePos_list(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	return ZRRESERVEOPLIST_AVAILABLES(ZRMPOOLRLIST_NEXTUNUSED(rlpool), sizeof(ZRReserveNextUnused), 0, pos, nb);
}

static void* freservePos_list(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	ZRRESERVEOPLIST_RESERVENB(ZRMPOOLRLIST_NEXTUNUSED(rlpool), sizeof(ZRReserveNextUnused), rlpool->nbBlocks, 0, pos, nb);
	pool->nbBlocks += nb;
	return &ZRMPOOLRLIST_RESERVE(rlpool)[pos * pool->blockSize];
}

// ============================================================================
// BITS
typedef struct ZRMPoolRBitsS ZRMPoolRBits;

#define ZRMPOOLRBITS_INFOS_NB 4

typedef enum
{
	ZRMPoolRBitsInfos_base, ZRMPoolRBitsInfos_bits, ZRMPoolRBitsInfos_reserve, ZRMPoolRBitsInfos_struct
} ZRMPoolRBitsInfos;

struct ZRMPoolRBitsS
{
	ZRMemoryPool pool;
	ZRObjAlignInfos infos[ZRMPOOLRBITS_INFOS_NB];
	size_t nbZRBits;
//	ZRBits bits[NBZRBITS];
//	char reserve[RESERVESIZE];
};

#define ZRMPOOLRBITS(POOL) ((ZRMPoolRBits*)POOL)
#define ZRMPOOLRBITS_GET(POOL,FIELD) ((char*)POOL + ZRMPOOLRBITS(POOL)->infos[FIELD].offset)
#define ZRMPOOLRBITS_BITS(POOL) ((ZRReserveNextUnused*)ZRMPOOLRBITS_GET(POOL,ZRMPoolRBitsInfos_bits))
#define ZRMPOOLRBITS_RESERVE(POOL) ZRMPOOLRBITS_GET(POOL,ZRMPoolRBitsInfos_reserve)

static void MPoolRBitsInfos(ZRObjAlignInfos *out, size_t blockSize, size_t blockAlignment, size_t nbObj, size_t nbZRBits)
{
	out[0] = (ZRObjAlignInfos ) { 0, alignof(ZRMPoolRBits), sizeof(ZRMPoolRBits) };
	out[1] = (ZRObjAlignInfos ) { 0, alignof(ZRBits), sizeof(ZRBits) * nbZRBits };
	out[2] = (ZRObjAlignInfos ) { 0, blockAlignment, blockSize * nbObj };
	out[3] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsets(ZRMPOOLRBITS_INFOS_NB - 1, out);
}

static void finitPool_bits(ZRMemoryPool *pool)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	memset(ZRMPOOLRBITS_BITS(rbpool), (int)ZRRESERVEOPBITS_FULLEMPTY, rbpool->nbZRBits * sizeof(ZRBits));
}

static void* freserve_bits(ZRMemoryPool *pool, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	size_t const offset = ZRReserveOpBits_reserveFirstAvailables(ZRMPOOLRBITS_BITS(rbpool), rbpool->nbZRBits, nb);

	if (offset == SIZE_MAX)
		return NULL ;

	pool->nbBlocks += nb;
	return &ZRMPOOLRBITS_RESERVE(rbpool)[offset * pool->blockSize];
}

static void frelease_bits(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);

//	assert((char* )firstBlock >= data->reserve);
//	assert((char* )firstBlock < &data->reserve[data->reserveSize]);

	size_t const pos = (size_t)((char*)firstBlock - ZRMPOOLRBITS_RESERVE(rbpool)) / pool->blockSize;
	ZRReserveOpBits_releaseNb(ZRMPOOLRBITS_BITS(rbpool), pos, nb);
	pool->nbBlocks -= nb;
}

static bool favailablePos_bits(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	return ZRRESERVEOPBITS_AVAILABLES(ZRMPOOLRBITS_BITS(rbpool), pos, nb);
}

static void* freservePos_bits(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	ZRRESERVEOPBITS_RESERVENB(ZRMPOOLRBITS_BITS(rbpool), pos, nb);
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
