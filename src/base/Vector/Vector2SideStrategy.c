/**
 * @author zuri
 * @date samedi 19 janvier 2019, 18:03:00 (UTC+0100)
 */

#include <zrlib/lib/init.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Identifier/Identifier.h>
#include <zrlib/base/struct.h>

#include <assert.h>
#include <stdbool.h>
#include <stdalign.h>

#define INITIAL_SIZE 512

typedef struct ZRVector2SideStrategyS ZRVector2SideStrategy;
typedef struct ZR2SSVectorS ZR2SSVector;

// ============================================================================

struct ZRVector2SideStrategyS
{
	ZRVectorStrategy strategy;
};

#define ZR2SSSTRATEGY_VECTOR(S) (&(S)->strategy)

// ============================================================================

typedef enum
{
	ZRVectorInfos_base = 0,
	ZRVectorInfos_objs,
	ZRVectorInfos_strategy,
	ZRVectorInfos_struct,
	ZRVECTOR_INFOS_NB,
} ZRVectorInfos;

struct ZR2SSVectorS
{
	ZRVector vector;

	ZRResizeData resizeData;

	size_t initialArrayOffset;

	/*
	 * In bytes
	 */
	size_t initialArraySize;

	size_t initialMemoryNbObj;

	ZRAllocator *allocator;

	/*
	 * Memory segment, or unused first memory space
	 */
	void *restrict FSpace;

	/*
	 * Effectively used memory space
	 * Replaced by the 'array' field of ZRVector
	 */
	// void * restrict USpace;
	//
	/*
	 * Unused end memory space
	 */
	void *restrict ESpace;

	/*
	 * First address of outside memory segment
	 */
	void *restrict OSpace;

	unsigned char *allocatedMemory;
	unsigned char *initialArray;

	unsigned staticStrategy :1;
};

// ============================================================================

#define ZRDATA_INITIALARRAY(V) ((V)->initialArray)
#define ZRVECTOR_INITIALARRAY(V) ZRDATA_INITIALARRAY(ZRVECTOR_2SS(V))

ZRMUSTINLINE
static inline size_t checkInitialNbObj(size_t initialNbObj)
{
	size_t const minimumNbObj = 2;

	if (initialNbObj > 0 && initialNbObj < minimumNbObj)
		return minimumNbObj;

	return initialNbObj;
}

static void getVectorInfos(ZRObjAlignInfos *out, size_t initialArrayNbObj, size_t objSize, size_t objAlignment, bool staticStrategy)
{
	initialArrayNbObj = checkInitialNbObj(initialArrayNbObj);

	out[ZRVectorInfos_base] = (ZRObjAlignInfos ) { 0, ZRTYPE_ALIGNMENT_SIZE(ZR2SSVector) };
	out[ZRVectorInfos_objs] = (ZRObjAlignInfos ) { 0, objAlignment, objSize * initialArrayNbObj };
	out[ZRVectorInfos_strategy] = staticStrategy ? (ZRObjAlignInfos ) { 0, ZRTYPE_ALIGNMENT_SIZE(ZRVector2SideStrategy) } : (ZRObjAlignInfos ) { };
	out[ZRVectorInfos_struct] = (ZRObjAlignInfos ) { };
	ZRSTRUCT_MAKEOFFSETS(ZRVECTOR_INFOS_NB - 1, out);
}

// ============================================================================

#define ZR2SS_VECTOR(V) (&(V)->vector)
#define ZRVECTOR_2SS(V) ((ZR2SSVector*)(V))
#define ZR2SS_STRATEGY(V) ((ZRVector2SideStrategy*)(ZR2SS_VECTOR(V)->strategy))

#define ZR2SS_FSPACE(V) (V)->FSpace
#define ZR2SS_USPACE(V) ZR2SS_VECTOR(V)->array.array
#define ZR2SS_ESPACE(V) (V)->ESpace
#define ZR2SS_OSPACE(V) (V)->OSpace

#define ZR2SS_FSPACE_SIZEOF(V) ((char*)ZR2SS_USPACE(V) - (char*)ZR2SS_FSPACE(V))
#define ZR2SS_USPACE_SIZEOF(V) ((char*)ZR2SS_ESPACE(V) - (char*)ZR2SS_USPACE(V))
#define ZR2SS_ESPACE_SIZEOF(V) ((char*)ZR2SS_OSPACE(V) - (char*)ZR2SS_ESPACE(V))
#define ZR2SS_TOTALSPACE_SIZEOF(V) ((char*)ZR2SS_OSPACE(V) - (char*)ZR2SS_FSPACE(V))
#define ZR2SS_FREESPACE_SIZEOF(V) (ZR2SS_FSPACE_SIZEOF(V) + ZR2SS_ESPACE_SIZEOF(V))

