/**
 * @author zuri
 * @date jeu. 30 juil. 2020 13:24:30 CEST
 */

#include <zrlib/base/Identifier/MapIdentifier.h>
#include <zrlib/base/MemoryPool/MemoryPool.h>
#include <zrlib/base/MemoryPool/MPoolDynamicStrategy.h>
#include <zrlib/base/Map/HashTable.h>

#include <stdio.h>

typedef struct
{
	ZRIdentifierStrategy identifier;
} MapIdentifierStrategy;

#define MAPIDSTRATEGY(MS) ((MapIdentifierStrategy*)(MS))
#define MAPIDSTRATEGY_ID(MS) (&(MS)->identifier)
#define MAPID_STRATEGY(M) MAPIDSTRATEGY(MAPID_ID(M)->strategy)
#define MAPID_IDSTRATEGY(M) MAPIDSTRATEGY(MAPID_ID(M)->strategy)->identifier

typedef struct
{
	ZRIdentifier identifier;

	ZRMap *map;
	ZRMap *map_ID;
	ZRMemoryPool *pool;

	ZRAllocator *allocator;

	ZRID nextID;
//	int staticStrategy :1;
} MapIdentifier;

#define MAPID(I) ((MapIdentifier*)(I))
#define MAPID_ID(M) (&(M)->identifier)

// ============================================================================

typedef struct
{
	ZRID id;
	void *objInPool;
} MapBucket;

#define MAPBUCKET(B) ((MapBucket*)(B))

// ============================================================================

ZRMUSTINLINE
static inline MapBucket* getPool_p(MapIdentifier *mapIdentifier, void *obj)
{
	void *objInPool = ZRMPOOL_RESERVE(mapIdentifier->pool);
	void *ref_p;
	MapBucket *ref;
	MapBucket cpy = { mapIdentifier->nextID, objInPool };
	memcpy(objInPool, obj, mapIdentifier->pool->blockSize);

	if (ZRMAP_PUTIFABSENTTHENGET(mapIdentifier->map, &objInPool, &cpy, &ref_p))
	{
		ref = (MapBucket*)ref_p;
		ref->objInPool = objInPool;

		ZRMAP_PUT(mapIdentifier->map_ID, &cpy.id, &cpy);

		mapIdentifier->nextID++;
		MAPID_ID(mapIdentifier)->nbObj++;
	}
	else
		ZRMPOOL_RELEASE_NB(mapIdentifier->pool, objInPool, 1);

	assert(ref_p != NULL);
	return (MapBucket*)ref_p;
}

// ============================================================================

static ZRID fgetID(ZRIdentifier *identifier, void *obj)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = getPool_p(mapIdentifier, obj);

//	if (found == NULL)
//		return ZRID_ABSENT;

	return found->id;
}

static
void* fintern(ZRIdentifier *identifier, void *obj)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = getPool_p(mapIdentifier, obj);

//	if (found == NULL)
//		return NULL;

	return found->objInPool;
}

static
void* ffromID(ZRIdentifier *identifier, ZRID id)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = (MapBucket*)ZRMAP_GET(mapIdentifier->map_ID, &id);

	if (found == NULL)
		return NULL;

	return found->objInPool;
}

static
void frelease(ZRIdentifier *identifier, void *obj)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
}

static
void freleaseID(ZRIdentifier *identifier, ZRID id)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
}

static
void freleaseAll(ZRIdentifier *identifier)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
}

static
void fdone(ZRIdentifier *identifier)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	ZRMAP_DESTROY(mapIdentifier->map);
	ZRMAP_DESTROY(mapIdentifier->map_ID);
	ZRMPOOL_DESTROY(mapIdentifier->pool);
	ZRFREE(mapIdentifier->allocator, MAPID_STRATEGY(mapIdentifier));
}

static
void fdestroy(ZRIdentifier *identifier)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	fdone(identifier);
	ZRFREE(mapIdentifier->allocator, mapIdentifier);
}

// ============================================================================

static void MapIdentifierStrategy_init(MapIdentifierStrategy *strategy)
{
	*strategy = (MapIdentifierStrategy ) { //
		.identifier = (ZRIdentifierStrategy ) { //
			.fgetID = fgetID,
			.fintern = fintern,
			.ffromID = ffromID,
			.frelease = frelease,
			.freleaseID = freleaseID,
			.freleaseAll = freleaseAll,
			.fdone = fdone,
			.fdestroy = fdone,
			} ,
		};
}

