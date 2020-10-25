/**
 * @author zuri
 * @date dim. 24 mai 2020 16:35:38 CEST
 */

#include <zrlib/lib/init.h>
#include <zrlib/base/Map/VectorMap.h>

#include <zrlib/base/struct.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <stdalign.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct ZRVectorMapS ZRVectorMap;
typedef struct ZRVectorMapStrategyS ZRVectorMapStrategy;

struct ZRVectorMapStrategyS
{
	ZRMapStrategy strategy;
};

// ============================================================================

#define ZRVMAP_MAP(VMAP) (&(VMAP)->map)
#define ZRVMAP(VMAP) ((ZRVectorMap*)(VMAP))

typedef enum
{
	BucketInfos_key,
	BucketInfos_obj,
	BucketInfos_struct,
	BUCKETINFOS_NB,
} BucketInfos;

struct ZRVectorMapS
{
	ZRMap map;
	ZRObjAlignInfos bucketInfos[BUCKETINFOS_NB];
	ZRAllocator *allocator;
	ZRVector *vector;

	zrfucmp fucmp;
	unsigned staticStrategy :1;
	unsigned destroyVector :1;
};

typedef enum
{
	VectorMapInfos_base = 0,
	VectorMapInfos_strategy,
	VectorMapInfos_vector,
	VectorMapInfos_struct,
	VECTORMAPINFOS_NB,
} VectorMapInfos;

enum InsertModeE
{
	PUT, REPLACE, PUTIFABSENT
};

// ============================================================================

#define bucket_get(VMAP,BUCKET,FIELD) ((char*)BUCKET + (VMAP)->bucketInfos[FIELD].offset)
#define bucket_key(VMAP,BUCKET) bucket_get(VMAP,BUCKET,BucketInfos_key)
#define bucket_obj(VMAP,BUCKET) bucket_get(VMAP,BUCKET,BucketInfos_obj)
#define bucket_nextUnused(VMAP,BUCKET) (*(ZRReserveNextUnused*)bucket_get(VMAP,BUCKET,ZRVectorMapInfos_nextUnused))

static void VectorMapInfos_make(ZRObjAlignInfos *infos, bool staticStrategy, ZRObjInfos vectorInfos_infos)
{
	infos[VectorMapInfos_base] = ZRTYPE_OBJALIGNINFOS(ZRVectorMap);
	infos[VectorMapInfos_strategy] = staticStrategy ? ZRTYPE_OBJALIGNINFOS(ZRVectorMapStrategy) : ZROBJALIGNINFOS_DEF0();
	infos[VectorMapInfos_vector] = ZROBJINFOS_CPYOBJALIGNINFOS(vectorInfos_infos);
	infos[VectorMapInfos_struct] = ZROBJALIGNINFOS_DEF0();
	ZRStruct_bestOffsetsPos(VECTORMAPINFOS_NB - 1, infos, 1);
}

static void bucketInfos_make(ZRObjAlignInfos *out, ZRObjInfos objInfos, ZRObjInfos keyInfos)
{
	out[BucketInfos_key] = ZROBJINFOS_CPYOBJALIGNINFOS(keyInfos);
	out[BucketInfos_obj] = ZROBJINFOS_CPYOBJALIGNINFOS(objInfos);
	out[BucketInfos_struct] = ZROBJALIGNINFOS_DEF0();
	ZRStruct_bestOffsets(BUCKETINFOS_NB - 1, out);
}

// ============================================================================

static void finitMap(ZRMap *map)
{
}

static void fdone(ZRMap *map)
{
	ZRVectorMap *const vmap = ZRVMAP(map);

	if (vmap->destroyVector)
		ZRVector_destroy(vmap->vector);
}

static int bucketCmp(void *a, void *b, void *vmap_p)
{
	ZRVectorMap *const vmap = ZRVMAP(vmap_p);
	return vmap->fucmp(a, bucket_key(vmap, b), vmap);
}

// ============================================================================
// ORDER