// ============================================================================

static void finsert(ZRVector *vec, size_t pos, size_t nb);
static void fdelete(ZRVector *vec, size_t pos, size_t nb);

static void finsertGrow(ZRVector *vec, size_t pos, size_t nb);
static void fdeleteShrink(ZRVector *vec, size_t pos, size_t nb);

static inline void moreSize(ZRVector *vec, size_t nbObjMore);
static inline void setFUEOSpaces(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj);

// ============================================================================

enum ShiftModeE
{
	ShiftLeft = 1, ShiftRight = 2, ShiftLR = 3
};

ZRMUSTINLINE
static inline bool ZRVector2SideStrategy_memoryIsAllocated(ZR2SSVector *vec)
{
	return vec->allocatedMemory != NULL ;
}

ZRMUSTINLINE static inline void operationAdd_shift(ZRVector *vec, size_t pos, size_t nbMore, enum ShiftModeE shiftMode);
ZRMUSTINLINE static inline void operationDel_shift(ZRVector *vec, size_t pos, size_t nbLess, enum ShiftModeE shiftMode);

ZRMUSTINLINE
static inline bool mustGrow(ZRVector *vec, size_t nbObjMore)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	return ZRRESIZE_MUSTGROW(ZRVECTOR_CAPACITY(vec), ZRVECTOR_NBOBJ(vec) + nbObjMore, &svector->resizeData);
}

ZRMUSTINLINE
static inline bool mustShrink(ZRVector *vec)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	return ZRRESIZE_MUSTSHRINK(ZRVECTOR_CAPACITY(vec), ZRVECTOR_NBOBJ(vec), &svector->resizeData);
}

// ============================================================================

ZRMUSTINLINE
static inline void _moreSize(ZRVector *vec, size_t nbObjMore,
	void (*fsetFUEOSpaces)(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj)
	)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);
	size_t const nbObj = ZRVECTOR_NBOBJ(vec);
	size_t const capacity = ZRVECTOR_CAPACITY(vec);
	bool const isAllocated = ZRVector2SideStrategy_memoryIsAllocated(svector);

	ZRArrayAndNbObj new = ZRRESIZE_MAKEMORESIZE(
		capacity, nbObj + nbObjMore, objSize, ZRVECTOR_OBJALIGNMENT(vec),
		svector->allocatedMemory, svector->allocator,
		&svector->resizeData, svector
		);

	size_t const nextTotalNbObj = new.nbObj;
	fsetFUEOSpaces(svector, new.array, nextTotalNbObj, ZR2SS_USPACE(svector), nbObj);

	if (isAllocated)
		ZRFREE(svector->allocator, svector->allocatedMemory);

	svector->allocatedMemory = new.array;
	ZRVECTOR_CAPACITY(ZR2SS_VECTOR(svector)) = nextTotalNbObj;
}

ZRMUSTINLINE
static inline void _lessSize(ZRVector *vec,
	void (*fsetFUEOSpaces)(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj)
	)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	assert(ZRVector2SideStrategy_memoryIsAllocated(svector));
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);
	size_t const nbObj = ZRVECTOR_NBOBJ(vec);
	size_t const capacity = ZRVECTOR_CAPACITY(vec);

	ZRArrayAndNbObj new = ZRRESIZE_MAKELESSSIZE(
		capacity, nbObj, objSize, ZRVECTOR_OBJALIGNMENT(vec),
		svector->initialArray, svector->initialArraySize / objSize, svector->allocator,
		&svector->resizeData, svector
		);

	size_t const nextTotalNbObj = new.nbObj;
	fsetFUEOSpaces(svector, new.array, nextTotalNbObj, ZR2SS_USPACE(svector), nbObj);

	ZRFREE(svector->allocator, svector->allocatedMemory);

	if (new.array == svector->initialArray)
	{
		svector->allocatedMemory = NULL;
		svector->resizeData.downLimit = 0;
	}
	else
		svector->allocatedMemory = new.array;

	ZRVECTOR_CAPACITY(ZR2SS_VECTOR(svector)) = nextTotalNbObj;
}

