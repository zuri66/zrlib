/**
 * @author zuri
 * @date mardi 19 novembre 2019, 21:52:33 (UTC+0100)
 */

#include <zrlib/base/Algorithm/fcmp.h>
#include <zrlib/base/struct.h>
#include <zrlib/base/ReserveOp_list.h>
#include <zrlib/base/Map/HashTable.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/ResizeOp.h>

#include <stdalign.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct ZRHashTableBucketS ZRHashTableBucket;
typedef struct ZRHashTableS ZRHashTable;
typedef struct ZRHashTableStrategyS ZRHashTableStrategy;

struct ZRHashTableStrategyS
{
	ZRMapStrategy map;
};

// ============================================================================

#define DEFAULT_CAPACITY 512

#define ZRHASHTABLE_MAP(HTABLE) (&(HTABLE)->map)
#define ZRHASHTABLE(HTABLE) ((ZRHashTable*)(HTABLE))

#define ZRHASHTABLESTRATEGY(HTABLE) (ZRHashTableStrategy*)(ZRHASHTABLE_MAP(HTABLE)->strategy)
#define ZRHASHTABLESTRATEGY_MAP(S) (&(S)->map)

typedef enum
{
	ZRHashTableStructInfos_base = 0,
	ZRHashTableStructInfos_table,
	ZRHashTableStructInfos_strategy,
	ZRHashTableStructInfos_fhash,
	ZRHashTableStructInfos_struct,
	ZRHASHTABLESTRUCTINFOS_NB
} ZRHashTableStructInfos;

enum BucketFlags
{
	FLAG_NONE = 0,
	FLAG_DELETED = 1,
};

typedef enum
{
	ZRHashTableBucketInfos_nextUnused = 0,
	ZRHashTableBucketInfos_key,
	ZRHashTableBucketInfos_obj,
	ZRHashTableBucketInfos_flags,
	ZRHashTableBucketInfos_struct,
	ZRHASHTABLEBUCKETINFOS_NB
} ZRHashTableBucketInfos;

typedef size_t (*fhash_t)(ZRHashTable *htable, zrfuhash fuhash, void *key);
typedef int (*fcmp_t)(ZRHashTable *htable, zrfucmp fucmp, void *a, void *b);

struct ZRHashTableS
{
	ZRMap map;
	ZRObjAlignInfos bucketInfos[ZRHASHTABLEBUCKETINFOS_NB];

	/* Internal functions */
	fhash_t fhashPos;
	fcmp_t fcmp;

	/* User functions */
	zrfuhash *fuhash;
	zrfucmp fucmp;

	ZRAllocator *allocator;

	ZRArray table;
	ZRResizeData resizeData;
	ZRVector *bucketPos;

	size_t nbfhash;

	unsigned staticStrategy :1;
};

#define ZRHTABLE_LVPARRAY(HT) (HT)->table.array
#define ZRHTABLE_LVNBOBJ(HT) (HT)->table.nbObj
#define ZRHTABLE_LVSIZE(HT) (HT)->table.size
#define ZRHTABLE_LVOBJINFOS(HT) (HT)->table.objInfos
#define ZRHTABLE_LVOBJSIZE(HT) (HT)->table.objInfos.size
#define ZRHTABLE_LVOBJALIGNMENT(HT) (HT)->table.objInfos.alignment

struct ZRHashTableBucketS
{
//	ZRReserveNextUnused nextUnused;
// KType key;
// OType obj;
};

static size_t hashPos(ZRHashTable *htable, zrfuhash fuhash, void *key)
{
	size_t const hash = (*fuhash)(key, htable);
	size_t pos = hash % ZRHTABLE_LVSIZE(htable);
	return pos;
}

static size_t ptr_hashPos(ZRHashTable *htable, zrfuhash fuhash, void *key)
{
	return hashPos(htable, fuhash, *(void**)key);
}

static int cmp(ZRHashTable *htable, zrfucmp fucmp, void *a, void *b)
{
	return htable->fucmp(a, b, htable);
}

static int ptr_cmp(ZRHashTable *htable, zrfucmp fucmp, void *a, void *b)
{
	return cmp(htable, fucmp, *(void**)a, *(void**)b);
}

// ============================================================================