static inline size_t getBucketPos(ZRVectorMap *vmap, void *key)
{
	return ZRARRAYOP_BSEARCH_POS(ZRARRAY_OON(vmap->vector->array), key, bucketCmp, vmap);
}

static inline void* getBucket(ZRVectorMap *vmap, void *key)
{
	size_t const pos = getBucketPos(vmap, key);

	if (pos == SIZE_MAX)
		return NULL ;

	return ZRVECTOR_GET(vmap->vector, pos);
}

#define insert_return(B) ZRBLOCK( \
	if (out) \
		*out = bucket_obj(vmap, bucket); \
	return B; \
)

static inline bool insert(ZRMap *map, void *key, void *obj, enum InsertModeE mode, void **out)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	size_t const pos = ZRARRAYOP_BINSERT_POS_LAST(ZRARRAY_OON(vmap->vector->array), key, bucketCmp, vmap);
	void *bucket;

// Check if the bucket is at pos
	if (pos == 0)
		bucket = NULL;
	else
	{
		bucket = ZRVECTOR_GET(vmap->vector, pos - 1);

		if (vmap->fucmp(key, bucket_key(vmap, bucket), vmap) != 0)
			bucket = NULL;
	}

// No bucket found
	if (bucket == NULL)
	{
		if (mode == REPLACE)
			insert_return(false);

		alignas(max_align_t) char tmpBucket[vmap->bucketInfos[BucketInfos_struct].size];

		memcpy(bucket_key(vmap, tmpBucket), key, map->keyInfos.size);
		memcpy(bucket_obj(vmap, tmpBucket), obj, map->objInfos.size);
		ZRVECTOR_INSERT(vmap->vector, pos, tmpBucket);
		ZRVMAP_MAP(vmap)->nbObj++;
	}
	else
	{
		if (mode == PUTIFABSENT)
			insert_return(false);

		memcpy(bucket_obj(vmap, bucket), obj, map->objInfos.size);
	}
	insert_return(true);
}

static void fput(ZRMap *map, void *key, void *obj, void **out)
{
	insert(map, key, obj, PUT, out);
}

static bool fputIfAbsent(ZRMap *map, void *key, void *obj, void **out)
{
	return insert(map, key, obj, PUTIFABSENT, out);
}

static bool freplace(ZRMap *map, void *key, void *obj, void **out)
{
	return insert(map, key, obj, REPLACE, out);
}

static void* fget(ZRMap *map, void *key)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	void *const bucket = getBucket(vmap, key);

	if (bucket == NULL)
		return NULL ;

	return bucket_obj(vmap, bucket);
}

static bool fdelete(ZRMap *map, void *key, void *cpy_out)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	size_t pos = getBucketPos(vmap, key);

	if (pos == SIZE_MAX)
		return false;

	if (cpy_out)
		memcpy(cpy_out, ZRVECTOR_GET(vmap->vector, pos), ZRVECTOR_OBJSIZE(vmap->vector));

	ZRVECTOR_DELETE(vmap->vector, pos);
	ZRVMAP_MAP(vmap)->nbObj--;
	return true;
}

// ============================================================================
// EQ

static inline size_t eq_getBucketPos(ZRVectorMap *vmap, void *key)
{
	return ZRARRAYOP_SEARCH_POS(ZRARRAY_OON(vmap->vector->array), key, bucketCmp, vmap);
}

static inline void* eq_getBucket(ZRVectorMap *vmap, void *key)
{
	size_t const pos = eq_getBucketPos(vmap, key);

	if (pos == SIZE_MAX)
		return NULL ;

	return ZRVECTOR_GET(vmap->vector, pos);
}

static inline bool eq_insert(ZRMap *map, void *key, void *obj, enum InsertModeE mode, void **out)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	size_t const pos = ZRARRAYOP_SEARCH_POS(ZRARRAY_OON(vmap->vector->array), key, bucketCmp, vmap);
	void *bucket;

