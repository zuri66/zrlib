/**
 * @author zuri
 * @date lundi 25 novembre 2019, 23:34:25 (UTC+0100)
 */

#include <zrlib/lib/init.h>
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

typedef struct ZRMPoolReserveStrategyS ZRMPoolReserveStrategy;

struct ZRMPoolReserveStrategyS
{
	ZRMemoryPoolStrategy strategy;
};
#define ZRMPOOL_STRATEGY(POOL) ((ZRMPoolReserveStrategy*)((POOL)->strategy))

typedef struct
{
	size_t nbArea;
	size_t nbBlocksTotal;
	size_t nbAvailables;
	size_t nbBlocksForAreaHead;

	size_t areaHeadSize;

	ZRObjAlignInfos areaMetaDataInfos;

	char *reserve;

	ZRAllocator *allocator;
} ZRMPoolCommonInfos;

typedef enum
{
	ZRMPoolRInfos_reserve = 0,
	ZRMPoolRInfos_strategy,
	ZRMPoolRInfos_struct,
} ZRMPoolRInfos;

// ============================================================================
// AREA HEAD

#define ZRAREA_GUARD_P ((void*)0xDEAD)
#define ZRAREA_HEAD_GUARD_SIZE sizeof(void*)
#define ZRAREA_HEAD_SIZE(infos) (infos->areaHeadSize)

typedef struct
{
	size_t nbBlocks;
} ZRAreaHead;

typedef enum
{
	ZRAreaHeadInfos_base = 0,
	ZRAreaHeadInfos_userMetaData,
	ZRAreaHeadInfos_struct,
	ZRAREAHEADINFOS_NB,
} ZRAreaHeadInfos;

ZRMUSTINLINE
static inline void ZRAreaHeadInfos_make(ZRObjAlignInfos *infos, ZRObjAlignInfos *userMetaData)
{
	infos[ZRAreaHeadInfos_base] = ZRTYPE_OBJALIGNINFOS(ZRAreaHead);
	infos[ZRAreaHeadInfos_userMetaData] = *userMetaData;
	infos[ZRAreaHeadInfos_struct] = ZROBJALIGNINFOS_DEF0();
	ZRStruct_makeOffsets(ZRAREAHEADINFOS_NB - 1, infos);

	*userMetaData = infos[ZRAreaHeadInfos_userMetaData];
}

ZRMUSTINLINE
static inline void ZRAreaHead_set(void *firstBlock, ZRMPoolCommonInfos *infos, ZRAreaHead *areaHead)
{
	memcpy((char*)firstBlock - sizeof(void*), (void*[] ) { ZRAREA_GUARD_P }, sizeof(void*));
	memcpy((char*)firstBlock - ZRAREA_HEAD_SIZE(infos), areaHead, sizeof(ZRAreaHead));
}

ZRMUSTINLINE
static inline void ZRAreaHead_cpyUserMetaData(void *firstBlock, void *lastFirstBlock, ZRMPoolCommonInfos *infos)
{
	size_t const uheadSize = ZRAREA_HEAD_SIZE(infos);
	memcpy(
		(char*)firstBlock ___ - uheadSize + infos->areaMetaDataInfos.offset,
		(char*)lastFirstBlock - uheadSize + infos->areaMetaDataInfos.offset,
		infos->areaMetaDataInfos.offset
		);
}

ZRMUSTINLINE
static inline void* ZRAreaHead_getP(void *firstBlock, size_t headSize)
{
	void *guard;
	memcpy(&guard, firstBlock - sizeof(void*), sizeof(void*));

	if (guard != ZRAREA_GUARD_P)
		return false;

	return (char*)firstBlock - headSize;
}

ZRMUSTINLINE
static inline bool ZRAreaHead_get(void *firstBlock, size_t headSize, ZRAreaHead *areaHead)
{
	void *guard;
	memcpy(&guard, firstBlock - sizeof(void*), sizeof(void*));

	if (guard != ZRAREA_GUARD_P)
		return false;

	memcpy(areaHead, firstBlock - headSize, sizeof(ZRAreaHead));
	return true;
}