// ============================================================================

typedef struct
{
	ZRObjInfos objInfos;
	ZRAllocator *allocator;
	zrfuhash *fuhash;
	size_t nbfhash;
} MapIdentifierInitInfos;

ZRObjInfos ZRMapIdentifierInfos_objInfos(void)
{
	return ZRTYPE_OBJINFOS(MapIdentifierInitInfos);
}

ZRObjInfos ZRMapIdentifier_objInfos(void *infos)
{
	return ZRTYPE_OBJINFOS(MapIdentifier);
}

void ZRMapIdentifierInfos(void *infos_out, ZRObjInfos objInfos, zrfuhash *fuhash, size_t nbfhash, ZRAllocator *allocator)
{
	MapIdentifierInitInfos *initInfos = (MapIdentifierInitInfos*)infos_out;

	*initInfos = (MapIdentifierInitInfos ) { //
		.objInfos = objInfos,
		.allocator = allocator,
		.fuhash = fuhash,
		.nbfhash = nbfhash,
		};

	initInfos->nbfhash = nbfhash;
	initInfos->fuhash = fuhash;
}

void ZRMapIdentifierStrategy_init(MapIdentifierStrategy *strategy)
{
	*strategy = (MapIdentifierStrategy )
		{ //
		.identifier = (ZRIdentifierStrategy )
			{ //
			.fgetID = fgetID,
			.fintern = fintern,
			.ffromID = ffromID,

			.frelease = frelease,
			.freleaseID = freleaseID,
			.freleaseAll = freleaseAll,

			.fdone = fdone,
			.fdestroy = fdone,
			} ,
		};
}

void ZRMapIdentifier_init(ZRIdentifier *identifier, void *infos)
{
	alignas(max_align_t) char infoBuffer[2048];
	MapIdentifier *mapIdentifier = MAPID(identifier);
	MapIdentifierInitInfos *initInfos = (MapIdentifierInitInfos*)infos;
	MapIdentifierStrategy *strategy = ZRALLOC(initInfos->allocator, sizeof(MapIdentifierStrategy));
	ZRMap *map, *map_ID;
	ZRMemoryPool *pool;

	/* Map init */
	{
		ZRObjInfos init_objInfos = ZRHashTableInfos_objInfos();
		assert(init_objInfos.size <= ZRCARRAY_NBOBJ(infoBuffer));

		ZRHashTableInfos(infoBuffer, ZRTYPE_OBJINFOS(void*), ZRTYPE_OBJINFOS(MapBucket), initInfos->fuhash, initInfos->nbfhash, NULL, initInfos->allocator);
		ZRHashTableInfos_dereferenceKey(infoBuffer);
		map = ZRHashTable_new(infoBuffer);

		ZRHashTableInfos(infoBuffer, ZRTYPE_OBJINFOS(ZRID), ZRTYPE_OBJINFOS(MapBucket), NULL, 0, NULL, initInfos->allocator);
		map_ID = ZRHashTable_new(infoBuffer);
	}
	/* Pool init */
	{
		pool = ZRMPoolDS_createDefault(ZROBJALIGNINFOS_SIZE_ALIGNMENT(initInfos->objInfos), initInfos->allocator);
	}
	ZRMapIdentifierStrategy_init(strategy);
	*mapIdentifier = (MapIdentifier ) { //
		.identifier = (ZRIdentifier ) { //
			.strategy = (ZRIdentifierStrategy*)strategy,
			},
		.allocator = initInfos->allocator,
		.map = map,
		.map_ID = map_ID,
		.pool = pool,
		};
}

ZRIdentifier* ZRMapIdentifier_new(void *infos)
{
	MapIdentifierInitInfos *initInfos = (MapIdentifierInitInfos*)infos;
	MapIdentifier *ret = ZRALLOC(initInfos->allocator, sizeof(MapIdentifier));

	ZRMapIdentifier_init(MAPID_ID(ret), infos);
	MAPIDSTRATEGY_ID(MAPID_STRATEGY(ret))->fdestroy = fdestroy;
	return MAPID_ID(ret);
}

// ============================================================================
