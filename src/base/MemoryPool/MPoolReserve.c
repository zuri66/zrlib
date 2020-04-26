/**
 * @author zuri
 * @date lundi 25 novembre 2019, 23:34:25 (UTC+0100)
 */

#include <zrlib/base/struct.h>
#include <zrlib/base/ReserveOp_list.h>
#include <zrlib/base/MemoryPool/MPoolReserve.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/ReserveOp_bits.h>
#include <zrlib/base/ReserveOp_chunk.h>
#include <zrlib/base/ReserveOp_list.h>

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

// ============================================================================

#define ZRMPOOL_STRATEGY(POOL) ((ZRMPoolReserveStrategy*)((POOL)->strategy))

typedef struct
{
	size_t nbArea;
	size_t nbBlocksTotal;
	size_t nbAvailables;
	size_t nbBlocksForAreaHead;
	char *reserve;
} ZRMPoolInfos;

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
	memcpy((char*)firstBlock - sizeof(void*), (void*[] ) { ZRAREA_GUARD_P }, sizeof(void*));
	memcpy((char*)firstBlock - ZRAREA_HEAD_SIZE, areaHead, sizeof(ZRAreaHead));
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
		fprintf(stderr, "The block %p seems not to be an area for the pool %p\n", firstBlock, pool);
		exit(1);
	}

	if ((void*)pool != areaHead->pool)
	{
		fprintf(stderr, "The block %p do not belongs to the pool %p\n", firstBlock, pool);
		exit(1);
	}
}

static ZRMemoryPool* fareaPool(ZRMemoryPool *pool, void *firstBlock)
{
	ZRAreaHead areaHead;

	if (!ZRAreaHead_get(firstBlock, &areaHead))
		return NULL ;

	return areaHead.pool;
}

static inline void* freserve(ZRMemoryPool *pool, ZRMPoolInfos *infos, size_t nb, size_t offset)
{
	if (offset == SIZE_MAX)
		return NULL ;

	infos->nbArea++;
	pool->nbBlocks += nb;
	infos->nbAvailables -= nb;

	void *firstBlock = ZRARRAYOP_GET(infos->reserve, pool->blockSize, offset + infos->nbBlocksForAreaHead);
	ZRAreaHead areaHead = { .nbBlocks = nb, .pool = pool };
	ZRAreaHead_set(firstBlock, &areaHead);
	return firstBlock;
}

static inline void frelease(ZRMemoryPool *pool, ZRMPoolInfos *infos, void *firstBlock, size_t *nb_p, size_t *areaPos)
{
	size_t nb = *nb_p;
	bool releaseArea = nb == SIZE_MAX;
	assert(nb > 0);
	ZRAreaHead areaHead;

//	assert(nb <= rlpool->nbBlocks);

	ZRAreaHead_checkAndGet(pool, firstBlock, &areaHead);

	if (releaseArea)
		nb = areaHead.nbBlocks;
	else
	{
		nb += infos->nbBlocksForAreaHead;
		assert(nb <= areaHead.nbBlocks);;
	}

	char *area = firstBlock - (infos->nbBlocksForAreaHead * pool->blockSize);
	*areaPos = (size_t)(area - infos->reserve) / pool->blockSize;

// Delete the guard value
	memset((char*)firstBlock - sizeof(void*), 0, sizeof(void*));

// Remove the entire area
	if (areaHead.nbBlocks == nb)
		infos->nbArea--;
// Remove the beginning of the area
	else
	{
		nb -= infos->nbBlocksForAreaHead;
		char *const newFirstBlock = ZRARRAYOP_GET(firstBlock, pool->blockSize, nb);
		areaHead.nbBlocks -= nb;
		ZRAreaHead_set(newFirstBlock, &areaHead);
	}
	*nb_p = nb;
	pool->nbBlocks -= nb;
	infos->nbAvailables += nb;
}

static void fclean_common(ZRMemoryPool *pool, ZRMPoolInfos *infos)
{
	pool->nbBlocks = 0;
	infos->nbArea = 0;
	infos->nbAvailables = infos->nbBlocksTotal;
}

// ============================================================================
// CHUNK

typedef struct ZRMPoolRChunkS ZRMPoolRChunk;

#define ZRMPOOLRCHUNK_INFOS_NB 4

typedef enum
{
	ZRMPoolRChunkInfos_base, ZRMPoolRChunkInfos_chunk, ZRMPoolRChunkInfos_reserve, ZRMPoolRChunkInfos_struct
} ZRMPoolRChunkInfos;

struct ZRMPoolRChunkS
{
	ZRMemoryPool pool;

	ZRMPoolInfos infos;

	size_t nbChunks;
	ZRReserveMemoryChunk *chunks;
	ZRReserveMemoryChunk *firstChunk;
};

#define ZRMPOOLRCHUNK(POOL) ((ZRMPoolRChunk*)POOL)