#define bucket_get(HTABLE,BUCKET,FIELD) ((char*)bucket + (HTABLE)->bucketInfos[FIELD].offset)
#define bucket_key(HTABLE,BUCKET) bucket_get(HTABLE,BUCKET,ZRHashTableBucketInfos_key)
#define bucket_obj(HTABLE,BUCKET) bucket_get(HTABLE,BUCKET,ZRHashTableBucketInfos_obj)
#define bucket_flags(HTABLE,BUCKET) (*(enum BucketFlags*)bucket_get(HTABLE,BUCKET,ZRHashTableBucketInfos_flags))
#define bucket_nextUnused(HTABLE,BUCKET) (*(ZRReserveNextUnused*)bucket_get(HTABLE,BUCKET,ZRHashTableBucketInfos_nextUnused))

static void bucketInfos(ZRObjAlignInfos *out, ZRObjInfos key, ZRObjInfos obj)
{
	out[ZRHashTableBucketInfos_nextUnused] = (ZRObjAlignInfos ) { 0, ZRTYPE_ALIGNMENT_SIZE(ZRReserveNextUnused) };
	out[ZRHashTableBucketInfos_key] = ZROBJINFOS_CPYOBJALIGNINFOS(key);
	out[ZRHashTableBucketInfos_obj] = ZROBJINFOS_CPYOBJALIGNINFOS(obj);
	out[ZRHashTableBucketInfos_flags] = ZRTYPE_OBJALIGNINFOS(enum BucketFlags);
	out[ZRHashTableBucketInfos_struct] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsetsPos(ZRHASHTABLEBUCKETINFOS_NB - 1, out, 1);
}

ZRMUSTINLINE
static inline size_t jenkins_one_at_a_time_hash(char *key, size_t len)
{
	size_t hash, i;
	for (hash = i = 0; i < len; ++i)
	{
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
}

static size_t default_fuhash(void *a, void *map_p)
{
	size_t ret = 0;
	ZRMap *map = (ZRMap*)map_p;

	return jenkins_one_at_a_time_hash(a, map->keyInfos.size);
}

static int default_fucmp(void *a, void *b, void *map)
{
	return memcmp(a, b, ZRMAP(map)->keyInfos.size);
}

// ============================================================================

enum InsertModeE
{
	PUT, REPLACE, PUTIFABSENT
};

static inline bool insert(ZRHashTable *htable, void *key, void *obj, enum InsertModeE mode, void **out);

ZRMUSTINLINE
static inline bool mustGrow(ZRHashTable *htable)
{
	return ZRRESIZE_MUSTGROW(ZRARRAY_SN(htable->table), &htable->resizeData);
}

ZRMUSTINLINE
static inline bool mustShrink(ZRHashTable *htable)
{
	return ZRRESIZE_MUSTSHRINK(ZRARRAY_SN(htable->table), &htable->resizeData);
}

ZRMUSTINLINE
static inline void reinsertBuckets(ZRHashTable *htable, ZRHashTableBucket *lastTable, size_t lastTableSize, size_t lastTableNbBuckets)
{
	size_t bucketSize = htable->bucketInfos[ZRHashTableBucketInfos_struct].size;
	void *buffer[512];
	size_t offset = 0;
	size_t nb = 0;
	ZRHASHTABLE_MAP(htable)->nbObj = 0;
	ZRHTABLE_LVNBOBJ(htable) = 0;

	for (size_t i = 0; i < lastTableNbBuckets; i++)
	{
		size_t pos;
		ZRVECTOR_POPFIRST(htable->bucketPos, &pos);
		ZRHashTableBucket *bucket = ZRARRAYOP_GET(lastTable, bucketSize, pos);
		insert(htable, bucket_key(htable, bucket), bucket_obj(htable, bucket), PUT, NULL);
	}
	assert(ZRVECTOR_NBOBJ(htable->bucketPos) == lastTableNbBuckets);

	ZRFREE(htable->allocator, lastTable);
}

ZRMUSTINLINE
static inline void moreSize(ZRHashTable *htable)
{
	void *lastTable = ZRHTABLE_LVPARRAY(htable);
	size_t lastNbBuckets = ZRHTABLE_LVNBOBJ(htable);
	size_t lastCapacity = ZRHTABLE_LVSIZE(htable);
	ZRArrayAndNb new = ZRRESIZE_MAKEMORESIZE(
		ZRHTABLE_LVSIZE(htable), ZRHTABLE_LVNBOBJ(htable) + 1, ZRHTABLE_LVOBJSIZE(htable), ZRHTABLE_LVOBJALIGNMENT(htable),
		false, htable->allocator, &htable->resizeData, htable
		);

	ZRHTABLE_LVSIZE(htable) = new.nbObj;
	ZRHTABLE_LVPARRAY(htable) = new.array;
	memset(ZRHTABLE_LVPARRAY(htable), 0, new.nbObj * ZRHTABLE_LVOBJSIZE(htable));

	/* Replace all buckets in the new array */
	if (lastTable != NULL)
	{
		reinsertBuckets(htable, lastTable, lastCapacity, lastNbBuckets);
	}
}

ZRMUSTINLINE
static inline void lessSize(ZRHashTable *htable)
{
	void *lastTable = ZRHTABLE_LVPARRAY(htable);
	size_t lastNbBuckets = ZRHTABLE_LVNBOBJ(htable);
	size_t lastCapacity = ZRHTABLE_LVSIZE(htable);
	ZRArrayAndNb new = ZRRESIZE_MAKELESSSIZE(
		ZRHTABLE_LVSIZE(htable), ZRHTABLE_LVNBOBJ(htable) - 1, ZRHTABLE_LVOBJSIZE(htable), ZRHTABLE_LVOBJALIGNMENT(htable),
		NULL, 0, htable->allocator, &htable->resizeData, htable
		);

	ZRHTABLE_LVSIZE(htable) = new.nbObj;
	ZRHTABLE_LVPARRAY(htable) = new.array;

	/* Replace all buckets in the new array */
	if (lastTable != NULL)
	{
		memset(ZRHTABLE_LVPARRAY(htable), 0, new.nbObj * ZRHTABLE_LVOBJSIZE(htable));
		reinsertBuckets(htable, lastTable, lastCapacity, lastNbBuckets);
	}
}

// ============================================================================

static void finitMap(ZRMap *map)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	moreSize(htable);
}

