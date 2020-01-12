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
typedef struct ZRHashTableDataS ZRHashTableData;
typedef struct ZRHashTableStrategyS ZRHashTableStrategy;

struct ZRHashTableStrategyS
{
	ZRMapStrategy strategy;
};

// ============================================================================

#define ZRHASHTABLE_STRATEGY(htable) ((ZRHashTableStrategy*)((htable)->strategy))
#define ZRHASHTABLE_DATA(htable) ((ZRHashTableData*)((htable)->sdata))
#define ZRHASHTABLEDATA_SIZE(NBFHASH) (sizeof(ZRHashTableData) + ((NBFHASH + 1) * sizeof(fhash_t)))
#define ZRHASHTABLE_DATASIZE(HTABLE) ZRHASHTABLEDATA_SIZE(ZRHASHTABLE_DATA(HTABLE)->nbfhash)

#define ZRHASHTABLE_INFOS_NB 4
typedef enum
{
	ZRHashTableInfos_nextUnused, ZRHashTableInfos_key, ZRHashTableInfos_obj, ZRHashTableInfos_struct
} ZRHashTableInfos;

struct ZRHashTableDataS
{
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

//#define bucket_offsetof(offset, alignment) \
//	return ZRSTRUCT_ALIGNOFFSET(offset, alignment);

#define bucket_get(HTABLE,BUCKET,FIELD) ((char*)bucket + ZRHASHTABLE_DATA(HTABLE)->infos[FIELD].offset)
#define bucket_key(HTABLE,BUCKET) bucket_get(HTABLE,BUCKET,ZRHashTableInfos_key)
#define bucket_obj(HTABLE,BUCKET) bucket_get(HTABLE,BUCKET,ZRHashTableInfos_obj)
#define bucket_nextUnused(HTABLE,BUCKET) (*(ZRReserveNextUnused*)bucket_get(HTABLE,BUCKET,ZRHashTableInfos_nextUnused))

static void bucketInfos(ZRObjAlignInfos *out, size_t keySize, size_t keyAlignment, size_t objSize, size_t objAlignment)
{
	out[0] = (ZRObjAlignInfos ) { 0, alignof(ZRReserveNextUnused), sizeof(ZRReserveNextUnused) };
	out[1] = (ZRObjAlignInfos ) { 0, keyAlignment, keySize };
	out[2] = (ZRObjAlignInfos ) { 0, objAlignment, objSize };
	out[3] = (ZRObjAlignInfos ) { };
	ZRObjAlignInfos (*pinfos[ZRHASHTABLE_INFOS_NB]);

	ZRSTRUCT_BESTOFFSETS(ZRHASHTABLE_INFOS_NB - 1, out, pinfos);
}

// ============================================================================

static size_t fsdataSize(ZRMap *htable)
{
	return ZRHASHTABLE_DATASIZE(htable);
}

static size_t fstrategySize()
{
	return sizeof(ZRHashTableStrategy);
}

static void finitMap(ZRMap *htable)
{
	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);
	size_t const bucketSize = data->infos[ZRHashTableInfos_struct].size;
	alignas (max_align_t)
	char bucket[bucketSize];
	memset(bucket, 0, bucketSize);
	ZRVector_fill(data->table, 0, data->table->capacity, &bucket);
}

static void fdone(ZRMap *htable)
{
	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);
	ZRVector_destroy(data->table);
}

enum InsertModeE
{
	PUT, REPLACE, PUTIFABSENT
};

static inline bool insert(ZRMap *htable, void *key, void *obj, enum InsertModeE mode)
{
	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);