ZRMUSTINLINE
static inline void _finsert(ZRVector *vec, size_t pos, size_t nb, enum ShiftModeE shiftMode)
{
	assert(ZR2SS_FREESPACE_SIZEOF(ZRVECTOR_2SS(vec)) >= nb * ZRVECTOR_OBJSIZE(vec));
	operationAdd_shift(vec, pos, nb, shiftMode);
	ZRVECTOR_NBOBJ(vec) += nb;
}

ZRMUSTINLINE
static inline void _finsertGrow(ZRVector *vec, size_t pos, size_t nb, enum ShiftModeE shiftMode,
	void (*fsetFUEOSpaces)(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj)
	)
{
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);

	if (mustGrow(vec, nb))
		_moreSize(vec, nb, fsetFUEOSpaces);

	_finsert(vec, pos, nb, shiftMode);
}

ZRMUSTINLINE
static inline void _fdelete(ZRVector *vec, size_t pos, size_t nb, enum ShiftModeE shiftMode)
{
	assert(ZR2SS_FSPACE(ZRVECTOR_2SS(vec)) != NULL);
	assert(nb <= ZRVECTOR_NBOBJ(vec));

	operationDel_shift(vec, pos, nb, shiftMode);
	ZRVECTOR_NBOBJ(vec) -= nb;
}

ZRMUSTINLINE
static inline void _fdeleteShrink(ZRVector *vec, size_t pos, size_t nb, enum ShiftModeE shiftMode,
	void (*fsetFUEOSpaces)(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj)
	)
{
	_fdelete(vec, pos, nb, shiftMode);

	if (mustShrink(vec))
		_lessSize(vec, fsetFUEOSpaces);
}

ZRMUSTINLINE
static inline void _fchangeObjSize(ZRVector *vector, ZRObjInfos objInfos,
	void (*fsetFUEOSpaces)(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj),
	void (*fmoreSize)(ZRVector *vec, size_t nbObjMore)
	)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vector);
	size_t const lastAlignment = ZRVECTOR_OBJALIGNMENT(vector);
	size_t const objAlignment = objInfos.alignment;
	size_t const objSize = objInfos.size;

	if (ZRVector2SideStrategy_memoryIsAllocated(svector))
	{
		ZRFREE(svector->allocator, svector->allocatedMemory);
		svector->allocatedMemory = NULL;
	}
	ZRVECTOR_NBOBJ(ZR2SS_VECTOR(svector)) = 0;
	ZRVECTOR_OBJINFOS(ZR2SS_VECTOR(svector)) = objInfos;

	//Realign initial
	if (svector->initialArraySize > 0)
	{
		if (objAlignment != lastAlignment)
		{
			// alignment + 1 obj Size
			if (objAlignment * 2 <= svector->initialArraySize)
			{
				size_t const offset = ZRSTRUCT_ALIGNOFFSET(svector->initialArrayOffset, objAlignment);
				size_t const offsetDiff = offset - svector->initialArrayOffset;
				svector->initialArray = ZRARRAYOP_GET(svector, 1, offset);
				size_t const size = (svector->initialArraySize - offsetDiff) / objSize;
				ZRVECTOR_CAPACITY(ZR2SS_VECTOR(svector)) = size;
				fsetFUEOSpaces(svector, svector->initialArray, size, NULL, 0);
			}
			else
			{
				ZR2SS_FSPACE(svector) =
				ZR2SS_USPACE(svector) =
				ZR2SS_OSPACE(svector) =
				ZR2SS_ESPACE(svector) = svector;
				fmoreSize(ZR2SS_VECTOR(svector), 0);
			}
		}
	}
	else
		fmoreSize(ZR2SS_VECTOR(svector), 0);
}

// ============================================================================

ZRMUSTINLINE
static inline void setFUEOSpaces_oneSide(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj)
{
	assert(fspaceNbObj >= sourceNbObj);
	size_t const objSize = ZRVECTOR_OBJSIZE(ZR2SS_VECTOR(svector));

	ZR2SS_USPACE(svector) =
	ZR2SS_FSPACE(svector) = fspace;
	ZR2SS_OSPACE(svector) = ZRARRAYOP_GET(ZR2SS_FSPACE(svector), objSize, fspaceNbObj);

	if (sourceNbObj == 0)
		ZR2SS_ESPACE(svector) = ZR2SS_USPACE(svector);
	else
	{
		ZR2SS_ESPACE(svector) = ZRARRAYOP_GET(ZR2SS_USPACE(svector), objSize, sourceNbObj);

		if (sourceNbObj > 0)
			ZRARRAYOP_CPY(ZR2SS_USPACE(svector), objSize, sourceNbObj, source);
	}
}