static void MPoolRChunkInfos(ZRObjAlignInfos *out, size_t blockSize, size_t blockAlignment, size_t nbObj, size_t nbChunks)
{
	out[0] = (ZRObjAlignInfos ) { 0, alignof(ZRMPoolRChunk), sizeof(ZRMPoolRChunk) };
	out[1] = (ZRObjAlignInfos ) { 0, alignof(ZRReserveMemoryChunk), sizeof(ZRReserveMemoryChunk) * nbChunks };
	out[2] = (ZRObjAlignInfos ) { 0, blockAlignment, blockSize * nbObj };
	out[3] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsetsPos(ZRMPOOLRCHUNK_INFOS_NB - 1, out, 1);
}

static void finitPool_chunk(ZRMemoryPool *pool)
{
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	memset(rcpool->infos.reserve, __ (int)0, rcpool->infos.nbBlocksTotal * pool->blockSize);
	ZRRESERVEOPCHUNK_INITARRAY(rcpool->chunks, rcpool->nbChunks);
	rcpool->firstChunk = rcpool->chunks;
	rcpool->firstChunk->nbFree = rcpool->infos.nbBlocksTotal;
}

static size_t fareaNbBlocks_chunk(ZRMemoryPool *pool, void *firstBlock)
{
	ZRAreaHead areaHead;
	ZRAreaHead_checkAndGet(pool, firstBlock, &areaHead);
	return areaHead.nbBlocks - ZRMPOOLRCHUNK(pool)->infos.nbBlocksForAreaHead;
}

static void* freserve_chunk(ZRMemoryPool *pool, size_t nb)
{
	assert(nb > 0);
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	nb += rcpool->infos.nbBlocksForAreaHead;

	if (nb > rcpool->infos.nbBlocksTotal - pool->nbBlocks)
		return NULL ;

	size_t const offset = ZRRESERVEOPCHUNK_RESERVEFIRSTAVAILABLES(&rcpool->firstChunk, nb, NULL);
	return freserve(pool, &rcpool->infos, nb, offset);
}

static void frelease_chunk(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	size_t areaPos;

	frelease(pool, &rcpool->infos, firstBlock, &nb, &areaPos);

	ZRReserveMemoryChunk *freeChunk = rcpool->chunks;
	size_t i, c;

	for (i = 0, c = rcpool->nbChunks; i < c; i++)
	{
		if (ZRRESERVEOPCHUNK_INITED(freeChunk))
			break;

		freeChunk++;
	}
	assert(i < c);
	ZRRESERVEOPCHUNK_RELEASENB(&rcpool->firstChunk, freeChunk, rcpool->infos.nbBlocksTotal, areaPos, nb, NULL);
}

static void fclean_chunk(ZRMemoryPool *pool)
{
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	ZRRESERVEOPCHUNK_INITARRAY(rcpool->chunks, rcpool->nbChunks);
	rcpool->firstChunk = rcpool->chunks;
	rcpool->firstChunk->nbFree = rcpool->infos.nbBlocksTotal;
	fclean_common(pool, &rcpool->infos);
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

	ZRMPoolInfos infos;

	ZRReserveNextUnused *nextUnused;
};

#define ZRMPOOLRLIST(POOL) ((ZRMPoolRList*)POOL)

static void MPoolRListInfos(ZRObjAlignInfos *out, size_t blockSize, size_t blockAlignment, size_t nbObj)
{
	out[0] = (ZRObjAlignInfos ) { 0, alignof(ZRMPoolRList), sizeof(ZRMPoolRList) };
	out[1] = (ZRObjAlignInfos ) { 0, alignof(ZRReserveNextUnused), sizeof(ZRReserveNextUnused) * nbObj };
	out[2] = (ZRObjAlignInfos ) { 0, blockAlignment, blockSize * nbObj };
	out[3] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsetsPos(ZRMPOOLRLIST_INFOS_NB - 1, out, 1);
}

static void finitPool_list(ZRMemoryPool *pool)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	memset(rlpool->infos.reserve, __ (int)0, rlpool->infos.nbBlocksTotal * pool->blockSize);
	ZRRESERVEOPLIST_INITARRAY(rlpool->nextUnused, rlpool->infos.nbBlocksTotal);
}

static size_t fareaNbBlocks_list(ZRMemoryPool *pool, void *firstBlock)
{
	ZRAreaHead areaHead;
	ZRAreaHead_checkAndGet(pool, firstBlock, &areaHead);
	return areaHead.nbBlocks - ZRMPOOLRLIST(pool)->infos.nbBlocksForAreaHead;
}

static void* freserve_list(ZRMemoryPool *pool, size_t nb)
{
	assert(nb > 0);
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	nb += rlpool->infos.nbBlocksForAreaHead;

	if (nb > rlpool->infos.nbBlocksTotal - pool->nbBlocks)
		return NULL ;

	size_t const offset = ZRRESERVEOPLIST_RESERVEFIRSTAVAILABLES(rlpool->nextUnused, sizeof(ZRReserveNextUnused), rlpool->infos.nbBlocksTotal, 0, nb);
	return freserve(pool, &rlpool->infos, nb, offset);
}