static void fdone(ZRMap *map)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	ZRFREE(htable->allocator, ZRHTABLE_LVPARRAY(htable));
	ZRVECTOR_DESTROY(htable->bucketPos);
}

void fdestroy(ZRMap *map)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	ZRMap_done(map);
	ZRAllocator *allocator = htable->allocator;

	if (htable->staticStrategy == 0)
		ZRFREE(allocator, ZRHASHTABLESTRATEGY(htable));

	ZRFREE(allocator, htable);
}

ZRMUSTINLINE
static inline void insertInBucket(ZRHashTable *htable, ZRHashTableBucket *bucket, void *key, void *obj, size_t pos)
{
	memcpy(bucket_key(htable, bucket), key, ZRHASHTABLE_MAP(htable)->keyInfos.size);
	memcpy(bucket_obj(htable, bucket), obj, ZRHASHTABLE_MAP(htable)->objInfos.size);
	bucket_flags(htable, bucket) = FLAG_NONE;
	ZRRESERVEOPLIST_RESERVENB(ZRHTABLE_LVPARRAY(htable),
		htable->bucketInfos[ZRHashTableBucketInfos_struct].size, ZRHTABLE_LVSIZE(htable),
		htable->bucketInfos[ZRHashTableBucketInfos_nextUnused].offset,
		pos, 1
		);
	ZRHASHTABLE_MAP(htable)->nbObj++;
	ZRHTABLE_LVNBOBJ(htable)++;

	ZRVECTOR_ADD(htable->bucketPos, &pos);
}

#define insert_return(B) ZRBLOCK( \
	if (out) \
		*out = bucket_obj(htable, bucket); \
	return B; \
)

ZRMUSTINLINE
static inline bool insert_resolveCollision(ZRHashTable *htable, void *key, void *obj, void **out, size_t pos, size_t nb)
{
	size_t const bucketSize = htable->bucketInfos[ZRHashTableBucketInfos_struct].size;

	for (; pos < nb; pos++)
	{
		ZRHashTableBucket *bucket = ZRARRAYOP_GET(ZRHTABLE_LVPARRAY(htable), bucketSize, pos);

		if (bucket_nextUnused(htable, bucket) != 0)
			continue;

		insertInBucket(htable, bucket, key, obj, pos);
		insert_return(true);
	}
	return false;
}