ZRMUSTINLINE
static inline void moreSize_oneSide(ZRVector *vec, size_t nbObjMore)
{
	_moreSize(vec, nbObjMore, setFUEOSpaces_oneSide);
}

/**
 * vec must be initialized to zero before.
 */
static void finitVec_oneSide(ZRVector *vec)
{
	ZR2SSVector *svector = ZRVECTOR_2SS(vec);

	if (svector->initialArraySize == 0)
	{
		// Must be set before moreSize()
		ZR2SS_FSPACE(svector) =
		ZR2SS_USPACE(svector) =
		ZR2SS_OSPACE(svector) =
		ZR2SS_ESPACE(svector) = svector;
		moreSize_oneSide(ZR2SS_VECTOR(svector), 0);
	}
	else
	{
		ZRVECTOR_CAPACITY(ZR2SS_VECTOR(svector)) = svector->initialArraySize / ZRVECTOR_OBJSIZE(vec);
		setFUEOSpaces_oneSide(svector, svector->initialArray, ZRVECTOR_CAPACITY(ZR2SS_VECTOR(svector)), NULL, 0);
	}
}

static void fchangeObjSize_oneSide(ZRVector *vector, ZRObjInfos objInfos)
{
	_fchangeObjSize(vector, objInfos, setFUEOSpaces_oneSide, moreSize_oneSide);
}

// ============================================================================

/**
 * vec must be initialized to zero before.
 */
static void finitVec(ZRVector *vec)
{
	ZR2SSVector *svector = ZRVECTOR_2SS(vec);

	if (svector->initialArraySize == 0)
	{
		/* Must be set before moreSize() */
		ZR2SS_FSPACE(svector) =
		ZR2SS_USPACE(svector) =
		ZR2SS_OSPACE(svector) =
		ZR2SS_ESPACE(svector) = svector;
		moreSize(ZR2SS_VECTOR(svector), 0);
	}
	else
	{
		ZRVECTOR_CAPACITY(ZR2SS_VECTOR(svector)) = svector->initialArraySize / ZRVECTOR_OBJSIZE(vec);
		setFUEOSpaces(svector, svector->initialArray, ZRVECTOR_CAPACITY(ZR2SS_VECTOR(svector)), NULL, 0);
	}
}

static void fchangeObjSize(ZRVector *vector, ZRObjInfos objInfos)
{
	_fchangeObjSize(vector, objInfos, setFUEOSpaces, moreSize);
}

void ZRVector2SideStrategy_growStrategy(ZRVector *vec, zrflimit fupLimit, zrfincrease fincrease)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	svector->resizeData.growStrategy = (ZRResizeGrowStrategy ) { fupLimit, fincrease };
}

void ZRVector2SideStrategy_shrinkStrategy(ZRVector *vec, zrflimit fdownLimit, zrfdecrease fdecrease)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	svector->resizeData.shrinkStrategy = (ZRResizeShrinkStrategy ) { fdownLimit, fdecrease };
}

static void fmemoryTrim(ZRVector *vec)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);

	if (!ZRVector2SideStrategy_memoryIsAllocated(svector))
		return;

	size_t const objSize = ZRVECTOR_OBJSIZE(vec);
	size_t const nbObj = checkInitialNbObj(ZRVECTOR_NBOBJ(vec));
	size_t const objAlignment = ZRVECTOR_OBJALIGNMENT(vec);
	void *lastSpace = svector->allocatedMemory;

	ZRArrayOp_deplace(ZR2SS_FSPACE(svector), objSize, nbObj, ZR2SS_USPACE(svector));
	svector->allocatedMemory = ZRAALLOC(svector->allocator, objAlignment, objSize * nbObj);
	ZRARRAYOP_CPY(svector->allocatedMemory, objSize, nbObj, lastSpace);
	ZRFREE(svector->allocator, lastSpace);

	ZR2SS_FSPACE(svector) =
	ZR2SS_USPACE(svector) = svector->allocatedMemory;
	ZR2SS_OSPACE(svector) =
	ZR2SS_ESPACE(svector) = (char*)svector->allocatedMemory + nbObj * objSize;
	ZRVECTOR_CAPACITY(ZR2SS_VECTOR(svector)) = nbObj;
}