ZRMUSTINLINE
static inline void ZRAreaHead_checkAndGet(ZRMemoryPool *pool, ZRMPoolCommonInfos *infos, void *firstBlock, ZRAreaHead *areaHead)
{
	if (!ZRAreaHead_get(firstBlock, ZRAREA_HEAD_SIZE(infos), areaHead))
	{
		fprintf(stderr, "The block %p seems not to be an area for the pool %p\n", firstBlock, pool);
		assert(false);
		exit(1);
	}
}

// ============================================================================

ZRMUSTINLINE
static inline void* fuserAreaMetaData(ZRMemoryPool *pool, ZRMPoolCommonInfos *infos, void *firstBlock)
{
	if (infos->areaMetaDataInfos.size == 0)
		return NULL ;

	return ZRARRAYOP_GET(ZRAreaHead_getP(firstBlock, ZRAREA_HEAD_SIZE(infos)), 1, infos->areaMetaDataInfos.offset);
}

ZRMUSTINLINE
static inline void* freserve(ZRMemoryPool *pool, ZRMPoolCommonInfos *infos, size_t nb, size_t offset)
{
	if (offset == SIZE_MAX)
		return NULL ;

	infos->nbArea++;
	pool->nbBlocks += nb;
	infos->nbAvailables -= nb;

	void *firstBlock = ZRARRAYOP_GET(infos->reserve, pool->blockSize, offset + infos->nbBlocksForAreaHead);
	ZRAreaHead areaHead = { .nbBlocks = nb };
	ZRAreaHead_set(firstBlock, infos, &areaHead);
	return firstBlock;
}

ZRMUSTINLINE
static inline void* frelease(ZRMemoryPool *pool, ZRMPoolCommonInfos *infos, void *firstBlock, size_t *nb_p, size_t *areaPos)
{
	void *newFirstBlock;
	size_t nb = *nb_p;
	bool releaseArea = nb == SIZE_MAX;
	assert(nb > 0);
	ZRAreaHead areaHead;

//	assert(nb <= rlpool->nbBlocks);

	ZRAreaHead_checkAndGet(pool, infos, firstBlock, &areaHead);

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
	{
		infos->nbArea--;
		newFirstBlock = NULL;
	}
// Remove the beginning of the area
	else
	{
		nb -= infos->nbBlocksForAreaHead;
		newFirstBlock = ZRARRAYOP_GET(firstBlock, pool->blockSize, nb);
		areaHead.nbBlocks -= nb;
		ZRAreaHead_set(newFirstBlock, infos, &areaHead);
		ZRAreaHead_cpyUserMetaData(newFirstBlock, firstBlock, infos);
	}
	*nb_p = nb;
	pool->nbBlocks -= nb;
	infos->nbAvailables += nb;
	return newFirstBlock;
}

ZRMUSTINLINE
static inline void fclean_common(ZRMemoryPool *pool, ZRMPoolCommonInfos *infos)
{
	pool->nbBlocks = 0;
	infos->nbArea = 0;
	infos->nbAvailables = infos->nbBlocksTotal;
}

ZRMUSTINLINE
static inline void fdone(ZRMemoryPool *pool, ZRMPoolCommonInfos *infos)
{
	ZRAllocator *allocator = infos->allocator;
}

ZRMUSTINLINE
static inline void fdestroy(ZRMemoryPool *pool, ZRMPoolCommonInfos *infos)
{
	ZRAllocator *allocator = infos->allocator;
	fdone(pool, infos);
	ZRFREE(allocator, pool);
}

// ============================================================================
// CHUNK

typedef struct ZRMPoolRChunkS ZRMPoolRChunk;