ZRMUSTINLINE
static inline bool insert(ZRHashTable *htable, void *key, void *obj, enum InsertModeE mode, void **out)
{
	size_t pos;
	zrfuhash *fuhash = htable->fuhash;
	ZRHashTableBucket *bucket = NULL;
	size_t const bucketSize = htable->bucketInfos[ZRHashTableBucketInfos_struct].size;

	if (out)
		*out = NULL;

	while (*fuhash)
	{
		pos = htable->fhashPos(htable, *fuhash, key);
		bucket = ZRARRAYOP_GET(ZRHTABLE_LVPARRAY(htable), bucketSize, pos);

		/* Empty bucket found */
		if (bucket_nextUnused(htable, bucket) == 0)
		{
			if (mode == REPLACE)
				insert_return(false);

			insertInBucket(htable, bucket, key, obj, pos);
			insert_return(true);
		}
		/* Used bucket found */
		else if (htable->fcmp(htable, htable->fucmp, key, bucket_key(htable, bucket)) == 0)
		{
			if (mode == PUTIFABSENT)
				insert_return(false);

			memcpy(bucket_obj(htable, bucket), obj, ZRHASHTABLE_MAP(htable)->objInfos.size);
			insert_return(true);
		}
		fuhash++;
	}

	if (bucket == NULL)
		pos = 0;
	else
		pos++;

	/* Collision resolution */
	size_t const c = ZRHTABLE_LVSIZE(htable);

	if (insert_resolveCollision(htable, key, obj, out, pos, c))
		return true;

	return insert_resolveCollision(htable, key, obj, out, 0, pos);
}

static void fputGrow(ZRMap *map, void *key, void *obj, void **out)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);

	if (mustGrow(htable))
		moreSize(htable);

	insert(htable, key, obj, PUT, out);
}

static bool fputIfAbsentGrow(ZRMap *map, void *key, void *obj, void **out)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);

	if (mustGrow(htable))
		moreSize(htable);

	return insert(htable, key, obj, PUTIFABSENT, out);
}

static bool freplaceGrow(ZRMap *map, void *key, void *obj, void **out)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);

	if (mustGrow(htable))
		moreSize(htable);

	return insert(htable, key, obj, REPLACE, out);
}

static void fput(ZRMap *map, void *key, void *obj, void **out)
{
	insert(ZRHASHTABLE(map), key, obj, PUT, out);
}

static bool fputIfAbsent(ZRMap *map, void *key, void *obj, void **out)
{
	return insert(ZRHASHTABLE(map), key, obj, PUTIFABSENT, out);
}

static bool freplace(ZRMap *map, void *key, void *obj, void **out)
{
	return insert(ZRHASHTABLE(map), key, obj, REPLACE, out);
}

static inline void* getBucket_resolveCollision(ZRHashTable *htable, void *key, size_t *outPos, size_t pos, size_t nb)
{
	size_t const bucketSize = htable->bucketInfos[ZRHashTableBucketInfos_struct].size;

	/* Not found, iterate */
	for (; pos < nb; pos++)
	{
		ZRHashTableBucket *bucket = ZRARRAYOP_GET(ZRHTABLE_LVPARRAY(htable), bucketSize, pos);

		if (bucket_nextUnused(htable, bucket) == 0)
		{
			/* Bucket empty but deleted : nothing to test */
			if (bucket_flags(htable,bucket) & FLAG_DELETED)
				continue;

			break;
		}

		if (htable->fcmp(htable, htable->fucmp, key, bucket_key(htable, bucket)) == 0)
		{
			if (outPos)
				*outPos = pos;

			return bucket;
		}
	}
	return NULL ;
}