// ============================================================================

ZRMUSTINLINE
static inline size_t getInitialMemoryNbObjs(size_t initialMemoryNbObj, size_t initialArrayNbObj, size_t objSize)
{
	if (initialMemoryNbObj > 0)
		return initialMemoryNbObj;

	if (initialArrayNbObj > 0)
		return initialArrayNbObj;

	return INITIAL_SIZE;
}

ZRMUSTINLINE
static inline void setFUEOSpaces(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj)
{
	assert(fspaceNbObj >= sourceNbObj);
	assert(fspaceNbObj >= 2);
	size_t const objSize = ZRVECTOR_OBJSIZE(ZR2SS_VECTOR(svector));

	ZR2SS_FSPACE(svector) = fspace;
	ZR2SS_OSPACE(svector) = ZRARRAYOP_GET(ZR2SS_FSPACE(svector), objSize, fspaceNbObj);

	if (sourceNbObj == 0)
	{
		ZR2SS_USPACE(svector) = ZRARRAYOP_GET(ZR2SS_FSPACE(svector), objSize, fspaceNbObj / 2);
		ZR2SS_ESPACE(svector) = ZR2SS_USPACE(svector);
	}
	else
	{
		ZR2SS_USPACE(svector) = ZRARRAYOP_GET(ZR2SS_FSPACE(svector), objSize, (fspaceNbObj - sourceNbObj) / 2);
		ZR2SS_ESPACE(svector) = ZRARRAYOP_GET(ZR2SS_USPACE(svector), objSize, sourceNbObj);

		if (sourceNbObj > 0)
			ZRARRAYOP_CPY(ZR2SS_USPACE(svector), objSize, sourceNbObj, source);
	}
}

ZRMUSTINLINE
static inline void centerFUEOSpaces(ZR2SSVector *svector, size_t nbMore)
{
	assert(nbMore > 0);
	size_t const objSize = ZRVECTOR_OBJSIZE(ZR2SS_VECTOR(svector));
	size_t const nbObj = ZRVECTOR_NBOBJ(ZR2SS_VECTOR(svector));
	size_t const newNbObj = nbObj + nbMore;
	void *const lastUSpace = ZR2SS_USPACE(svector);

	size_t const uspaceOffset = (ZR2SS_TOTALSPACE_SIZEOF(svector) - (newNbObj * objSize)) / 2;
	ZR2SS_USPACE(svector) = (char*)ZR2SS_FSPACE(svector) + uspaceOffset;

// We dont add the new elements here, this is a virtual setting
	ZR2SS_ESPACE(svector) = (char*)ZR2SS_USPACE(svector) + (nbObj * objSize);

	ZRARRAYOP_CPY(ZR2SS_USPACE(svector), objSize, nbObj, lastUSpace);
}

static inline void moreSize(ZRVector *vec, size_t nbObjMore)
{
	_moreSize(vec, nbObjMore, setFUEOSpaces);
}

static bool enoughSpaceForInsert(ZR2SSVector *svector, size_t pos, size_t nbMore)
{
	size_t freeSpace;
	size_t const nbBytesMore = nbMore * ZRVECTOR_OBJSIZE(ZR2SS_VECTOR(svector));
	size_t const middleOffset = ZRVECTOR_NBOBJ(ZR2SS_VECTOR(svector)) / 2;
	bool const toTheRight = pos >= middleOffset;

	if (toTheRight)
		freeSpace = ZR2SS_ESPACE_SIZEOF(svector);
	else
		freeSpace = ZR2SS_FSPACE_SIZEOF(svector);

	return freeSpace >= nbBytesMore;
}

/**
 * Shift some object on the right or left to free space for new objects.
 *
 * This function assume that it exists enough space (according to CenterAdd) to do its work.
 */
