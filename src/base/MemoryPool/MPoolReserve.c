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
// AREA HEAD

#define ZRAREA_GUARD_P ((void*)0xDEAD)
#define ZRAREA_HEAD_SIZE (sizeof(ZRAreaHead) + sizeof(void*))

typedef struct
{
	void *pool;
	size_t nbBlocks;
} ZRAreaHead;

static inline void ZRAreaHead_set(void *firstBlock, ZRAreaHead *areaHead)
{
	memcpy(firstBlock - sizeof(void*), (void*[] ) { ZRAREA_GUARD_P }, sizeof(void*));
	memcpy(firstBlock - ZRAREA_HEAD_SIZE, areaHead, sizeof(ZRAreaHead));
}

static inline bool ZRAreaHead_get(void *firstBlock, ZRAreaHead *areaHead)
{
	void *guard;
	memcpy(&guard, firstBlock - sizeof(void*), sizeof(void*));

	if (guard != ZRAREA_GUARD_P)
		return false;

	memcpy(areaHead, firstBlock - ZRAREA_HEAD_SIZE, sizeof(ZRAreaHead));
	return true;
}

static inline void ZRAreaHead_checkAndGet(ZRMemoryPool *pool, void *firstBlock, ZRAreaHead *areaHead)
{
	if (!ZRAreaHead_get(firstBlock, areaHead))
	{
		fprintf(stderr, "The block %p seems not to be a area for the pool %p\n", firstBlock, pool);
		exit(1);
	}

	if ((void*)pool != areaHead->pool)
	{
		fprintf(stderr, "The block %p do not belongs to the pool %p\n", firstBlock, pool);
		exit(1);
	}
}

static inline void* freserve(ZRMemoryPool *pool, size_t nb, size_t nbBlocksForAreaHead, size_t offset, void *reserve, size_t *nbArea)
{
	if (offset == SIZE_MAX)
		return NULL ;

	nbArea++;
	pool->nbBlocks += nb;

	void *firstBlock = ZRARRAYOP_GET(reserve, pool->blockSize, offset + nbBlocksForAreaHead);
	ZRAreaHead areaHead = { .nbBlocks = nb, .pool = pool };
	ZRAreaHead_set(firstBlock, &areaHead);
	return firstBlock;
}

static inline void frelease(ZRMemoryPool *pool, void *firstBlock, size_t nb, size_t nbBlocksForAreaHead, void *reserve, size_t *nbArea, size_t *areaPos)
{
	assert(nb > 0);
	ZRAreaHead areaHead;
	nb += nbBlocksForAreaHead;

//	assert(nb <= rlpool->nbBlocks);

	ZRAreaHead_checkAndGet(pool, firstBlock, &areaHead);

	assert(nb <= areaHead.nbBlocks);

	char *area = firstBlock - (nbBlocksForAreaHead * pool->blockSize);
	*areaPos = (size_t)(area - (char*)reserve) / pool->blockSize;

// Delete the guard value
	memset((char*)firstBlock - sizeof(void*), 0, sizeof(void*));

// Remove the entire area
	if (areaHead.nbBlocks == nb)
	{
		nbArea--;
	}
// Remove the beginning of the area
	else
	{
		nb -= nbBlocksForAreaHead;
		char *const newFirstBlock = ZRARRAYOP_GET(firstBlock, pool->blockSize, nb);
		areaHead.nbBlocks -= nb;
		ZRAreaHead_set(newFirstBlock, &areaHead);
	}
}

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

	size_t nbArea;
	size_t nbBlocks;
	size_t nbBlocksForAreaHead;

	ZRReserveNextUnused *nextUnused;
	char *reserve;
};

#define ZRMPOOLRLIST(POOL) ((ZRMPoolRList*)POOL)

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
	memset(rlpool->nextUnused, (int)0, rlpool->nbBlocks * pool->blockSize);
	memset(rlpool->reserve, __ (int)0, rlpool->nbBlocks * sizeof(ZRReserveNextUnused));
}