static inline void* getBucket(ZRHashTable *htable, void *key, size_t *outPos)
{
	ZRHashTableBucket *bucket = NULL;
	size_t const bucketSize = htable->bucketInfos[ZRHashTableBucketInfos_struct].size;
	zrfuhash *fuhash = htable->fuhash;
	size_t pos;

	while (*fuhash)
	{
		pos = htable->fhashPos(htable, *fuhash, key);
		bucket = ZRARRAYOP_GET(ZRHTABLE_LVPARRAY(htable), bucketSize, pos);

		if (bucket_nextUnused(htable,bucket) == 0)
			;
		else if (htable->fcmp(htable, htable->fucmp, key, bucket_key(htable, bucket)) == 0)
		{
			if (outPos)
				*outPos = pos;

			return bucket;
		}
		fuhash++;
	}

	if (bucket == NULL)
		pos = 0;
	else
		pos++;

	size_t const c = ZRHTABLE_LVSIZE(htable);
	bucket = getBucket_resolveCollision(htable, key, outPos, pos, c);

	if (bucket)
		return bucket;

	return getBucket_resolveCollision(htable, key, outPos, 0, pos);
}

static void* fget(ZRMap *map, void *key)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	ZRHashTableBucket *const bucket = getBucket(htable, key, NULL);

	if (bucket == NULL)
		return NULL ;

	return bucket_obj(htable, bucket);
}

static int cmp_size_t(void *a, void *b, void *htable)
{
	return zrfcmp_size_t(a, b);
}

ZRMUSTINLINE
static inline bool delete(ZRHashTable *htable, void *key, void *cpy_out)
{
	size_t pos;
	ZRHashTableBucket *const bucket = getBucket(htable, key, &pos);

	if (bucket == NULL)
		return false;

	if (cpy_out)
		memcpy(cpy_out, bucket_obj(htable, bucket), ZRHASHTABLE_MAP(htable)->objInfos.size);

	size_t bucketSize = htable->bucketInfos[ZRHashTableBucketInfos_struct].size;
	ZRRESERVEOPLIST_RELEASENB(ZRHTABLE_LVPARRAY(htable), bucketSize, ZRHTABLE_LVSIZE(htable),
		htable->bucketInfos[ZRHashTableBucketInfos_nextUnused].offset,
		pos, 1
		);
	bucket_flags(htable, bucket) = FLAG_DELETED;

	size_t *bucketPos_p = ZRARRAYOP_SEARCH(ZRARRAY_OON(htable->bucketPos->array), &pos, cmp_size_t, htable);
	assert(bucketPos_p != NULL);
	ZRVECTOR_DELETE(htable->bucketPos, ((char*)bucketPos_p - (char*)ZRVECTOR_LVPARRAY(htable->bucketPos)) / bucketSize);
	ZRHASHTABLE_MAP(htable)->nbObj--;
	ZRHTABLE_LVNBOBJ(htable)--;
	return true;
}

static bool fdeleteShrink(ZRMap *map, void *key, void *cpy_out)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);

	if (mustShrink(htable))
		lessSize(htable);

	return delete(htable, key, cpy_out);
}

static bool fdelete(ZRMap *map, void *key, void *cpy_out)
{
	return delete(ZRHASHTABLE(map), key, cpy_out);
}

static void fdeleteAll(ZRMap *map)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	size_t bucketSize = htable->bucketInfos[ZRHashTableBucketInfos_struct].size;
	ZRRESERVEOPLIST_INIT(ZRHTABLE_LVPARRAY(htable), bucketSize, ZRHTABLE_LVSIZE(htable),
		htable->bucketInfos[ZRHashTableBucketInfos_nextUnused].offset
		);
	ZRVECTOR_DELETE_ALL(htable->bucketPos);
	ZRHASHTABLE_MAP(htable)->nbObj = 0;
	ZRHTABLE_LVNBOBJ(htable) = 0;
}

// ============================================================================

typedef struct
{
	ZRObjAlignInfos infos[ZRHASHTABLESTRUCTINFOS_NB];
	ZRObjAlignInfos bucketInfos[ZRHASHTABLEBUCKETINFOS_NB];

	zrfucmp fucmp;
	zrfuhash *fuhash;
	zrfuhash default_fuhash;
	size_t nbfhash;

	void *tableInitInfos;
	ZRVector *table;
	ZRAllocator *allocator;

	unsigned staticStrategy :1;
	unsigned dereferenceKey :1;
} ZRHashTableInitInfos;