ZRMUSTINLINE
static inline void operationAdd_shift(ZRVector *vec, size_t pos, size_t nbMore, enum ShiftModeE shiftMode)
{
	bool centerSpace = shiftMode == ShiftLR;
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	assert(ZR2SS_FREESPACE_SIZEOF(svector) >= nbMore * ZRVECTOR_OBJSIZE(ZR2SS_VECTOR(svector)));

	if (centerSpace && !enoughSpaceForInsert(svector, pos, nbMore))
		centerFUEOSpaces(svector, nbMore);

	size_t const nbObj = ZRVECTOR_NBOBJ(ZR2SS_VECTOR(svector));
	size_t const objSize = ZRVECTOR_OBJSIZE(ZR2SS_VECTOR(svector));
	size_t const nbBytesMore = nbMore * objSize;

	/* The insertion will be act on the right part ? */
	bool toTheRight;

	if (shiftMode == ShiftLR)
	{
		size_t const middleOffset = nbObj / 2;
		toTheRight = pos >= middleOffset;
	}
	else
		toTheRight = shiftMode == ShiftRight;

	void *dest;
	void *src;
	size_t nbObjForDepl;

	if (toTheRight)
	{
		nbObjForDepl = nbObj - pos;
		src = ZRVECTOR_GET(ZR2SS_VECTOR(svector), pos);
		dest = (char*)src + nbBytesMore;
		ZR2SS_ESPACE(svector) = (char*)ZR2SS_ESPACE(svector) + nbBytesMore;
	}
	else
	{
		nbObjForDepl = pos;
		src = ZRVECTOR_GET(ZR2SS_VECTOR(svector), 0);
		dest = (char*)src - nbBytesMore;
		ZR2SS_USPACE(svector) = dest;
	}

	if (nbObjForDepl == 0)
		return;

	ZRARRAYOP_DEPLACE(dest, objSize, nbObjForDepl, src);
}

ZRMUSTINLINE
static inline void operationDel_shift(ZRVector *vec, size_t pos, size_t nbLess, enum ShiftModeE shiftMode)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	size_t const nbObj = ZRVECTOR_NBOBJ(ZR2SS_VECTOR(svector));
	size_t const objSize = ZRVECTOR_OBJSIZE(ZR2SS_VECTOR(svector));
	size_t const nbBytesLess = nbLess * objSize;
	bool toTheRight;

	if (shiftMode == ShiftLR)
	{
		size_t const middleOffset = nbObj / 2;
		toTheRight = pos >= middleOffset;
	}
	else
		toTheRight = shiftMode == ShiftRight;

	void *dest;
	void *src;
	size_t nbObjForDepl;

	if (toTheRight)
	{
		nbObjForDepl = nbObj - pos - nbLess;
		dest = ZRVECTOR_GET(ZR2SS_VECTOR(svector), pos);
		src = dest + nbBytesLess;
		ZR2SS_ESPACE(svector) = (char*)ZR2SS_ESPACE(svector) - nbBytesLess;
	}
	else
	{
		nbObjForDepl = pos;
		src = ZRVECTOR_GET(ZR2SS_VECTOR(svector), 0);
		dest = src + nbBytesLess;
		ZR2SS_USPACE(svector) = dest;
	}

	if (nbObjForDepl > 0)
		ZRARRAYOP_DEPLACE(dest, objSize, nbObjForDepl, src);
}

// ============================================================================

// ============================================================================

static void finsert_oneSide(ZRVector *vec, size_t pos, size_t nb)
{
	_finsert(vec, pos, nb, ShiftRight);
}

static void finsertGrow_oneSide(ZRVector *vec, size_t pos, size_t nb)
{
	_finsertGrow(vec, pos, nb, ShiftRight, setFUEOSpaces_oneSide);
}

static void fdelete_oneSide(ZRVector *vec, size_t pos, size_t nb)
{
	_fdelete(vec, pos, nb, ShiftRight);
}

static void fdeleteShrink_oneSide(ZRVector *vec, size_t pos, size_t nb)
{
	_fdeleteShrink(vec, pos, nb, ShiftRight, setFUEOSpaces_oneSide);
}

// ============================================================================

static void finsert(ZRVector *vec, size_t pos, size_t nb)
{
	_finsert(vec, pos, nb, ShiftLR);
}

static void finsertGrow(ZRVector *vec, size_t pos, size_t nb)
{
	_finsertGrow(vec, pos, nb, ShiftLR, setFUEOSpaces);
}

static void fdelete(ZRVector *vec, size_t pos, size_t nb)
{
	_fdelete(vec, pos, nb, ShiftLR);
}

static void fdeleteShrink(ZRVector *vec, size_t pos, size_t nb)
{
	_fdeleteShrink(vec, pos, nb, ShiftLR, setFUEOSpaces);
}

static void fdone(ZRVector *vec)
{
	if (ZRVector2SideStrategy_memoryIsAllocated(ZRVECTOR_2SS(vec)))
		ZRFREE(ZRVECTOR_2SS(vec)->allocator, ZRVECTOR_2SS(vec)->allocatedMemory);
}

