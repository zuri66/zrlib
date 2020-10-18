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

static void ZRVector2SideStrategy_growOnAdd(ZRVectorStrategy *strategy, bool v);
static void ZRVector2SideStrategy_shrinkOnDelete(ZRVectorStrategy *strategy, bool v);

static inline void moreSize(ZRVector *vec, size_t nbObjMore);
static inline void setFUEOSpaces(ZR2SSVector *svector, void *fspace, size_t fspaceNbObj, void *source, size_t sourceNbObj);

// ============================================================================

ZRMUSTINLINE
static inline bool ZRVector2SideStrategy_memoryIsAllocated(ZR2SSVector *vec)
{
	return vec->allocatedMemory != NULL ;
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
		// Must be set before moreSize()
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
				svector->initialArray = (char*)svector + offset;
				size_t const size = (svector->initialArraySize - offsetDiff) / objSize;
				ZRVECTOR_CAPACITY(ZR2SS_VECTOR(svector)) = size;
				setFUEOSpaces(svector, svector->initialArray, size, NULL, 0);
			}
			else
			{
				ZR2SS_FSPACE(svector) =
				ZR2SS_USPACE(svector) =
				ZR2SS_OSPACE(svector) =
				ZR2SS_ESPACE(svector) = svector;
				moreSize(ZR2SS_VECTOR(svector), 0);
			}
		}
	}
	else
		moreSize(ZR2SS_VECTOR(svector), 0);
}

static
void ZRVector2SideStrategy_fixedMemory(ZRVectorStrategy *strategy)
{
	ZRVector2SideStrategy_growOnAdd(strategy, false);
	ZRVector2SideStrategy_shrinkOnDelete(strategy, false);
}

static
void ZRVector2SideStrategy_dynamicMemory(ZRVectorStrategy *strategy)
{
	ZRVector2SideStrategy_growOnAdd(strategy, true);
	ZRVector2SideStrategy_shrinkOnDelete(strategy, true);
}

static
void ZRVector2SideStrategy_growOnAdd(ZRVectorStrategy *strategy, bool v)
{
	ZRVector2SideStrategy *twoSideStrategy = (ZRVector2SideStrategy*)strategy;
	twoSideStrategy->strategy.finsert = v ? finsertGrow : finsert;
}

static
void ZRVector2SideStrategy_shrinkOnDelete(ZRVectorStrategy *strategy, bool v)
{
	ZRVector2SideStrategy *twoSideStrategy = (ZRVector2SideStrategy*)strategy;
	twoSideStrategy->strategy.fdelete = v ? fdeleteShrink : fdelete;
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
	ZR2SS_OSPACE(svector) = (char*)ZR2SS_FSPACE(svector) + (fspaceNbObj * objSize);

	if (sourceNbObj == 0)
	{
		ZR2SS_USPACE(svector) = (char*)ZR2SS_FSPACE(svector) + ((fspaceNbObj * objSize) / 2);
		ZR2SS_ESPACE(svector) = (char*)ZR2SS_USPACE(svector);
	}
	else
	{
		ZR2SS_USPACE(svector) = (char*)ZR2SS_FSPACE(svector) + ((fspaceNbObj - sourceNbObj) * objSize) / 2;
		ZR2SS_ESPACE(svector) = (char*)ZR2SS_USPACE(svector) + (sourceNbObj * objSize);

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
	setFUEOSpaces(svector, new.array, nextTotalNbObj, ZR2SS_USPACE(svector), nbObj);

	if (isAllocated)
		ZRFREE(svector->allocator, svector->allocatedMemory);

	svector->allocatedMemory = new.array;
	ZRVECTOR_CAPACITY(ZR2SS_VECTOR(svector)) = nextTotalNbObj;
}

ZRMUSTINLINE
static inline void lessSize(ZRVector *vec)
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
	setFUEOSpaces(svector, new.array, nextTotalNbObj, ZR2SS_USPACE(svector), nbObj);

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
static inline void operationAdd_shift(ZRVector *vec, size_t pos, size_t nbMore)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	assert(ZR2SS_FREESPACE_SIZEOF(svector) >= nbMore * ZRVECTOR_OBJSIZE(ZR2SS_VECTOR(svector)));

	if (!enoughSpaceForInsert(svector, pos, nbMore))
		centerFUEOSpaces(svector, nbMore);

	size_t const nbObj = ZRVECTOR_NBOBJ(ZR2SS_VECTOR(svector));
	size_t const objSize = ZRVECTOR_OBJSIZE(ZR2SS_VECTOR(svector));
	size_t const nbBytesMore = nbMore * objSize;
	size_t const middleOffset = nbObj / 2;

// The insertion will be act on the right part ?
	bool const toTheRight = pos >= middleOffset;

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
static inline void operationDel_shift(ZRVector *vec, size_t pos, size_t nbLess)
{
	ZR2SSVector *const svector = ZRVECTOR_2SS(vec);
	size_t const nbObj = ZRVECTOR_NBOBJ(ZR2SS_VECTOR(svector));
	size_t const objSize = ZRVECTOR_OBJSIZE(ZR2SS_VECTOR(svector));
	size_t const nbBytesLess = nbLess * objSize;
	size_t const middleOffset = nbObj / 2;
	bool const toTheRight = pos >= middleOffset;

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

static void finsert(ZRVector *vec, size_t pos, size_t nb)
{
	operationAdd_shift(vec, pos, nb);
	ZRVECTOR_NBOBJ(vec) += nb;
}

static void finsertGrow(ZRVector *vec, size_t pos, size_t nb)
{
	size_t const objSize = ZRVECTOR_OBJSIZE(vec);

	if (mustGrow(vec, nb))
		moreSize(vec, nb);

	operationAdd_shift(vec, pos, nb);
	ZRVECTOR_NBOBJ(vec) += nb;
}

static void fdelete(ZRVector *vec, size_t pos, size_t nb)
{
	assert(ZR2SS_FSPACE(ZRVECTOR_2SS(vec)) != NULL);
	assert(nb <= ZRVECTOR_NBOBJ(vec));

	operationDel_shift(vec, pos, nb);
	ZRVECTOR_NBOBJ(vec) -= nb;
}

static void fdeleteShrink(ZRVector *vec, size_t pos, size_t nb)
{
	assert(ZR2SS_FSPACE(ZRVECTOR_2SS(vec)) != NULL);
	assert(nb <= ZRVECTOR_NBOBJ(vec));

	operationDel_shift(vec, pos, nb);
	ZRVECTOR_NBOBJ(vec) -= nb;

	if (mustShrink(vec))
		lessSize(vec);
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
}

void ZRVector2SideStrategyInfos_initialMemorySize(void *infos_out, size_t size)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	initInfos->initialMemoryNbObj = checkInitialNbObj(size);
}

void ZRVector2SideStrategyInfos_fixedArray(void *infos_out)
{
	ZR2SSInitInfos *initInfos = (ZR2SSInitInfos*)infos_out;
	initInfos->fixed = 1;
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
		.initialArray = (char*)vector + infos[ZRVectorInfos_objs].offset,
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

	ZR2SSVector *ret = ZRALLOC(initInfos->allocator, initInfos->vectorInfos[ZRVectorInfos_struct].size);

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

ZRVector* ZRVector2SideStrategy_createFixed(_ size_t initialNbObj, ZRObjInfos objInfos, ZRAllocator *allocator)
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