static size_t fareaNbBlocks_list(ZRMemoryPool *pool, void *firstBlock)
{
	ZRAreaHead areaHead;
	ZRAreaHead_checkAndGet(pool, firstBlock, &areaHead);
	return areaHead.nbBlocks - ZRMPOOLRLIST(pool)->nbBlocksForAreaHead;
}

static void* freserve_list(ZRMemoryPool *pool, size_t nb)
{
	assert(nb > 0);
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	nb += rlpool->nbBlocksForAreaHead;

	if (nb > rlpool->nbBlocks - pool->nbBlocks)
		return NULL ;

	size_t const offset = ZRReserveOpList_reserveFirstAvailables(rlpool->nextUnused, sizeof(ZRReserveNextUnused), rlpool->nbBlocks, 0, nb);
	return freserve(pool, nb, rlpool->nbBlocksForAreaHead, offset, rlpool->reserve, &rlpool->nbArea);
}

static void frelease_list(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	size_t areaPos;
	frelease(pool, firstBlock, nb, rlpool->nbBlocksForAreaHead, rlpool->reserve, &rlpool->nbArea, &areaPos);
	ZRReserveOpList_releaseNb(rlpool->nextUnused, sizeof(ZRReserveNextUnused), rlpool->nbBlocks, 0, areaPos, nb);
}

static bool favailablePos_list(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	return ZRRESERVEOPLIST_AVAILABLES(rlpool->nextUnused, sizeof(ZRReserveNextUnused), 0, pos, nb);
}

static void* freservePos_list(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	ZRRESERVEOPLIST_RESERVENB(rlpool->nextUnused, sizeof(ZRReserveNextUnused), rlpool->nbBlocks, 0, pos, nb);
	pool->nbBlocks += nb;
	return &rlpool->reserve[pos * pool->blockSize];
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
	size_t nbZRBits;

	size_t nbArea;
	size_t nbBlocks;
	size_t nbBlocksForAreaHead;

	ZRBits *bits;
	char *reserve;
};

#define ZRMPOOLRBITS(POOL) ((ZRMPoolRBits*)POOL)

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
	memset(rbpool->bits, (int)ZRRESERVEOPBITS_FULLEMPTY, rbpool->nbZRBits * sizeof(ZRBits));
}

static size_t fareaNbBlocks_bits(ZRMemoryPool *pool, void *firstBlock)
{
	ZRAreaHead areaHead;
	ZRAreaHead_checkAndGet(pool, firstBlock, &areaHead);
	return areaHead.nbBlocks - ZRMPOOLRBITS(pool)->nbBlocksForAreaHead;
}

static void* freserve_bits(ZRMemoryPool *pool, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	nb += rbpool->nbBlocksForAreaHead;

	if (nb > rbpool->nbBlocks - pool->nbBlocks)
		return NULL ;

	size_t const offset = ZRReserveOpBits_reserveFirstAvailables(rbpool->bits, rbpool->nbZRBits, nb);
	return freserve(pool, nb, rbpool->nbBlocksForAreaHead, offset, rbpool->reserve, &rbpool->nbArea);
}

static void frelease_bits(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	size_t areaPos;
	frelease(pool, firstBlock, nb, rbpool->nbBlocksForAreaHead, rbpool->reserve, &rbpool->nbArea, &areaPos);
	ZRReserveOpBits_releaseNb(rbpool->bits, areaPos, nb);
}

static bool favailablePos_bits(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	return ZRRESERVEOPBITS_AVAILABLES(rbpool->bits, pos, nb);
}

static void* freservePos_bits(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	ZRRESERVEOPBITS_RESERVENB(rbpool->bits, pos, nb);
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
				.fareaNbBlocks = fareaNbBlocks_bits, //
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
				.fareaNbBlocks = fareaNbBlocks_list, //
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

// TODO: adapt with area head blocks

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