// No bucket found
	if (pos == SIZE_MAX)
	{
		if (mode == REPLACE)
			return false;

		alignas(max_align_t) char tmpBucket[vmap->bucketInfos[BucketInfos_struct].size];

		memcpy(bucket_key(vmap, tmpBucket), key, map->keyInfos.size);
		memcpy(bucket_obj(vmap, tmpBucket), obj, map->objInfos.size);
		ZRVECTOR_ADD(vmap->vector, tmpBucket);
		ZRVMAP_MAP(vmap)->nbObj++;
	}
	else
	{
		if (mode == PUTIFABSENT)
			return false;

		bucket = ZRVECTOR_GET(vmap->vector, pos);
		memcpy(bucket_obj(vmap, bucket), obj, map->objInfos.size);
	}

	if (out)
		*out = bucket_obj(vmap, bucket);

	return true;
}

static void eq_fput(ZRMap *map, void *key, void *obj, void **out)
{
	eq_insert(map, key, obj, PUT, out);
}

static bool eq_fputIfAbsent(ZRMap *map, void *key, void *obj, void **out)
{
	return eq_insert(map, key, obj, PUTIFABSENT, out);
}

static bool eq_freplace(ZRMap *map, void *key, void *obj, void **out)
{
	return eq_insert(map, key, obj, REPLACE, out);
}

static void* eq_fget(ZRMap *map, void *key)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	void *const bucket = eq_getBucket(vmap, key);

	if (bucket == NULL)
		return NULL ;

	return bucket_obj(vmap, bucket);
}

static bool eq_fdelete(ZRMap *map, void *key, void *cpy_out)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	size_t pos = eq_getBucketPos(vmap, key);

	if (pos == SIZE_MAX)
		return false;

	if (cpy_out)
		memcpy(cpy_out, ZRVECTOR_GET(vmap->vector, pos), ZRVECTOR_OBJSIZE(vmap->vector));

	ZRVECTOR_DELETE(vmap->vector, pos);
	ZRVMAP_MAP(vmap)->nbObj--;
	return true;
}

// ============================================================================

static size_t fcpyKeyValPtr(ZRMap *map, ZRMapKeyVal *cpyTo, size_t offset, size_t maxNbCpy)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	size_t const nbObj = ZRVECTOR_NBOBJ(vmap->vector);

	if (offset > nbObj)
		return 0;

	size_t const nbCpy = ZRMIN(maxNbCpy, nbObj - offset);
	size_t i = nbCpy;

	while (i--)
	{
		void *bucket = ZRVECTOR_GET(vmap->vector, offset++);
		cpyTo->key = bucket_key(vmap, bucket);
		cpyTo->val = bucket_obj(vmap, bucket);
		cpyTo++;
	}
	return nbCpy;
}

static void fdeleteAll(ZRMap *map)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	ZRVECTOR_DELETE_ALL(vmap->vector);
	ZRVMAP_MAP(vmap)->nbObj = 0;
}

ZRVector* ZRVectorMap_vector(ZRMap *map)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	return vmap->vector;
}

static void fdestroy(ZRMap *map)
{
	ZRMap_done(map);
	ZRAllocator *allocator = ZRVMAP(map)->allocator;
	ZRFREE(allocator, map);
}

// ============================================================================

static int default_cmp(void *a, void *b, void *map_p)
{
	ZRVectorMap *vmap = ZRVMAP(map_p);
	return memcmp(a, b, vmap->vector->array.objInfos.size);
}

typedef struct
{
	ZRObjAlignInfos infos[VECTORMAPINFOS_NB];
	ZRAllocator *allocator;

	union
	{
		ZRVector *vector;
		struct
		{
			void *vectorInfos;
			ZRObjInfos vector_infos;
			void (*fvectorInit)(ZRVector*, void*);
		};
	};
	ZRObjInfos keyInfos;
	ZRObjInfos objInfos;

	zrfucmp fucmp;
	enum ZRVectorMap_modeE mode;

	unsigned staticStrategy :1;
	unsigned changefdestroy :1;
	unsigned staticVector :1;
	unsigned destroyVector :1;
} VectorMapInitInfos;