typedef enum
{
	ZRMPoolRChunkInfos_base = 0,
	ZRMPoolRChunkInfos_chunk,
	ZRMPoolRChunkInfos_reserve,
	ZRMPoolRChunkInfos_strategy,
	ZRMPoolRChunkInfos_struct,
	ZRMPOOLRCHUNKINFOS_NB,
} ZRMPoolRChunkInfos;

struct ZRMPoolRChunkS
{
	ZRMemoryPool pool;

	ZRMPoolCommonInfos infos;

	size_t nbChunks;
	ZRReserveMemoryChunk *chunks;
	ZRReserveMemoryChunk *firstChunk;
};

#define ZRMPOOLRCHUNK(POOL) ((ZRMPoolRChunk*)POOL)

ZRMUSTINLINE
static inline void ZRMPoolRChunkInfos_make(ZRObjAlignInfos *out, size_t blockSize, size_t blockAlignment, size_t nbObj, size_t *nbChunks_out)
{
	size_t const nbChunks = nbObj / 2 + 1;
	out[ZRMPoolRChunkInfos_base] = ZRTYPE_OBJALIGNINFOS(ZRMPoolRChunk);
	out[ZRMPoolRChunkInfos_chunk] = ZRTYPENB_OBJALIGNINFOS(ZRReserveMemoryChunk, nbChunks);
	out[ZRMPoolRChunkInfos_reserve] = (ZRObjAlignInfos ) { 0, blockAlignment, blockSize * nbObj };
	out[ZRMPoolRChunkInfos_strategy] = ZRTYPE_OBJALIGNINFOS(ZRMPoolReserveStrategy);
	out[ZRMPoolRChunkInfos_struct] = ZROBJALIGNINFOS_DEF0();
	ZRStruct_bestOffsetsPos(ZRMPOOLRCHUNKINFOS_NB - 1, out, 1);
	*nbChunks_out = nbChunks;
}

static void finitPool_chunk(ZRMemoryPool *pool)
{
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	memset(rcpool->infos.reserve, __ (int)0, rcpool->infos.nbBlocksTotal * pool->blockSize);
	ZRRESERVEOPCHUNK_INITARRAY(rcpool->chunks, rcpool->nbChunks);
	rcpool->firstChunk = rcpool->chunks;
	rcpool->firstChunk->nbFree = rcpool->infos.nbBlocksTotal;
}

static void* fuserAreaMetaData_chunk(ZRMemoryPool *pool, void *firstBlock)
{
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	return fuserAreaMetaData(pool, &rcpool->infos, firstBlock);
}

static size_t fareaNbBlocks_chunk(ZRMemoryPool *pool, void *firstBlock)
{
	ZRAreaHead areaHead;
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	ZRAreaHead_checkAndGet(pool, &rcpool->infos, firstBlock, &areaHead);
	return areaHead.nbBlocks - ZRMPOOLRCHUNK(pool)->infos.nbBlocksForAreaHead;
}

static void* freserve_chunk(ZRMemoryPool *pool, size_t nb)
{
	assert(nb > 0);
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	nb += rcpool->infos.nbBlocksForAreaHead;

	if (nb > rcpool->infos.nbBlocksTotal - pool->nbBlocks)
		return NULL ;

	size_t const offset = ZRRESERVEOPCHUNK_RESERVEFIRSTAVAILABLES(&rcpool->firstChunk, nb, ZRReserveOpChunk_init);
	return freserve(pool, &rcpool->infos, nb, offset);
}

static void* frelease_chunk(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	size_t areaPos;

	void *newFirstBlock = frelease(pool, &rcpool->infos, firstBlock, &nb, &areaPos);

	ZRReserveMemoryChunk *freeChunk = rcpool->chunks;
	size_t i, c;

	/* Get a free chunk in memory */
	for (i = 0, c = rcpool->nbChunks; i < c; i++)
	{
		if (ZRRESERVEOPCHUNK_INITED(freeChunk))
			break;

		freeChunk++;
	}
	assert(i < c);
	ZRRESERVEOPCHUNK_RELEASENB(&rcpool->firstChunk, freeChunk, rcpool->infos.nbBlocksTotal, areaPos, nb, ZRReserveOpChunk_init);
	return newFirstBlock;
}