static void tableInitInfos(void *tableInfos_out, ZRHashTableInitInfos *initInfos)
{
	ZRVector2SideStrategyInfos_dynamic(tableInfos_out, 1024, 1024, ZROBJALIGNINFOS_SIZE_ALIGNMENT(initInfos->bucketInfos[ZRHashTableBucketInfos_struct]), initInfos->allocator);
}

static void hashTableStructInfos(ZRObjAlignInfos *out, size_t nbfhash, ZRObjInfos *tableInfos, bool staticStrategy)
{
	out[ZRHashTableStructInfos_base] = (ZRObjAlignInfos ) { 0, ZRTYPE_ALIGNMENT_SIZE(ZRHashTable) };
	out[ZRHashTableStructInfos_fhash] = (ZRObjAlignInfos ) { 0, alignof(zrfuhash), sizeof(zrfuhash) * (nbfhash + 1) };
	out[ZRHashTableStructInfos_table] = tableInfos != NULL ? (ZRObjAlignInfos ) { 0, ZROBJINFOS_ALIGNMENT_SIZE(*tableInfos) } : (ZRObjAlignInfos ) { };
	out[ZRHashTableStructInfos_strategy] = staticStrategy ? (ZRObjAlignInfos ) { 0, ZRTYPE_ALIGNMENT_SIZE(ZRHashTableStrategy) } : (ZRObjAlignInfos ) { };
	out[ZRHashTableStructInfos_struct] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsetsPos(ZRHASHTABLESTRUCTINFOS_NB - 1, out, 1);
}

static void hashTableStructInfos_validate(ZRHashTableInitInfos *initInfos)
{
	size_t const tableInfosSize = ZRVector2SideStrategyInfos_objInfos().size;

	if (initInfos->tableInitInfos == NULL)
		initInfos->tableInitInfos = malloc(tableInfosSize);

	ZRObjInfos tableObjInfos;

	tableInitInfos(initInfos->tableInitInfos, initInfos);
	tableObjInfos = ZRVector2SideStrategy_objInfos(initInfos->tableInitInfos);

	hashTableStructInfos(initInfos->infos, initInfos->nbfhash, &tableObjInfos, (bool)initInfos->staticStrategy);
}

void ZRHashTableInfos( //
	void *infos_out, //
	ZRObjInfos key, ZRObjInfos obj,
	zrfuhash fuhash[], //
	size_t nbfhash, //
	ZRVector *table, //
	ZRAllocator *allocator //
	)
{
	ZRHashTableInitInfos *initInfos = infos_out;

	if (nbfhash == 0)
	{
		nbfhash = 1;
		fuhash = &initInfos->default_fuhash;
	}
	*initInfos = (ZRHashTableInitInfos )
		{ //
		.fuhash = fuhash,
		.nbfhash = nbfhash,
		.table = table,
		.allocator = allocator,
		.fucmp = default_fucmp,
		.default_fuhash = default_fuhash,
		};
	bucketInfos(initInfos->bucketInfos, key, obj);
}

void ZRHashTableInfos_dereferenceKey(void *infos_out)
{
	ZRHashTableInitInfos *initInfos = infos_out;
	initInfos->dereferenceKey = 1;
}

ZRObjInfos ZRHashTableInfos_objInfos(void)
{
	ZRObjInfos ret = { ZRTYPE_ALIGNMENT_SIZE(ZRHashTableInitInfos) };
	return ret;
}

ZRObjInfos ZRHashTable_objInfos(void *infos)
{
	ZRHashTableInitInfos *initInfos = (ZRHashTableInitInfos*)infos;
	return ZROBJALIGNINFOS_CPYOBJINFOS(initInfos->infos[ZRHashTableStructInfos_struct]);
}

void ZRHashTableInfos_staticStrategy(void *infos_out)
{
	ZRHashTableInitInfos *initInfos = infos_out;
	initInfos->staticStrategy = 1;
	hashTableStructInfos_validate(initInfos);
}

void ZRHashTableInfos_fucmp(void *infos_out, zrfucmp fucmp)
{
	ZRHashTableInitInfos *initInfos = infos_out;
	initInfos->fucmp = fucmp;
}