/**
 * Delete a vector created by one of the creation helper functions
 */
static void fdestroy(ZRVector *vec)
{
	ZRAllocator *const allocator = ZRVECTOR_2SS(vec)->allocator;
	ZRVECTOR_DONE(vec);
	ZRFREE(allocator, vec);
}

// ============================================================================
// Help
// ============================================================================

typedef struct
{
	ZRObjAlignInfos vectorInfos[ZRVECTOR_INFOS_NB];
	ZRObjInfos objInfos;
	size_t initialArrayNbObj;
	size_t initialMemoryNbObj;
	ZRAllocator *allocator;
	unsigned oneSide :1;
	unsigned fixed :1;
	unsigned staticStrategy :1;
	unsigned changefdestroy :1;
} ZR2SSInitInfos;

ZRObjInfos ZRVector2SideStrategyInfos_objInfos(void)
{
	ZRObjInfos ret = { ZRTYPE_ALIGNMENT_SIZE(ZR2SSInitInfos) };
	return ret;
}

ZRMUSTINLINE
static inline void ZRVector2SideStrategyInfos_validate(ZR2SSInitInfos *initInfos)
{
	getVectorInfos((void*)initInfos, initInfos->initialArrayNbObj, ZROBJINFOS_SIZE_ALIGNMENT(initInfos->objInfos), (bool)initInfos->staticStrategy);
}

void ZRVector2SideStrategyInfos_allocator(void *infos_out, ZRAllocator *allocator)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	initInfos->allocator = allocator;
}

void ZRVector2SideStrategyInfos_initialArraySize(void *infos_out, size_t size)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	initInfos->initialArrayNbObj = checkInitialNbObj(size);
	ZRVector2SideStrategyInfos_validate(initInfos);
}

void ZRVector2SideStrategyInfos_initialMemorySize(void *infos_out, size_t size)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	initInfos->initialMemoryNbObj = checkInitialNbObj(size);
	ZRVector2SideStrategyInfos_validate(initInfos);
}

void ZRVector2SideStrategyInfos_fixedArray(void *infos_out)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	initInfos->fixed = 1;
}

void ZRVector2SideStrategyInfos_oneSide(void *infos_out)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	initInfos->oneSide = 1;
}

void ZRVector2SideStrategyInfos(void *infos_out, ZRObjInfos objInfos)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	*initInfos = (ZR2SSInitInfos ) { //
		.objInfos = objInfos,
		.initialArrayNbObj = INITIAL_SIZE,
		.initialMemoryNbObj = INITIAL_SIZE,
		.allocator = NULL,
		.fixed = 0,
		};
	ZRVector2SideStrategyInfos_validate(initInfos);
}

void ZRVector2SideStrategyInfos_staticStrategy(void *infos_out)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	initInfos->staticStrategy = 1;
	ZRVector2SideStrategyInfos_validate(initInfos);
}

ZRObjInfos ZRVector2SideStrategy_objInfos(void *infos)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos;
	return ZROBJALIGNINFOS_CPYOBJINFOS(initInfos->vectorInfos[ZRVectorInfos_struct]);
}

static void ZRVector2SideStrategy_initStrategy_oneSide(ZRVector2SideStrategy *strategy, ZR2SSInitInfos *infos)
{
	*strategy = (ZRVector2SideStrategy ) { //
		.strategy = { //
			.finitVec = finitVec_oneSide,
			.finsert = infos->fixed ? finsert_oneSide : finsertGrow_oneSide,
			.fdelete = infos->fixed ? fdelete_oneSide : fdeleteShrink_oneSide,
			.fchangeObjSize = fchangeObjSize_oneSide,
			.fmemoryTrim = fmemoryTrim,
			.fdone = fdone,
			.fdestroy = infos->changefdestroy ? fdestroy : fdone,
			},
		};
}

static void ZRVector2SideStrategy_initStrategy(ZRVector2SideStrategy *strategy, ZR2SSInitInfos *infos)
{
	*strategy = (ZRVector2SideStrategy ) { //
		.strategy = { //
			.finitVec = finitVec,
			.finsert = infos->fixed ? finsert : finsertGrow,
			.fdelete = infos->fixed ? fdelete : fdeleteShrink,
			.fchangeObjSize = fchangeObjSize,
			.fmemoryTrim = fmemoryTrim,
			.fdone = fdone,
			.fdestroy = infos->changefdestroy ? fdestroy : fdone,
			},
		};
}