static void fclean_chunk(ZRMemoryPool *pool)
{
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	ZRRESERVEOPCHUNK_INITARRAY(rcpool->chunks, rcpool->nbChunks);
	rcpool->firstChunk = rcpool->chunks;
	rcpool->firstChunk->nbFree = rcpool->infos.nbBlocksTotal;
	fclean_common(pool, &rcpool->infos);
}

static void fdone_chunk(ZRMemoryPool *pool)
{
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	fdone(pool, &rcpool->infos);
}

static void fdestroy_chunk(ZRMemoryPool *pool)
{
	ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
	fdestroy(pool, &rcpool->infos);
}

// ============================================================================
// LIST

typedef struct ZRMPoolRListS ZRMPoolRList;

typedef enum
{
	ZRMPoolRListInfos_base = 0,
	ZRMPoolRListInfos_nextUnused,
	ZRMPoolRListInfos_reserve,
	ZRMPoolRListInfos_strategy,
	ZRMPoolRListInfos_struct,
	ZRMPOOLRLISTINFOS_NB,
} ZRMPoolRListInfos;

struct ZRMPoolRListS
{
	ZRMemoryPool pool;

	ZRMPoolCommonInfos infos;

	ZRReserveNextUnused *nextUnused;
};

#define ZRMPOOLRLIST(POOL) ((ZRMPoolRList*)POOL)

ZRMUSTINLINE
static inline void ZRMPoolRListInfos_make(ZRObjAlignInfos *out, size_t blockSize, size_t blockAlignment, size_t nbObj)
{
	out[ZRMPoolRListInfos_base] = ZRTYPE_OBJALIGNINFOS(ZRMPoolRList);
	out[ZRMPoolRListInfos_nextUnused] = ZRTYPENB_OBJALIGNINFOS(ZRReserveNextUnused, nbObj);
	out[ZRMPoolRListInfos_reserve] = ZROBJALIGNINFOS_DEF_AS(blockAlignment, blockSize * nbObj);
	out[ZRMPoolRListInfos_strategy] = ZRTYPE_OBJALIGNINFOS(ZRMPoolReserveStrategy);
	out[ZRMPoolRListInfos_struct] = ZROBJALIGNINFOS_DEF0();
	ZRStruct_bestOffsetsPos(ZRMPOOLRLISTINFOS_NB - 1, out, 1);
}

static void finitPool_list(ZRMemoryPool *pool)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	memset(rlpool->infos.reserve, __ (int)0, rlpool->infos.nbBlocksTotal * pool->blockSize);
	ZRRESERVEOPLIST_INITARRAY(rlpool->nextUnused, rlpool->infos.nbBlocksTotal);
}

static size_t fareaNbBlocks_list(ZRMemoryPool *pool, void *firstBlock)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	ZRAreaHead areaHead;
	ZRAreaHead_checkAndGet(pool, &rlpool->infos, firstBlock, &areaHead);
	return areaHead.nbBlocks - ZRMPOOLRLIST(pool)->infos.nbBlocksForAreaHead;
}

static void* fuserAreaMetaData_list(ZRMemoryPool *pool, void *firstBlock)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	return fuserAreaMetaData(pool, &rlpool->infos, firstBlock);
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