static void ZRVectorMapStrategy_init(ZRVectorMapStrategy *strategy, VectorMapInitInfos *infos)
{
	zrlib_initPType(strategy);
	void (*tmp_fdestroy)(ZRMap*) = infos->changefdestroy ? fdestroy : fdone;

	if (infos->mode == ZRVectorMap_modeOrder)
		*strategy = (ZRVectorMapStrategy )
			{
				.strategy =
					{
						.finitMap = finitMap,
						.fput = fput,
						.fputIfAbsent = fputIfAbsent,
						.freplace = freplace,
						.fcpyKeyValPtr = fcpyKeyValPtr,
						.fget = fget,
						.fdelete = fdelete,
						.fdeleteAll = fdeleteAll,
						.fdone = fdone,
						.fdestroy = tmp_fdestroy,
					},
			};
	else
		*strategy = (ZRVectorMapStrategy )
			{
				.strategy =
					{
						.finitMap = finitMap,
						.fput = eq_fput,
						.fputIfAbsent = eq_fputIfAbsent,
						.freplace = eq_freplace,
						.fcpyKeyValPtr = fcpyKeyValPtr,
						.fget = eq_fget,
						.fdelete = eq_fdelete,
						.fdeleteAll = fdeleteAll,
						.fdone = fdone,
						.fdestroy = tmp_fdestroy,
					},
			};

}

static ZRObjInfos vectorInfos(VectorMapInitInfos *infos)
{
	return infos->staticVector ? infos->vector_infos : ZROBJINFOS_DEF0();
}

static void ZRVectorMapInfos_validate(VectorMapInitInfos *infos)
{
	VectorMapInfos_make(infos->infos, infos->staticStrategy, vectorInfos(infos));
}

ZRObjInfos ZRVectorMapInfos_objInfos(void)
{
	return ZRTYPE_OBJINFOS(VectorMapInitInfos);
}

ZRObjInfos ZRVectorMap_itemObjInfos(void *infos_p)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_p;
	ZRObjAlignInfos bucketInfos[BUCKETINFOS_NB];
	bucketInfos_make(bucketInfos, infos->keyInfos, infos->objInfos);
	return ZROBJALIGNINFOS_CPYOBJINFOS(bucketInfos[BucketInfos_struct]);
}

void ZRVectorMapInfos(void *infos_out, ZRObjInfos keyInfos, ZRObjInfos objInfos)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_out;
	*infos = (VectorMapInitInfos ) { //
		.keyInfos = keyInfos,
		.objInfos = objInfos,

		.allocator = NULL,
		.fucmp = default_cmp,
		.mode = ZRVectorMap_modeOrder,
		};
	ZRVectorMapInfos_validate(infos);
}

void ZRVectorMapInfos_keyInfos(void *infos_p, ZRObjInfos keyInfos)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_p;
	infos->keyInfos = keyInfos;
	ZRVectorMapInfos_validate(infos);
}

void ZRVectorMapInfos_setObjInfos(void *infos_p, ZRObjInfos objInfos)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_p;
	infos->objInfos = objInfos;
	ZRVectorMapInfos_validate(infos);
}

void ZRVectorMapInfos_fucmp(void *infos_out, zrfucmp fucmp, enum ZRVectorMap_modeE mode)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_out;

	if (fucmp == NULL)
		fucmp = default_cmp;

	infos->fucmp = fucmp;
	infos->mode = mode;
}

void ZRVectorMapInfos_staticStrategy(void *infos_p)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_p;
	infos->staticStrategy = 1;
	ZRVectorMapInfos_validate(infos);
}

void ZRVectorMapInfos_allocator(void *infos_p, ZRAllocator *allocator)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_p;
	infos->allocator = allocator;
}

ZRObjInfos ZRVectorMap_objInfos(void *infos_p)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_p;
	return ZROBJALIGNINFOS_CPYOBJINFOS(infos->infos[VectorMapInfos_struct]);
}

void ZRVectorMapInfos_vector(void *infos_p, ZRVector *vector, bool destroyVector)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_p;
	infos->vector = vector;
	infos->destroyVector = vector == NULL ? false : destroyVector;
	infos->staticVector = false;
	ZRVectorMapInfos_validate(infos);
}