// TODO mustgrow

	fhash_t *fhash = data->fhash;
	ZRHashTableBucket *bucket = NULL;

	while (*fhash)
	{
		size_t const hash = (*fhash)(key);
		size_t const pos = hash % data->table->nbObj;
		bucket = ZRVECTOR_GET(data->table, pos);

		// Empty bucket found
		if (bucket_nextUnused(htable,bucket) == 0)
		{
			if (mode == REPLACE)
				return false;

			memcpy(bucket_key(htable, bucket), key, htable->keySize);
			memcpy(bucket_obj(htable, bucket), obj, htable->objSize);
			ZRRESERVEOPLIST_RESERVENB(data->table->array, data->infos[ZRHashTableInfos_struct].size, data->table->nbObj, data->infos[ZRHashTableInfos_nextUnused].offset, pos, 1);
			goto end;
		}
		else if (memcmp(key, bucket_key(htable, bucket), htable->keySize) == 0)
		{
			if (mode == PUTIFABSENT)
				return false;

			memcpy(bucket_obj(htable, bucket), obj, htable->objSize);
			goto end;
		}
		fhash++;
	}

	if (bucket == NULL)
	{
		bucket = data->table->array;
		bucket += bucket_nextUnused(htable, bucket);
	}
	else
	{
		bucket += bucket_nextUnused(htable,bucket) % data->table->nbObj;
	}
	end:
	htable->nbObj++;
	return true;
}

static void fput(ZRMap *htable, void *key, void *obj)
{
	insert(htable, key, obj, PUT);
}

static bool fputIfAbsent(ZRMap *htable, void *key, void *obj)
{
	return insert(htable, key, obj, PUTIFABSENT);
}

static bool freplace(ZRMap *htable, void *key, void *obj)
{
	return insert(htable, key, obj, REPLACE);
}

static inline void* getBucket(ZRMap *htable, void *key, size_t *outPos)
{
	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);
	fhash_t *fhash = data->fhash;

	while (*fhash)
	{
		size_t const hash = (*fhash)(key);
		size_t const pos = hash % data->table->nbObj;
		ZRHashTableBucket *const bucket = ZRVECTOR_GET(data->table, pos);

		if (bucket_nextUnused(htable,bucket) == 0)
			;
		else if (memcmp(key, bucket_key(htable, bucket), htable->keySize) == 0)
		{
			if (outPos)
				*outPos = pos;

			return bucket;
		}
		fhash++;
	}
	return NULL ;
}

static void* fget(ZRMap *htable, void *key)
{
	ZRHashTableBucket *const bucket = getBucket(htable, key, NULL);

	if (bucket == NULL)
		return NULL ;

	return bucket_obj(htable, bucket);
}

static bool fdelete(ZRMap *htable, void *key)
{
	size_t pos;
	ZRHashTableBucket *const bucket = getBucket(htable, key, &pos);

	if (bucket == NULL)
		return false;

	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);

	ZRRESERVEOPLIST_RELEASENB(data->table->array, data->infos[ZRHashTableInfos_struct].size, data->table->nbObj, data->infos[ZRHashTableInfos_nextUnused].offset, pos, 1);
	htable->nbObj--;
	return true;
}

// ============================================================================

static void ZRHashTableStrategy_init(ZRMapStrategy *strategy)
{
	*(ZRHashTableStrategy*)strategy = (ZRHashTableStrategy ) { //
		.strategy = { //
			.fsdataSize = fsdataSize, //
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

static void ZRHashTable_init(size_t keySize, size_t keyAlignment, size_t objSize, size_t objAlignment, ZRMap *htable, size_t nbfhash, fhash_t fhash[nbfhash], ZRVector *table, ZRAllocator *allocator)
{
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);
	memcpy(data->fhash, fhash, nbfhash * sizeof(fhash_t));
	data->nbfhash = nbfhash;
	data->fhash[nbfhash] = NULL;
	data->allocator = allocator,

	bucketInfos(data->infos, keySize, keyAlignment, objSize, objAlignment);

	//TODO: when table is set change the table size
	if (table == NULL)
		table = ZRVector2SideStrategy_createDynamic(1024, data->infos[ZRHashTableInfos_struct].size, allocator);

	data->table = table;
}

// ============================================================================

#include "HashTable_help.c"
