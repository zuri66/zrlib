/**
 * @author zuri
 * @date mardi 19 novembre 2019, 21:52:33 (UTC+0100)
 */

#include <zrlib/base/Map/HashTable.h>
#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <stdint.h>
#include <stdbool.h>

typedef struct ZRHashTableStrategyS ZRHashTableStrategy;

struct ZRHashTableStrategyS
{
	ZRMapStrategy strategy;

	ZRAllocator *allocator;

	size_t nbfhash;

	fhash_t fhash;

	/**
	 * @optional
	 */
	void (*ftable_destroy)(ZRVector*);
};

// ============================================================================

#define ZRHASHTABLE_STRATEGY(htable) ((ZRHashTableStrategy*)((htable)->strategy))
#define ZRHASHTABLE_DATA(htable) ((ZRHashTableData*)((htable)->sdata))

#define TYPEDEF_SDATA(NBFHASH) typedef STRUCT_SDATA(NBFHASH) ZRHashTableData
#define STRUCT_SDATA(NBFHASH) \
struct \
{ \
	ZRVector *table; \
	fhash_t fhash[NBFHASH+1]; \
}

#define STRATEGY_TYPEDEF_BUCKET(strategy) \
	TYPEDEF_BUCKET(\
		strategy->keySize, \
		strategy->objSize \
		)
#define TYPEDEF_BUCKET(KeySize, ObjSize) typedef STRUCT_BUCKET(KeySize, ObjSize) ZRHashTableBucket
#define STRUCT_BUCKET(KeySize, ObjSize) \
struct \
{ \
	uint_fast32_t nextUnused; \
	char key[KeySize]; \
	struct { char obj[ObjSize]; }; \
}

// ============================================================================

static size_t fsdataSize(ZRMap *htable)
{
	TYPEDEF_SDATA(ZRHASHTABLE_STRATEGY(htable)->nbfhash);
	return sizeof(ZRHashTableData);
}

static size_t fstrategySize()
{
	return sizeof(ZRHashTableStrategy);
}

static void finitMap(ZRMap *htable)
{
	TYPEDEF_BUCKET(htable->keySize, htable->objSize);
	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	TYPEDEF_SDATA(strategy->nbfhash);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);
	ZRHashTableBucket bucket;
	memset(&bucket, 0, sizeof(ZRHashTableBucket));
	ZRVector_fill(data->table, 0, data->table->capacity, &bucket);
}

static void fdone(ZRMap *htable)
{
	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	TYPEDEF_SDATA(strategy->nbfhash);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);
	ZRVector_done(data->table);

	if (strategy->ftable_destroy)
		strategy->ftable_destroy(data->table);
}

static inline void add_updateBuckets(ZRMap *htable, void *vbucket)
{
	TYPEDEF_BUCKET(htable->keySize, htable->objSize);
	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	TYPEDEF_SDATA(strategy->nbfhash);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);
	ZRVector *table = data->table;
	ZRHashTableBucket *bucket = vbucket;
	ZRHashTableBucket *nextBucket = bucket + 1;

	// Over the array space
	if (nextBucket == ZRVECTOR_GET(data->table, data->table->nbObj))
		bucket->nextUnused = ((ZRHashTableBucket*)data->table->array)->nextUnused + 1;
	else
		bucket->nextUnused = (bucket + 1)->nextUnused + 1;

	// Update the previous chain of buckets

	if (bucket == table->array)
		return;

	size_t i = 1;
	ZRHashTableBucket *prevBucket = bucket - 1;

	while (prevBucket != table->array && prevBucket->nextUnused != 0)
	{
		prevBucket->nextUnused = bucket->nextUnused + i;
		prevBucket--;
		i++;
	}
}

enum InsertModeE
{
	PUT, REPLACE, PUTIFABSENT
};

