/**
 * @author zuri
 * @date mardi 19 novembre 2019, 21:52:33 (UTC+0100)
 */

#include <zrlib/base/macro.h>
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
	ZRMapStrategy strategy;
};

// ============================================================================

#define ZRHASHTABLE_MAP(HTABLE) (&(HTABLE)->map)
#define ZRHASHTABLE(HTABLE) ((ZRHashTable*)(HTABLE))
#define ZRHASHTABLEDATA_SIZE(NBFHASH) (sizeof(ZRHashTable) + ((NBFHASH + 1) * sizeof(fhash_t)))
#define ZRHASHTABLE_DATASIZE(HTABLE) ZRHASHTABLEDATA_SIZE(ZRHASHTABLE(HTABLE)->nbfhash)

#define ZRHASHTABLE_INFOS_NB 4
typedef enum
{
	ZRHashTableInfos_nextUnused, ZRHashTableInfos_key, ZRHashTableInfos_obj, ZRHashTableInfos_struct
} ZRHashTableInfos;

struct ZRHashTableS
{
	ZRMap map;
	ZRObjAlignInfos infos[ZRHASHTABLE_INFOS_NB];
	ZRAllocator *allocator;
	ZRVector *table;
	size_t nbfhash;
	fhash_t fhash[];
};

struct ZRHashTableBucketS
{
//	ZRReserveNextUnused nextUnused;
// KType key;
// OType obj;
};

// ============================================================================

#define bucket_get(HTABLE,BUCKET,FIELD) ((char*)bucket + (HTABLE)->infos[FIELD].offset)
#define bucket_key(HTABLE,BUCKET) bucket_get(HTABLE,BUCKET,ZRHashTableInfos_key)
#define bucket_obj(HTABLE,BUCKET) bucket_get(HTABLE,BUCKET,ZRHashTableInfos_obj)
#define bucket_nextUnused(HTABLE,BUCKET) (*(ZRReserveNextUnused*)bucket_get(HTABLE,BUCKET,ZRHashTableInfos_nextUnused))

static void bucketInfos(ZRObjAlignInfos *out, size_t keySize, size_t keyAlignment, size_t objSize, size_t objAlignment)
{
	out[0] = (ZRObjAlignInfos ) { 0, alignof(ZRReserveNextUnused), sizeof(ZRReserveNextUnused) };
	out[1] = (ZRObjAlignInfos ) { 0, keyAlignment, keySize };
	out[2] = (ZRObjAlignInfos ) { 0, objAlignment, objSize };
	out[3] = (ZRObjAlignInfos ) { };
	ZRStruct_bestOffsets(ZRHASHTABLE_INFOS_NB - 1, out);
}

// ============================================================================

static size_t fstrategySize()
{
	return sizeof(ZRHashTableStrategy);
}

static void finitMap(ZRMap *map)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	size_t const bucketSize = htable->infos[ZRHashTableInfos_struct].size;
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

			memcpy(bucket_key(htable, bucket), key, map->keySize);
			memcpy(bucket_obj(htable, bucket), obj, map->objSize);
			ZRRESERVEOPLIST_RESERVENB(htable->table->array, htable->infos[ZRHashTableInfos_struct].size, htable->table->nbObj, htable->infos[ZRHashTableInfos_nextUnused].offset, pos, 1);
			goto end;
		}
		else if (memcmp(key, bucket_key(htable, bucket), map->keySize) == 0)
		{
			if (mode == PUTIFABSENT)
				return false;

			memcpy(bucket_obj(htable, bucket), obj, map->objSize);
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
		else if (memcmp(key, bucket_key(htable, bucket), ZRHASHTABLE_MAP(htable)->keySize) == 0)
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

	ZRRESERVEOPLIST_RELEASENB(htable->table->array, htable->infos[ZRHashTableInfos_struct].size, htable->table->nbObj, htable->infos[ZRHashTableInfos_nextUnused].offset, pos, 1);
	ZRHASHTABLE_MAP(htable)->nbObj--;
	return true;
}

// ============================================================================

static void ZRHashTableStrategy_init(ZRMapStrategy *strategy)
{
	*(ZRHashTableStrategy*)strategy = (ZRHashTableStrategy ) { //
		.strategy = { //
			.fstrategySize = fstrategySize, //
			.finitMap = finitMap, //
			.fput = fput, //
			.fputIfAbsent = fputIfAbsent, //
			.freplace = freplace, //
			.fget = fget, //
			.fdelete = fdelete, //
			.fdone = fdone, //
			}, //
		};
}

static void ZRHashTable_init(size_t keySize, size_t keyAlignment, size_t objSize, size_t objAlignment, ZRMap *map, size_t nbfhash, fhash_t fhash[nbfhash], ZRVector *table, ZRAllocator *allocator)
{
	ZRHashTable *const htable = ZRHASHTABLE(map);
	memcpy(htable->fhash, fhash, nbfhash * sizeof(fhash_t));
	htable->nbfhash = nbfhash;
	htable->fhash[nbfhash] = NULL;
	htable->allocator = allocator,

	bucketInfos(htable->infos, keySize, keyAlignment, objSize, objAlignment);

	if (table == NULL)
		table = ZRVector2SideStrategy_createDynamic(1024, htable->infos[ZRHashTableInfos_struct].size, htable->infos[ZRHashTableInfos_struct].alignment, allocator);
	else
		ZRVECTOR_CHANGEOBJSIZE(table, objSize, objAlignment);

	htable->table = table;
}

// ============================================================================

#include "HashTable_help.c"