static void* frelease_list(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	size_t areaPos;
	void *newFirstBlock = frelease(pool, &rlpool->infos, firstBlock, &nb, &areaPos);
	ZRRESERVEOPLIST_RELEASENB(rlpool->nextUnused, sizeof(ZRReserveNextUnused), rlpool->infos.nbBlocksTotal, 0, areaPos, nb);
	return newFirstBlock;
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

static void fdone_list(ZRMemoryPool *pool)
{
	ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
	fdone(pool, &rlpool->infos);
}

static void fdestroy_list(ZRMemoryPool *pool)
{
	ZRMPoolRChunk *rlpool = ZRMPOOLRCHUNK(pool);
	fdestroy(pool, &rlpool->infos);
}

// ============================================================================
// BITS
typedef struct ZRMPoolRBitsS ZRMPoolRBits;

typedef enum
{
	ZRMPoolRBitsInfos_base = 0,
	ZRMPoolRBitsInfos_bits,
	ZRMPoolRBitsInfos_reserve,
	ZRMPoolRBitsInfos_strategy,
	ZRMPoolRBitsInfos_struct,
	ZRMPOOLRBITSINFOS_NB,
} ZRMPoolRBitsInfos;

struct ZRMPoolRBitsS
{
	ZRMemoryPool pool;

	ZRMPoolCommonInfos infos;
	size_t nbZRBits;

	ZRBits *bits;
};

#define ZRMPOOLRBITS(POOL) ((ZRMPoolRBits*)POOL)

ZRMUSTINLINE
static inline void ZRMPoolRBitsInfos_make(ZRObjAlignInfos *out, size_t blockSize, size_t blockAlignment, size_t nbObj, size_t *nbZRBits_out)
{
	size_t const nbZRBits = nbObj / ZRBITS_NBOF + ((nbObj % ZRBITS_NBOF) ? 1 : 0);
	out[ZRMPoolRBitsInfos_base] = ZRTYPE_OBJALIGNINFOS(ZRMPoolRBits);
	out[ZRMPoolRBitsInfos_bits] = ZRTYPENB_OBJALIGNINFOS(ZRBits, nbZRBits);
	out[ZRMPoolRBitsInfos_reserve] = ZROBJALIGNINFOS_DEF_AS(blockAlignment, blockSize * nbObj);
	out[ZRMPoolRBitsInfos_strategy] = ZRTYPE_OBJALIGNINFOS(ZRMPoolReserveStrategy);
	out[ZRMPoolRBitsInfos_struct] = ZROBJALIGNINFOS_DEF0();
	ZRStruct_bestOffsetsPos(ZRMPOOLRBITSINFOS_NB - 1, out, 1);
	*nbZRBits_out = nbZRBits;
}

static void finitPool_bits(ZRMemoryPool *pool)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	memset(rbpool->bits, (int)ZRRESERVEOPBITS_FULLEMPTY, rbpool->nbZRBits * sizeof(ZRBits));
}

static void* fuserAreaMetaData_bits(ZRMemoryPool *pool, void *firstBlock)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	return fuserAreaMetaData(pool, &rbpool->infos, firstBlock);
}

static size_t fareaNbBlocks_bits(ZRMemoryPool *pool, void *firstBlock)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	ZRAreaHead areaHead;
	ZRAreaHead_checkAndGet(pool, &rbpool->infos, firstBlock, &areaHead);
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