void ZRVectorMapInfos_staticVector(void *infos_p, void *vectorInfos,
	void (*fsetObjSize)(void*, ZRObjInfos),
	ZRObjInfos (*finfos_objSize)(void*),
	void (*finit)(ZRVector*, void*)
	)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_p;
	fsetObjSize(vectorInfos, ZRVectorMap_itemObjInfos(infos));
	infos->staticVector = true;
	infos->destroyVector = true;
	infos->vectorInfos = vectorInfos;
	infos->vector_infos = finfos_objSize(vectorInfos);
	infos->fvectorInit = finit;
	ZRVectorMapInfos_validate(infos);
}

void ZRVectorMap_init(ZRMap *map, void *infos_p)
{
	bool destroyVector;
	ZRVectorMap *vmap = ZRVMAP(map);
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_p;
	ZRVector *vector;
	ZRObjAlignInfos bucketInfos[BUCKETINFOS_NB];
	bucketInfos_make(bucketInfos, infos->keyInfos, infos->objInfos);

	ZRVectorMapStrategy ref, *strategy;
	ZRVectorMapStrategy_init(&ref, infos);

	if (infos->staticStrategy)
	{
		strategy = ZRARRAYOP_GET(vmap, 1, infos->infos[VectorMapInfos_strategy].offset);
		ZRPTYPE_CPY(strategy, &ref);
	}
	else
		strategy = zrlib_internPType(&ref);

	if (infos->staticVector)
	{
		vector = ZRARRAYOP_GET(vmap, 1, infos->infos[VectorMapInfos_vector].offset);
		infos->fvectorInit(vector, infos->vectorInfos);
		destroyVector = true;
	}
	else if (infos->vector == NULL)
	{
		destroyVector = true;
		vector = ZRVector2SideStrategy_createDynamic(ZROBJALIGNINFOS_CPYOBJINFOS(bucketInfos[BucketInfos_struct]), 1024, infos->allocator);
	}
	else
	{
		vector = infos->vector;
		ZRVECTOR_CHANGEOBJSIZE(vector, ZROBJALIGNINFOS_CPYOBJINFOS(bucketInfos[BucketInfos_struct]));
		destroyVector = infos->destroyVector;
	}
	ZRAllocator *allocator;

	if (infos->allocator == NULL)
		allocator = zrlib_getServiceFromID(ZRSERVICE_ID(ZRService_allocator)).object;
	else
		allocator = infos->allocator;

	*vmap = (ZRVectorMap ) { //
		.vector = vector,
		.allocator = allocator,
		.fucmp = infos->fucmp,
		.destroyVector = destroyVector,
		};
	ZRCARRAY_CPY(vmap->bucketInfos, bucketInfos);
	ZRMAP_INIT(ZRVMAP_MAP(vmap), infos->keyInfos, infos->objInfos, (ZRMapStrategy*)strategy);
}

ZRMap* ZRVectorMap_new(void *infos_p)
{
	VectorMapInitInfos *infos = (VectorMapInitInfos*)infos_p;
	infos->changefdestroy = 1;

	ZRVectorMap *vmap = ZRALLOC(infos->allocator, infos->infos[VectorMapInfos_struct].size);
	ZRVectorMap_init(ZRVMAP_MAP(vmap), infos);

	infos->changefdestroy = 0;
	return ZRVMAP_MAP(vmap);
}

ZRMap* ZRVectorMap_create(
	ZRObjInfos keyInfos, ZRObjInfos objInfos,
	zrfucmp fucmp,
	ZRVector *vector,
	ZRAllocator *allocator,
	enum ZRVectorMap_modeE mode
	)
{
	VectorMapInitInfos infos;
	ZRVectorMapInfos(&infos, keyInfos, objInfos);
	ZRVectorMapInfos_allocator(&infos, allocator);
	ZRVectorMapInfos_fucmp(&infos, fucmp, mode);
	return ZRVectorMap_new(&infos);
}
