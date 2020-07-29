/**
 * @author zuri
 * @date mardi 19 novembre 2019, 21:52:33 (UTC+0100)
 */

#include <zrlib/base/struct.h>
#include <zrlib/base/ReserveOp_list.h>
#include <zrlib/base/Map/HashTable.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

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

typedef enum
{
	ZRHashTableBucketInfos_nextUnused = 0,
	ZRHashTableBucketInfos_key,
	ZRHashTableBucketInfos_obj,
	ZRHashTableBucketInfos_struct,
	ZRHASHTABLEBUCKETINFOS_NB
} ZRHashTableBucketInfos;

struct ZRHashTableS
{
	ZRMap map;
	ZRObjAlignInfos bucketInfos[ZRHASHTABLEBUCKETINFOS_NB];

	fhash_t *fhash;
	ZRVector *table;
	ZRAllocator *allocator;

	size_t nbfhash;

	unsigned staticStrategy :1;
};

struct ZRHashTableBucketS
{
//	ZRReserveNextUnused nextUnused;
// KType key;
// OType obj;
};

// ============================================================================

#define bucket_get(HTABLE,BUCKET,FIELD) ((char*)bucket + (HTABLE)->bucketInfos[FIELD].offset)
#define bucket_key(HTABLE,BUCKET) bucket_get(HTABLE,BUCKET,ZRHashTableBucketInfos_key)
#define bucket_obj(HTABLE,BUCKET) bucket_get(HTABLE,BUCKET,ZRHashTableBucketInfos_obj)
#define bucket_nextUnused(HTABLE,BUCKET) (*(ZRReserveNextUnused*)bucket_get(HTABLE,BUCKET,ZRHashTableBucketInfos_nextUnused))

static void bucketInfos(ZRObjAlignInfos *out, ZRObjInfos key, ZRObjInfos obj)
{
	out[ZRHashTableBucketInfos_nextUnused] = (ZRObjAlignInfos ) { 0, ZRTYPE_ALIGNMENT_SIZE(ZRReserveNextUnused) };
	out[ZRHashTableBucketInfos_key] = ZROBJINFOS_CPYOBJALIGNINFOS(key);
	out[ZRHashTableBucketInfos_obj] = ZROBJINFOS_CPYOBJALIGNINFOS(obj);
	out[ZRHashTableBucketInfos_struct] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsetsPos(ZRHASHTABLEBUCKETINFOS_NB - 1, out, 1);
}

// ============================================================================

static void finitMap(ZRMap *map)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	size_t const bucketSize = htable->bucketInfos[ZRHashTableBucketInfos_struct].size;
	alignas (max_align_t)
	char bucket[bucketSize];
	memset(bucket, 0, bucketSize);
	ZRVector_fill(htable->table, 0, htable->table->capacity, &bucket);
}

static void fdone(ZRMap *map)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	ZRVector_destroy(htable->table);
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

enum InsertModeE
{
	PUT, REPLACE, PUTIFABSENT
};

static inline bool insert(ZRMap *map, void *key, void *obj, enum InsertModeE mode)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);

// TODO mustgrow

	fhash_t *fhash = htable->fhash;
	ZRHashTableBucket *bucket = NULL;

	while (*fhash)
	{
		size_t const hash = (*fhash)(key);
		size_t const pos = hash % htable->table->nbObj;
		bucket = ZRVECTOR_GET(htable->table, pos);

		// Empty bucket found
		if (bucket_nextUnused(htable,bucket) == 0)
		{
			if (mode == REPLACE)
				return false;

			memcpy(bucket_key(htable, bucket), key, map->keyInfos.size);
			memcpy(bucket_obj(htable, bucket), obj, map->objInfos.size);
			ZRRESERVEOPLIST_RESERVENB(htable->table->array, htable->bucketInfos[ZRHashTableBucketInfos_struct].size, htable->table->nbObj, htable->bucketInfos[ZRHashTableBucketInfos_nextUnused].offset, pos, 1);
			goto end;
		}
		else if (memcmp(key, bucket_key(htable, bucket), map->keyInfos.size) == 0)
		{
			if (mode == PUTIFABSENT)
				return false;

			memcpy(bucket_obj(htable, bucket), obj, map->objInfos.size);
			goto end;
		}
		fhash++;
	}

	if (bucket == NULL)
	{
		bucket = htable->table->array;
		bucket += bucket_nextUnused(htable, bucket);
	}
	else
	{
		bucket += bucket_nextUnused(htable,bucket) % htable->table->nbObj;
	}
	end:
	ZRHASHTABLE_MAP(htable)->nbObj++;
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

static inline void* getBucket(ZRHashTable *htable, void *key, size_t *outPos)
{
	fhash_t *fhash = htable->fhash;

	while (*fhash)
	{
		size_t const hash = (*fhash)(key);
		size_t const pos = hash % htable->table->nbObj;
		ZRHashTableBucket *const bucket = ZRVECTOR_GET(htable->table, pos);

		if (bucket_nextUnused(htable,bucket) == 0)
			;
		else if (memcmp(key, bucket_key(htable, bucket), ZRHASHTABLE_MAP(htable)->keyInfos.size) == 0)
		{
			if (outPos)
				*outPos = pos;

			return bucket;
		}
		fhash++;
	}
	return NULL ;
}