static void frelease_list(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	size_t areaPos;
	frelease(pool, &rlpool->infos, firstBlock, &nb, &areaPos);
	ZRRESERVEOPLIST_RELEASENB(rlpool->nextUnused, sizeof(ZRReserveNextUnused), rlpool->infos.nbBlocksTotal, 0, areaPos, nb);
}

static void fclean_list(ZRMemoryPool *pool)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	ZRRESERVEOPLIST_INITARRAY(rlpool->nextUnused, rlpool->infos.nbBlocksTotal);
	fclean_common(pool, &rlpool->infos);
}

static bool favailablePos_list(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	return ZRRESERVEOPLIST_AVAILABLES(rlpool->nextUnused, sizeof(ZRReserveNextUnused), 0, pos, nb);
}

static void* freservePos_list(ZRMemoryPool *pool, size_t pos, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	ZRRESERVEOPLIST_RESERVENB(rlpool->nextUnused, sizeof(ZRReserveNextUnused), rlpool->infos.nbBlocksTotal, 0, pos, nb);
	pool->nbBlocks += nb;
	return &rlpool->infos.reserve[pos * pool->blockSize];
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

	ZRMPoolInfos infos;
	size_t nbZRBits;

	ZRBits *bits;
};

#define ZRMPOOLRBITS(POOL) ((ZRMPoolRBits*)POOL)

static void MPoolRBitsInfos(ZRObjAlignInfos *out, size_t blockSize, size_t blockAlignment, size_t nbObj, size_t nbZRBits)
{
	out[0] = (ZRObjAlignInfos ) { 0, alignof(ZRMPoolRBits), sizeof(ZRMPoolRBits) };
	out[1] = (ZRObjAlignInfos ) { 0, alignof(ZRBits), sizeof(ZRBits) * nbZRBits };
	out[2] = (ZRObjAlignInfos ) { 0, blockAlignment, blockSize * nbObj };
	out[3] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsetsPos(ZRMPOOLRBITS_INFOS_NB - 1, out, 1);
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
	return areaHead.nbBlocks - ZRMPOOLRBITS(pool)->infos.nbBlocksForAreaHead;
}

static void* freserve_bits(ZRMemoryPool *pool, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	nb += rbpool->infos.nbBlocksForAreaHead;

	if (nb > rbpool->infos.nbBlocksTotal - pool->nbBlocks)
		return NULL ;

	size_t const offset = ZRRESERVEOPBITS_RESERVEFIRSTAVAILABLES(rbpool->bits, rbpool->nbZRBits, nb);
	return freserve(pool, &rbpool->infos, nb, offset);
}

static void frelease_bits(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	size_t areaPos;
	frelease(pool, &rbpool->infos, firstBlock, &nb, &areaPos);
	ZRRESERVEOPBITS_RELEASENB(rbpool->bits, areaPos, nb);
}

static void fclean_bits(ZRMemoryPool *pool)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	memset(rbpool->bits, (int)ZRRESERVEOPBITS_FULLEMPTY, rbpool->nbZRBits * sizeof(ZRBits));
	fclean_common(pool, &rbpool->infos);
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

void ZRMPoolReserveStrategy_init(ZRMemoryPoolStrategy *strategy, ZRAllocator *allocator, enum ZRMPoolReserveModeE mode)
{
	switch (mode)
	{
	case ZRMPoolReserveMode_bits:
		*(ZRMPoolReserveStrategy*)strategy = (ZRMPoolReserveStrategy ) { //
			.strategy = (ZRMemoryPoolStrategy ) { //
				.fstrategySize = fstrategySize, //
				.finit = finitPool_bits, //
				.fdone = fdone, //
				.fclean = fclean_bits, //
				.fareaNbBlocks = fareaNbBlocks_bits, //
				.fareaPool = fareaPool, //
				.freserve = freserve_bits, //
				.frelease = frelease_bits, //
				},//
			.allocator = allocator, //
			};
		break;
	case ZRMPoolReserveMode_list:
		*(ZRMPoolReserveStrategy*)strategy = (ZRMPoolReserveStrategy ) { //
			.strategy = (ZRMemoryPoolStrategy ) { //
				.fstrategySize = fstrategySize, //
				.finit = finitPool_list, //
				.fdone = fdone, //
				.fclean = fclean_list, //
				.fareaNbBlocks = fareaNbBlocks_list, //
				.fareaPool = fareaPool, //
				.freserve = freserve_list, //
				.frelease = frelease_list, //
				},//
			.allocator = allocator, //
			};
		break;
	case ZRMPoolReserveMode_chunk:
		*(ZRMPoolReserveStrategy*)strategy = (ZRMPoolReserveStrategy ) { //
			.strategy = (ZRMemoryPoolStrategy ) { //
				.fstrategySize = fstrategySize, //
				.finit = finitPool_chunk, //
				.fdone = fdone, //
				.fclean = fclean_chunk, //
				.fareaNbBlocks = fareaNbBlocks_chunk, //
				.fareaPool = fareaPool, //
				.freserve = freserve_chunk, //
				.frelease = frelease_chunk, //
				},//
			.allocator = allocator, //
			};
		break;
	}
}

// ============================================================================

#include "MPoolReserve_help.c"
