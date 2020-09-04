/**
 * @author zuri
 * @date dim. 24 mai 2020 16:35:38 CEST
 */

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

#define ZRVMAP_INFOS_NB 3
typedef enum
{
	BucketInfos_key, BucketInfos_obj, BucketInfos_struct
} BucketInfos;

struct ZRVectorMapS
{
	ZRMap map;
	ZRObjAlignInfos bucketInfos[ZRVMAP_INFOS_NB];
	ZRAllocator *allocator;
	ZRVector *vector;

	int (*fcmp)(void*, void*);
};

enum InsertModeE
{
	PUT, REPLACE, PUTIFABSENT
};

// ============================================================================

#define bucket_get(VMAP,BUCKET,FIELD) ((char*)BUCKET + (VMAP)->bucketInfos[FIELD].offset)
#define bucket_key(VMAP,BUCKET) bucket_get(VMAP,BUCKET,BucketInfos_key)
#define bucket_obj(VMAP,BUCKET) bucket_get(VMAP,BUCKET,BucketInfos_obj)
#define bucket_nextUnused(VMAP,BUCKET) (*(ZRReserveNextUnused*)bucket_get(VMAP,BUCKET,ZRVectorMapInfos_nextUnused))

static void bucketInfos_make(ZRObjAlignInfos *out, size_t keySize, size_t keyAlignment, size_t objSize, size_t objAlignment)
{
	out[BucketInfos_key] = (ZRObjAlignInfos ) { 0, keyAlignment, keySize };
	out[BucketInfos_obj] = (ZRObjAlignInfos ) { 0, objAlignment, objSize };
	out[BucketInfos_struct] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsets(ZRVMAP_INFOS_NB - 1, out);
}

//// ============================================================================

static void finitMap(ZRMap *map)
{
}

static void fdone(ZRMap *map)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	ZRVector_destroy(vmap->vector);
}

static int bucketCmp(void *a, void *b, void *vmap_p)
{
	ZRVectorMap *const vmap = ZRVMAP(vmap_p);
	return vmap->fcmp(a, bucket_key(vmap, b));
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

static inline bool insert(ZRMap *map, void *key, void *obj, enum InsertModeE mode)
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

		if (vmap->fcmp(key, bucket_key(vmap, bucket)) != 0)
			bucket = NULL;
	}

	// No bucket found
	if (bucket == NULL)
	{
		if (mode == REPLACE)
			return false;

		alignas(max_align_t) char tmpBucket[vmap->bucketInfos[BucketInfos_struct].size];

		memcpy(bucket_key(vmap, tmpBucket), key, map->keyInfos.size);
		memcpy(bucket_obj(vmap, tmpBucket), obj, map->objInfos.size);
		ZRVECTOR_INSERT(vmap->vector, pos, tmpBucket);
		ZRVMAP_MAP(vmap)->nbObj++;
	}
	else
	{
		if (mode == PUTIFABSENT)
			return false;

		memcpy(bucket_obj(vmap, bucket), obj, map->objInfos.size);
	}
	return true;
}

static void fput(ZRMap *map, void *key, void *obj)
{
	insert(map, key, obj, PUT);
}

static bool fputIfAbsent(ZRMap *map, void *key, void *obj)
{
	return insert(map, key, obj, PUTIFABSENT);
}

static bool freplace(ZRMap *map, void *key, void *obj)
{
	return insert(map, key, obj, REPLACE);
}

static void* fget(ZRMap *map, void *key)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	void *const bucket = getBucket(vmap, key);

	if (bucket == NULL)
		return NULL ;

	return bucket_obj(vmap, bucket);
}

static bool fdelete(ZRMap *map, void *key)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	size_t pos = getBucketPos(vmap, key);

	if (pos == SIZE_MAX)
		return false;

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

static inline bool eq_insert(ZRMap *map, void *key, void *obj, enum InsertModeE mode)
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
	return true;
}

static void eq_fput(ZRMap *map, void *key, void *obj)
{
	eq_insert(map, key, obj, PUT);
}