static void* fget(ZRMap *map, void *key)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	ZRHashTableBucket *const bucket = getBucket(htable, key, NULL);

	if (bucket == NULL)
		return NULL ;

	return bucket_obj(htable, bucket);
}

static bool fdelete(ZRMap *map, void *key)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	size_t pos;
	ZRHashTableBucket *const bucket = getBucket(htable, key, &pos);

	if (bucket == NULL)
		return false;

	ZRRESERVEOPLIST_RELEASENB(htable->table->array, htable->bucketInfos[ZRHashTableBucketInfos_struct].size, htable->table->nbObj, htable->bucketInfos[ZRHashTableBucketInfos_nextUnused].offset, pos, 1);
	ZRHASHTABLE_MAP(htable)->nbObj--;
	return true;
}

// ============================================================================

typedef struct
{
	ZRObjAlignInfos infos[ZRHASHTABLESTRUCTINFOS_NB];
	ZRObjAlignInfos bucketInfos[ZRHASHTABLEBUCKETINFOS_NB];

	fhash_t *fhash;
	size_t nbfhash;

	void *tableInitInfos;
	ZRVector *table;
	ZRAllocator *allocator;

	unsigned staticStrategy :1;
} ZRHashTableInitInfos;

static void tableInitInfos(void *tableInfos_out, ZRHashTableInitInfos *initInfos)
{
	ZRVector2SideStrategyInfos_dynamic(tableInfos_out, 1024, 1024, ZROBJALIGNINFOS_SIZE_ALIGNMENT(initInfos->bucketInfos[ZRHashTableBucketInfos_struct]), initInfos->allocator);
}

static void hashTableStructInfos(ZRObjAlignInfos *out, size_t nbfhash, ZRObjInfos *tableInfos, bool staticStrategy)
{
	out[ZRHashTableStructInfos_base] = (ZRObjAlignInfos ) { 0, ZRTYPE_ALIGNMENT_SIZE(ZRHashTable) };
	out[ZRHashTableStructInfos_fhash] = (ZRObjAlignInfos ) { 0, alignof(fhash_t), sizeof(fhash_t) * (nbfhash + 1) };
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
	fhash_t fhash[], //
	size_t nbfhash, //
	ZRVector *table, //
	ZRAllocator *allocator //
	)
{
	ZRHashTableInitInfos *initInfos = infos_out;
	*initInfos = (ZRHashTableInitInfos )
		{ //
		.fhash = fhash,
		.nbfhash = nbfhash,
		.table = table,
		.allocator = allocator,
		};
	bucketInfos(initInfos->bucketInfos, key, obj);
	hashTableStructInfos_validate(initInfos);
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

static void ZRHashTableStrategy_init(ZRMapStrategy *strategy)
{
	*(ZRHashTableStrategy*)strategy = (ZRHashTableStrategy ) { //
		.map = { //
			.finitMap = finitMap, //
			.fput = fput, //
			.fputIfAbsent = fputIfAbsent, //
			.freplace = freplace, //
			.fget = fget, //
			.fdelete = fdelete, //
			.fdone = fdone, //
			.fdestroy = fdone, //
			}, //
		};
}

ZRMap* ZRHashTable_new(void *initInfos_p)
{
	ZRHashTableInitInfos *initInfos = initInfos_p;
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
		.fhash = (fhash_t*)((char*)htable + initInfos->infos[ZRHashTableStructInfos_fhash].offset),
		.nbfhash = initInfos->nbfhash,
		.allocator = initInfos->allocator,
		.staticStrategy = initInfos->staticStrategy,
		};
	memcpy(htable->bucketInfos, initInfos->bucketInfos, sizeof(ZRObjAlignInfos[ZRHASHTABLEBUCKETINFOS_NB]));
	memcpy(htable->fhash, initInfos->fhash, sizeof(fhash_t) * initInfos->nbfhash);

	// There must be a guard
	htable->fhash[initInfos->nbfhash] = NULL;

	if (initInfos->table)
	{
		htable->table = initInfos->table;
		ZRVECTOR_CHANGEOBJSIZE(htable->table, ZROBJALIGNINFOS_SIZE_ALIGNMENT(initInfos->bucketInfos[ZRHashTableBucketInfos_obj]));
	}
	else
	{
		htable->table = (ZRVector*)((char*)htable + initInfos->infos[ZRHashTableStructInfos_table].offset);
		ZRVector2SideStrategy_init(htable->table, initInfos->tableInitInfos);
	}

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
	fhash_t fhash[], //
	size_t nbfhash, //
	ZRVector *table, //
	ZRAllocator *allocator //
	)
{
	ZRHashTableInitInfos initInfos;
	ZRHashTableInfos(&initInfos, key, obj, fhash, nbfhash, table, allocator);
	return ZRHashTable_new(&initInfos);
}