void ZRVector2SideStrategy_init(ZRVector *vector, void *infos_p)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_p;
	ZRObjAlignInfos *infos = initInfos->vectorInfos;
	ZR2SSVector *ssvector = ZRVECTOR_2SS(vector);
	ZRAllocator *allocator;

	if (initInfos->allocator == NULL)
		allocator = zrlib_getServiceFromID(ZRSERVICE_ID(ZRService_allocator)).object;
	else
		allocator = initInfos->allocator;

	*ssvector = (ZR2SSVector )
		{ //
		.allocator = allocator,
		.initialArrayOffset = infos[ZRVectorInfos_objs].offset,
		.initialArray = ZRARRAYOP_GET(vector, 1, infos[ZRVectorInfos_objs].offset),
		.initialArraySize = infos[ZRVectorInfos_objs].size,
		.initialMemoryNbObj = initInfos->initialMemoryNbObj,
		.staticStrategy = initInfos->staticStrategy,

		.resizeData = (ZRResizeData ) { //
			.growStrategy = (ZRResizeGrowStrategy ) { ZRResizeOp_limit_100, ZRResizeOp_increase_100 },
			.shrinkStrategy = (ZRResizeShrinkStrategy ) { ZRResizeOp_limit_90, ZRResizeOp_limit_50 } ,
			} ,
		};
	ssvector->resizeData.initialNb = getInitialMemoryNbObjs(initInfos->initialMemoryNbObj, initInfos->initialArrayNbObj, initInfos->objInfos.size);

	ZRVector2SideStrategy *strategy;
	ZRVector2SideStrategy ref;
	zrlib_initPType(&ref);

	if (initInfos->oneSide)
		ZRVector2SideStrategy_initStrategy_oneSide(&ref, initInfos);
	else
		ZRVector2SideStrategy_initStrategy(&ref, initInfos);

	if (initInfos->staticStrategy)
	{
		strategy = ZRARRAYOP_GET(vector, 1, infos[ZRVectorInfos_strategy].offset);
		ZRPTYPE_CPY(strategy, &ref);
	}
	else
		strategy = zrlib_internPType(&ref);

	ZRVECTOR_INIT(vector, initInfos->objInfos, ZR2SSSTRATEGY_VECTOR(strategy));
}

ZRVector* ZRVector2SideStrategy_new(void *infos_p)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_p;

	ZR2SSVector *ret = ZROBJALLOC(initInfos->allocator, ZROBJALIGNINFOS_CPYOBJINFOS(initInfos->vectorInfos[ZRVectorInfos_struct]));

	initInfos->changefdestroy = 1;
	ZRVector2SideStrategy_init(ZR2SS_VECTOR(ret), infos_p);
	initInfos->changefdestroy = 0;
	return ZR2SS_VECTOR(ret);
}

void ZRVector2SideStrategyInfos_fixed(void *initInfos, size_t initialNbObj, ZRObjInfos objInfos, ZRAllocator *allocator)
{
	ZRVector2SideStrategyInfos(initInfos, objInfos);
	ZRVector2SideStrategyInfos_fixedArray(initInfos);
	ZRVector2SideStrategyInfos_allocator(initInfos, allocator);
	ZRVector2SideStrategyInfos_initialArraySize(initInfos, initialNbObj);
}

void ZRVector2SideStrategyInfos_dynamic(void *initInfos, size_t initialNbObj, ZRObjInfos objInfos, ZRAllocator *allocator)
{
	ZRVector2SideStrategyInfos(initInfos, objInfos);
	ZRVector2SideStrategyInfos_allocator(initInfos, allocator);
	ZRVector2SideStrategyInfos_initialArraySize(initInfos, initialNbObj);
}

ZRVector* ZRVector2SideStrategy_createFixed(size_t initialNbObj, ZRObjInfos objInfos, ZRAllocator *allocator)
{
	ZR2SSInitInfos initInfos;
	ZRVector2SideStrategyInfos_fixed(&initInfos, initialNbObj, objInfos, allocator);
	return ZRVector2SideStrategy_new(&initInfos);
}

ZRVector* ZRVector2SideStrategy_createDynamic(size_t initialNbObj, ZRObjInfos objInfos, ZRAllocator *allocator)
{
	ZR2SSInitInfos initInfos;
	ZRVector2SideStrategyInfos_dynamic(&initInfos, initialNbObj, objInfos, allocator);
	return ZRVector2SideStrategy_new(&initInfos);
}