static bool eq_fputIfAbsent(ZRMap *map, void *key, void *obj)
{
	return eq_insert(map, key, obj, PUTIFABSENT);
}

static bool eq_freplace(ZRMap *map, void *key, void *obj)
{
	return eq_insert(map, key, obj, REPLACE);
}

static void* eq_fget(ZRMap *map, void *key)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	void *const bucket = eq_getBucket(vmap, key);

	if (bucket == NULL)
		return NULL ;

	return bucket_obj(vmap, bucket);
}

static bool eq_fdelete(ZRMap *map, void *key)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	size_t pos = eq_getBucketPos(vmap, key);

	if (pos == SIZE_MAX)
		return false;

	ZRVECTOR_DELETE(vmap->vector, pos);
	ZRVMAP_MAP(vmap)->nbObj--;
	return true;
}

// ============================================================================

static void ZRVectorMapStrategy_init(ZRMapStrategy *strategy, enum ZRVectorMap_modeE mode)
{
	if (mode == ZRVectorMap_modeOrder)
		*(ZRVectorMapStrategy*)strategy = (ZRVectorMapStrategy )
			{
				.strategy =
					{
						.finitMap = finitMap,
						.fput = fput,
						.fputIfAbsent = fputIfAbsent,
						.freplace = freplace,
						.fget = fget,
						.fdelete = fdelete,
						.fdone = fdone,
					},
			};
	else
		*(ZRVectorMapStrategy*)strategy = (ZRVectorMapStrategy )
			{
				.strategy =
					{
						.finitMap = finitMap,
						.fput = eq_fput,
						.fputIfAbsent = eq_fputIfAbsent,
						.freplace = eq_freplace,
						.fget = eq_fget,
						.fdelete = eq_fdelete,
						.fdone = fdone,
					},
			};

}

static void ZRVectorMap_init(ZRVectorMap *vmap, size_t keySize, size_t keyAlignment, size_t objSize, size_t objAlignment, int (*fcmp)(void*, void*), ZRVector *vector, ZRAllocator *allocator)
{
	bucketInfos_make(vmap->bucketInfos, keySize, keyAlignment, objSize, objAlignment);

	if (vector == NULL)
		vector = ZRVector2SideStrategy_createDynamic(1024, vmap->bucketInfos[BucketInfos_struct].size, vmap->bucketInfos[BucketInfos_struct].alignment, allocator);
	else
		ZRVECTOR_CHANGEOBJSIZE(vector, objSize, objAlignment);

	vmap->fcmp = fcmp;
	vmap->allocator = allocator;
	vmap->vector = vector;
}

ZRVector* ZRVectorMap_vector(ZRMap *map)
{
	ZRVectorMap *const vmap = ZRVMAP(map);
	return vmap->vector;
}

// ============================================================================

void ZRVectorMap_destroy(ZRMap *map)
{
	ZRMap_done(map);
	ZRAllocator *allocator = ZRVMAP(map)->allocator;
	ZRFREE(allocator, map->strategy);
	ZRFREE(allocator, map);
}

ZRMap* ZRVectorMap_create(
	size_t keySize, size_t keyAlignment,
	size_t objSize, size_t objAlignment,
	int (*fcmp)(void*, void*),
	ZRVector *vector,
	ZRAllocator *allocator,
	enum ZRVectorMap_modeE mode
	)
{
	ZRMapStrategy *strategy = ZRALLOC(allocator, sizeof(ZRVectorMapStrategy));
	ZRVectorMapStrategy_init(strategy, mode);
	strategy->fdestroy = ZRVectorMap_destroy;

	ZRVectorMap *vmap = ZRALLOC(allocator, sizeof(ZRVectorMap));
	ZRVectorMap_init(vmap, keySize, keyAlignment, objSize, objAlignment, fcmp, vector, allocator);
	ZRMap_init(ZRVMAP_MAP(vmap), ZROBJINFOS_DEF(0, keySize), ZROBJINFOS_DEF(0, objSize), strategy);

	return ZRVMAP_MAP(vmap);
}