static inline bool insert(ZRMap *htable, void *key, void *obj, enum InsertModeE mode)
{
	TYPEDEF_BUCKET(htable->keySize, htable->objSize);
	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	TYPEDEF_SDATA(strategy->nbfhash);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);

	fhash_t *fhash = data->fhash;
	ZRHashTableBucket *bucket = NULL;

	while (*fhash)
	{
		size_t const hash = (*fhash)(key);
		size_t const pos = hash % data->table->nbObj;
		bucket = ZRVECTOR_GET(data->table, pos);

		// Empty bucket found
		if (bucket->nextUnused == 0)
		{
			if (mode == REPLACE)
				return false;

			memcpy(bucket->key, key, htable->keySize);
			memcpy(bucket->obj, obj, htable->objSize);
			add_updateBuckets(htable, bucket);
			goto end;
		}
		else if (memcmp(key, bucket->key, htable->keySize) == 0)
		{
			if (mode == PUTIFABSENT)
				return false;

			memcpy(bucket->obj, obj, htable->objSize);
			goto end;
		}
		fhash++;
	}

	if (bucket == NULL)
	{
		bucket = data->table->array;
		bucket += bucket->nextUnused;
	}
	else
	{
		bucket += bucket->nextUnused;

		// The end of the vector is reached
		if (bucket == ZRVECTOR_GET(data->table, data->table->nbObj - 1))
		{
			ZRHashTableBucket *first = data->table->array;
			bucket = first + first->nextUnused;
		}
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

static inline void* getBucket(ZRMap *htable, void *key)
{
	TYPEDEF_BUCKET(htable->keySize, htable->objSize);
	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	TYPEDEF_SDATA(strategy->nbfhash);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);
	fhash_t *fhash = data->fhash;

	while (*fhash)
	{
		size_t const hash = (*fhash)(key);
		size_t const pos = hash % data->table->nbObj;
		ZRHashTableBucket *const bucket = ZRVECTOR_GET(data->table, pos);

		if (bucket->nextUnused == 0)
			;
		else if (memcmp(key, bucket->key, htable->keySize) == 0)
			return bucket;

		fhash++;
	}
	return NULL ;
}

static void* fget(ZRMap *htable, void *key)
{
	TYPEDEF_BUCKET(htable->keySize, htable->objSize);
	ZRHashTableBucket *const bucket = getBucket(htable, key);

	if (bucket == NULL)
		return NULL ;

	return &bucket->obj;
}

static inline void delete_updateBuckets(ZRMap *htable, void *vbucket)
{
	TYPEDEF_BUCKET(htable->keySize, htable->objSize);
	ZRHashTableStrategy *strategy = ZRHASHTABLE_STRATEGY(htable);
	TYPEDEF_SDATA(strategy->nbfhash);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);
	ZRVector *table = data->table;
	ZRHashTableBucket *bucket = vbucket;
	ZRHashTableBucket *nextBucket = bucket + 1;

	bucket->nextUnused = 0;

	// Update the previous chain of buckets

	if (bucket == table->array)
		return;

	size_t i = 1;
	ZRHashTableBucket *prevBucket = bucket - 1;

	while (prevBucket != table->array && prevBucket->nextUnused != 0)
	{
		prevBucket->nextUnused = i;
		prevBucket--;
		i++;
	}
}

static bool fdelete(ZRMap *htable, void *key)
{
	TYPEDEF_BUCKET(htable->keySize, htable->objSize);
	ZRHashTableBucket *const bucket = getBucket(htable, key);

	if (bucket == NULL)
		return false;

	delete_updateBuckets(htable, bucket);
	htable->nbObj--;
	return true;
}

// ============================================================================

static void ZRHashTableStrategy_init(ZRMapStrategy *strategy, void (*ftable_destroy)(ZRVector*), ZRAllocator *allocator)
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
			},//
		.allocator = allocator, //
		.ftable_destroy = ftable_destroy, //
		};
}

static void ZRHashTable_init(ZRMap *htable, size_t nbfhash, fhash_t fhash[nbfhash], ZRVector *table)
{
	TYPEDEF_SDATA(nbfhash);
	ZRHashTableData *data = ZRHASHTABLE_DATA(htable);
	data->table = table;
	memcpy(data->fhash, fhash, nbfhash * sizeof(fhash_t));
	data->fhash[nbfhash] = NULL;
}

// ============================================================================

#include "HashTable_help.c"