static void ZRHashTableStrategy_init(ZRMapStrategy *strategy)
{
	*(ZRHashTableStrategy*)strategy = (ZRHashTableStrategy ) { //
		.map = { //
			.finitMap = finitMap, //
			.fput = fputGrow, //
			.fputIfAbsent = fputIfAbsentGrow, //
			.freplace = freplaceGrow, //
			.fget = fget, //
			.fdelete = fdeleteShrink, //
			.fdeleteAll = fdeleteAll,
			.fdone = fdone, //
			.fdestroy = fdone, //
			}, //
		};
}

ZRMap* ZRHashTable_new(void *initInfos_p)
{
	ZRHashTableInitInfos *initInfos = initInfos_p;
	hashTableStructInfos_validate(initInfos);
	ZRHashTable *ret = ZRALLOC(initInfos->allocator, initInfos->infos[ZRHashTableStructInfos_struct].size);
	ZRHashTable_init(ZRHASHTABLE_MAP(ret), initInfos);
	ZRHASHTABLESTRATEGY_MAP(ZRHASHTABLESTRATEGY(ret))->fdestroy = fdestroy;
	return ZRHASHTABLE_MAP(ret);
}

void ZRHashTable_init(ZRMap *map, void *initInfos_p)
{
	ZRMapStrategy *strategy;
	ZRHashTableInitInfos *initInfos = initInfos_p;
	ZRHashTable *htable = (ZRHashTable*)map;

	hashTableStructInfos_validate(initInfos);

	*htable = (ZRHashTable ) { //
		.fhashPos = initInfos->dereferenceKey ? ptr_hashPos : hashPos,
		.fcmp = initInfos->dereferenceKey ? ptr_cmp : cmp,
		.fuhash = (zrfuhash*)((char*)htable + initInfos->infos[ZRHashTableStructInfos_fhash].offset),
		.nbfhash = initInfos->nbfhash,
		.allocator = initInfos->allocator,
		.staticStrategy = initInfos->staticStrategy,
		.fucmp = initInfos->fucmp,
		.table = (ZRArray ) { //
			.objInfos = ZROBJALIGNINFOS_CPYOBJINFOS(initInfos->bucketInfos[ZRHashTableBucketInfos_struct]),
			},
		.resizeData = (ZRResizeData ) { //
			.growStrategy = (ZRResizeGrowStrategy ) { ZRResizeOp_limit_75, ZRResizeOp_increase_100 },
			.shrinkStrategy = (ZRResizeShrinkStrategy ) { ZRResizeOp_limit_90, ZRResizeOp_limit_50 },
			.initialNb = DEFAULT_CAPACITY
			},
		.bucketPos = ZRVector2SideStrategy_createDynamic(DEFAULT_CAPACITY, ZRTYPE_SIZE_ALIGNMENT(size_t), initInfos->allocator),
		};
	memcpy(htable->bucketInfos, initInfos->bucketInfos, sizeof(ZRObjAlignInfos[ZRHASHTABLEBUCKETINFOS_NB]));
	memcpy(htable->fuhash, initInfos->fuhash, sizeof(zrfuhash) * initInfos->nbfhash);

// There must be a guard
	htable->fuhash[initInfos->nbfhash] = NULL;

	if (initInfos->staticStrategy)
		strategy = (void*)((char*)htable + initInfos->infos[ZRHashTableStructInfos_strategy].offset);
	else
		strategy = ZRALLOC(initInfos->allocator, sizeof(ZRHashTableStrategy));

	ZRHashTableStrategy_init(strategy);

	free(initInfos->tableInitInfos);
	initInfos->tableInitInfos = NULL;
	ZRMAP_INIT(ZRHASHTABLE_MAP(htable),
		ZROBJALIGNINFOS_CPYOBJINFOS(initInfos->bucketInfos[ZRHashTableBucketInfos_key]),
		ZROBJALIGNINFOS_CPYOBJINFOS(initInfos->bucketInfos[ZRHashTableBucketInfos_obj]),
		strategy);
}

ZRMap* ZRHashTable_create(
	ZRObjInfos key, ZRObjInfos obj,
	zrfuhash fuhash[], //
	size_t nbfhash, //
	ZRVector *table, //
	ZRAllocator *allocator //
	)
{
	ZRHashTableInitInfos initInfos;
	ZRHashTableInfos(&initInfos, key, obj, fuhash, nbfhash, table, allocator);
	return ZRHashTable_new(&initInfos);
}