static void* frelease_bits(ZRMemoryPool *pool, void *firstBlock, size_t nb)
{
	ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
	size_t areaPos;
	void *newFirstBlock = frelease(pool, &rbpool->infos, firstBlock, &nb, &areaPos);
	ZRRESERVEOPBITS_RELEASENB(rbpool->bits, areaPos, nb);
	return newFirstBlock;
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

static void fdone_bits(ZRMemoryPool *pool)
{
	ZRMPoolRList *rbpool = ZRMPOOLRLIST(pool);
	fdone(pool, &rbpool->infos);
}

static void fdestroy_bits(ZRMemoryPool *pool)
{
	ZRMPoolRChunk *rbpool = ZRMPOOLRCHUNK(pool);
	fdestroy(pool, &rbpool->infos);
}

// ============================================================================

typedef struct
{
	ZRObjAlignInfos infos[ZRMAX(ZRMPOOLRLISTINFOS_NB, ZRMPOOLRBITSINFOS_NB, ZRMPOOLRCHUNKINFOS_NB)];
	ZRObjAlignInfos areaHeadInfos[ZRAREAHEADINFOS_NB];
	ZRObjAlignInfos *rpoolInfos;

	ZRAllocator *allocator;

	ZRObjAlignInfos *areaMetaData;
	ZRObjInfos blockInfos;

	size_t areaHeadSize;
	size_t nbBlocksForAreaHead;
	size_t nbBlocks;
	size_t nbBlocks_real;

	union
	{
		size_t nbZRBits;
		size_t nbChunks;
	};
	enum ZRMPoolReserveModeE mode;

	unsigned staticStrategy :1;
	unsigned changefdestroy :1;
} MPoolReserveInitInfos;

static void ZRMPoolReserveStrategy_init(ZRMPoolReserveStrategy *strategy, MPoolReserveInitInfos *infos)
{
	switch (infos->mode)
	{
	case ZRMPoolReserveMode_bits:
		*strategy = (ZRMPoolReserveStrategy ) { //
			.strategy = (ZRMemoryPoolStrategy ) { //
				.finit = finitPool_bits, //
				.fdone = fdone_bits, //
				.fdestroy = infos->changefdestroy ? fdestroy_bits : fdone_bits,
				.fclean = fclean_bits, //
				.fareaNbBlocks = fareaNbBlocks_bits, //
				.fuserAreaMetaData = fuserAreaMetaData_bits, //
				.freserve = freserve_bits, //
				.frelease = frelease_bits, //
				} , //
			};
		break;
	case ZRMPoolReserveMode_list:
		*strategy = (ZRMPoolReserveStrategy ) { //
			.strategy = (ZRMemoryPoolStrategy ) { //
				.finit = finitPool_list, //
				.fdone = fdone_list, //
				.fdestroy = infos->changefdestroy ? fdestroy_list : fdone_list,
				.fclean = fclean_list, //
				.fareaNbBlocks = fareaNbBlocks_list, //
				.fuserAreaMetaData = fuserAreaMetaData_list, //
				.freserve = freserve_list, //
				.frelease = frelease_list, //
				} , //
			};
		break;
	case ZRMPoolReserveMode_chunk:
		*strategy = (ZRMPoolReserveStrategy ) { //
			.strategy = (ZRMemoryPoolStrategy ) { //
				.finit = finitPool_chunk, //
				.fdone = fdone_chunk, //
				.fdestroy = infos->changefdestroy ? fdestroy_chunk : fdone_chunk,
				.fclean = fclean_chunk, //
				.fareaNbBlocks = fareaNbBlocks_chunk, //
				.fuserAreaMetaData = fuserAreaMetaData_chunk, //
				.freserve = freserve_chunk, //
				.frelease = frelease_chunk, //
				} , //
			};
		break;
	}
}

// ============================================================================

ZRObjInfos ZRMPoolReserveInfos_objInfos(void)
{
	return ZRTYPE_OBJINFOS(MPoolReserveInitInfos);
}

ZRMUSTINLINE
static inline void ZRMPoolReserveInfos_validate(MPoolReserveInitInfos *infos)
{
	size_t const blockSize = infos->blockInfos.size;

	ZRAreaHeadInfos_make(infos->areaHeadInfos, infos->areaMetaData);

	size_t const headSize = infos->areaHeadInfos[ZRAreaHeadInfos_struct].size + ZRAREA_HEAD_GUARD_SIZE;
	size_t const nbBlocksForAreaHead = headSize / blockSize + (headSize % blockSize ? 1 : 0);

	// TODO: better strategy for area head ?
	size_t const nbAreaInSpace = infos->nbBlocks;

	infos->nbBlocks_real = infos->nbBlocks + nbBlocksForAreaHead * nbAreaInSpace;
	infos->nbBlocksForAreaHead = nbBlocksForAreaHead;
	infos->areaHeadSize = headSize;

	switch (infos->mode)
	{
	case ZRMPoolReserveMode_bits:
		ZRMPoolRBitsInfos_make(infos->infos, ZROBJINFOS_SIZE_ALIGNMENT(infos->blockInfos), infos->nbBlocks_real, &infos->nbZRBits);
		infos->rpoolInfos = &infos->infos[ZRMPoolRBitsInfos_reserve];
		break;

	case ZRMPoolReserveMode_list:
		ZRMPoolRListInfos_make(infos->infos, ZROBJINFOS_SIZE_ALIGNMENT(infos->blockInfos), infos->nbBlocks_real);
		infos->rpoolInfos = &infos->infos[ZRMPoolRListInfos_reserve];
		break;

	case ZRMPoolReserveMode_chunk:
		ZRMPoolRChunkInfos_make(infos->infos, ZROBJINFOS_SIZE_ALIGNMENT(infos->blockInfos), infos->nbBlocks_real, &infos->nbChunks);
		infos->rpoolInfos = &infos->infos[ZRMPoolRChunkInfos_reserve];
		break;
	}
}

static ZRObjAlignInfos NULLOBJ;

void ZRMPoolReserveInfos(void *infos, ZRObjInfos blockInfos, size_t nbBlocks, ZRAllocator *allocator)
{
	MPoolReserveInitInfos *initInfos = (MPoolReserveInitInfos*)infos;
	*initInfos = (MPoolReserveInitInfos ) { //
		.mode = ZRMPoolReserveMode_default,
		.blockInfos = blockInfos,
		.nbBlocks = nbBlocks,
		.allocator = allocator,
		.areaMetaData = &NULLOBJ,
		};
	ZRMPoolReserveInfos_validate(initInfos);
}

void ZRMPoolReserveInfos_areaMetaData(void *infos, ZRObjAlignInfos *areaMetaData)
{
	MPoolReserveInitInfos *initInfos = (MPoolReserveInitInfos*)infos;

	if (areaMetaData == NULL)
		initInfos->areaMetaData = &NULLOBJ;
	else
		initInfos->areaMetaData = areaMetaData;

	ZRMPoolReserveInfos_validate(initInfos);

}
void ZRMPoolReserveInfos_mode(void *infos, enum ZRMPoolReserveModeE mode)
{
	MPoolReserveInitInfos *initInfos = (MPoolReserveInitInfos*)infos;
	initInfos->mode = mode;
	ZRMPoolReserveInfos_validate(initInfos);
}

void ZRMPoolReserveInfos_staticStrategy(void *infos)
{
	MPoolReserveInitInfos *initInfos = (MPoolReserveInitInfos*)infos;
	initInfos->staticStrategy = 1;
}

ZRObjInfos ZRMPoolReserve_objInfos(void *infos)
{
	MPoolReserveInitInfos *initInfos = (MPoolReserveInitInfos*)infos;
	return ZROBJALIGNINFOS_CPYOBJINFOS(initInfos->rpoolInfos[ZRMPoolRInfos_struct]);
}

void ZRMPoolReserve_init(ZRMemoryPool *pool, void *infos_p)
{
	MPoolReserveInitInfos *initInfos = (MPoolReserveInitInfos*)infos_p;
	ZRMPoolCommonInfos *pinfos;
	void *reserve;

	switch (initInfos->mode)
	{
	case ZRMPoolReserveMode_bits:
		{
		ZRMPoolRBits *rbpool = ZRMPOOLRBITS(pool);
		pinfos = &rbpool->infos;

		rbpool->bits = ZRARRAYOP_GET(rbpool, 1, initInfos->infos[ZRMPoolRBitsInfos_bits].offset);
		reserve = ZRARRAYOP_GET(rbpool, 1, initInfos->infos[ZRMPoolRBitsInfos_reserve].offset);
		rbpool->nbZRBits = initInfos->nbZRBits;
		pool = &rbpool->pool;
		break;
	}
	case ZRMPoolReserveMode_list:
		{
		ZRMPoolRList *rlpool = ZRMPOOLRLIST(pool);
		pinfos = &rlpool->infos;

		rlpool->nextUnused = ZRARRAYOP_GET(rlpool, 1, initInfos->infos[ZRMPoolRListInfos_nextUnused].offset);
		reserve = ZRARRAYOP_GET(rlpool, 1, initInfos->infos[ZRMPoolRListInfos_reserve].offset);
		pool = &rlpool->pool;
		break;
	}
	case ZRMPoolReserveMode_chunk:
		{
		ZRMPoolRChunk *rcpool = ZRMPOOLRCHUNK(pool);
		pinfos = &rcpool->infos;

		rcpool->nbChunks = initInfos->nbChunks;
		rcpool->chunks = ZRARRAYOP_GET(rcpool, 1, initInfos->infos[ZRMPoolRChunkInfos_chunk].offset);
		reserve = ZRARRAYOP_GET(rcpool, 1, initInfos->infos[ZRMPoolRChunkInfos_reserve].offset);
		pool = &rcpool->pool;
		break;
	}
	}
	*pinfos = (ZRMPoolCommonInfos ) { //
		.areaMetaDataInfos = initInfos->areaHeadInfos[ZRAreaHeadInfos_userMetaData],
		.areaHeadSize = initInfos->areaHeadSize,
		.nbBlocksForAreaHead = initInfos->nbBlocksForAreaHead,
		.nbBlocksTotal = initInfos->nbBlocks_real,
		.nbAvailables = initInfos->nbBlocks_real,
		.allocator = initInfos->allocator,
		.reserve = reserve,
		};

	ZRMPoolReserveStrategy *strategy, ref;
	ZRMPoolReserveStrategy_init(&ref, initInfos);

	if (initInfos->staticStrategy)
	{
		strategy = ZRARRAYOP_GET(pool, 1, initInfos->rpoolInfos[ZRMPoolRInfos_strategy].offset);
		ZRPTYPE_CPY(strategy, &ref);
	}
	else
		strategy = zrlib_internPType(&ref);

	ZRMPOOL_INIT(pool, ZROBJINFOS_SIZE_ALIGNMENT(initInfos->blockInfos), &strategy->strategy);
}

ZRMemoryPool* ZRMPoolReserve_new(void *infos_p)
{
	MPoolReserveInitInfos *initInfos = (MPoolReserveInitInfos*)infos_p;
	ZRMemoryPool *pool = ZRALLOC(initInfos->allocator, initInfos->rpoolInfos[ZRMPoolRInfos_struct].size);
	initInfos->changefdestroy = 1;
	ZRMPoolReserve_init(pool, infos_p);
	initInfos->changefdestroy = 0;
	return pool;
}

ZRMemoryPool* ZRMPoolReserve_create(
	size_t blockSize, size_t alignment, size_t nbBlocks,
	ZRObjAlignInfos *areaMetaData,
	ZRAllocator *allocator,
	enum ZRMPoolReserveModeE mode
	)
{
	MPoolReserveInitInfos infos;
	ZRMPoolReserveInfos(&infos, ZROBJINFOS_DEF(alignment, blockSize), nbBlocks, allocator);
	ZRMPoolReserveInfos_mode(&infos, mode);
	ZRMPoolReserveInfos_areaMetaData(&infos, areaMetaData);
	return ZRMPoolReserve_new(&infos);
}

ZRMemoryPool* ZRMPoolReserve_createSimple(
	size_t blockSize, size_t alignment, size_t nbBlocks,
	ZRAllocator *allocator, enum ZRMPoolReserveModeE mode
	)
{
	return ZRMPoolReserve_create(blockSize, alignment, nbBlocks, NULL, allocator, mode);
}
