/**
 * @author zuri
 * @date jeu. 30 juil. 2020 13:24:30 CEST
 */

#include <zrlib/base/Identifier/MapIdentifier.h>
#include <zrlib/base/MemoryPool/MemoryPool.h>
#include <zrlib/base/MemoryPool/MPoolDynamicStrategy.h>
#include <zrlib/base/Map/HashTable.h>
#include <zrlib/base/Identifier/IDGenerator/IDGenerator.h>

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

	ZRIDGenerator *generator;

	unsigned int staticStrategy :1;
} MapIdentifier;

#define MAPID(I) ((MapIdentifier*)(I))
#define MAPID_ID(M) (&(M)->identifier)

typedef enum
{
	MapIdentifierInfos_base = 0,
	MapIdentifierInfos_generator,
	MapIdentifierInfos_strategy,
	MapIdentifierInfos_struct,
	MAPIDENTIFIERINFOS_NB,
} MapIdentifierInfos;

// ============================================================================

typedef struct
{
	ZRID id;
	void *objInPool;
} MapBucket;

#define MAPBUCKET(B) ((MapBucket*)(B))

// ============================================================================

ZRMUSTINLINE
static inline MapBucket* getBucket(MapIdentifier *mapIdentifier, void *obj,
	void (*fmakeMemory)(MapIdentifier *mapIdentifier, void *objInPool_p))
{
	void *objInPool = ZRMPOOL_RESERVE(mapIdentifier->pool);
	void *ref_p;
	MapBucket *ref;
	MapBucket cpy = { ZRIDGenerator_nextID(mapIdentifier->generator), objInPool };
	memcpy(objInPool, obj, ZRMPOOL_BLOCKSIZE(mapIdentifier->pool));

	if (ZRMAP_PUTIFABSENTTHENGET(mapIdentifier->map, &objInPool, &cpy, &ref_p))
	{
		ref = (MapBucket*)ref_p;
		ref->objInPool = objInPool;

		if (fmakeMemory)
			fmakeMemory(mapIdentifier, objInPool);

		ZRMAP_PUT(mapIdentifier->map_ID, &cpy.id, &cpy);

		ZRIDGenerator_generate(mapIdentifier->generator);
		MAPID_ID(mapIdentifier)->nbObj++;
	}
	else
		ZRMPOOL_RELEASE_NB(mapIdentifier->pool, objInPool, 1);

	assert(ref_p != NULL);
	return (MapBucket*)ref_p;
}

ZRMUSTINLINE
static inline bool release(ZRIdentifier *identifier, void *obj, void (*ffree)(MapIdentifier*, MapBucket))
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket cpy;

	if (!ZRMAP_CPYTHENDELETE(mapIdentifier->map, &obj, &cpy))
		return false;

	bool const del = ZRMAP_DELETE(mapIdentifier->map_ID, &cpy.id);
	assert(del == true);
	ZRIDGenerator_release(mapIdentifier->generator, cpy.id);
	MAPID_ID(mapIdentifier)->nbObj--;

	if (ffree)
		ffree(mapIdentifier, cpy);

	return true;
}

ZRMUSTINLINE
static inline bool releaseID(ZRIdentifier *identifier, ZRID id, void (*ffree)(MapIdentifier*, MapBucket))
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket cpy;

	if (!ZRMAP_CPYTHENDELETE(mapIdentifier->map_ID, &id, &cpy))
		return false;

	bool const del = ZRMAP_DELETE(mapIdentifier->map, &cpy.objInPool);
	assert(del == true);
	ZRIDGenerator_release(mapIdentifier->generator, id);
	MAPID_ID(mapIdentifier)->nbObj--;

	if (ffree)
		ffree(mapIdentifier, cpy);

	return true;
}

ZRMUSTINLINE
static inline bool releaseAll(ZRIdentifier *identifier, void (*ffree)(MapIdentifier*))
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);

	if (ffree)
		ffree(mapIdentifier);

	ZRMAP_DELETEALL(mapIdentifier->map);
	ZRMAP_DELETEALL(mapIdentifier->map_ID);
	MAPID_ID(mapIdentifier)->nbObj = 0;
	ZRIDGenerator_releaseAll(mapIdentifier->generator);

	return true;
}

ZRMUSTINLINE
static inline void done(ZRIdentifier *identifier, void (*ffree)(MapIdentifier*))
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);

	if (ffree)
		ffree(mapIdentifier);

	ZRMAP_DESTROY(mapIdentifier->map);
	ZRMAP_DESTROY(mapIdentifier->map_ID);
	ZRMPOOL_DESTROY(mapIdentifier->pool);
	ZRIDGenerator_destroy(mapIdentifier->generator);

	if (mapIdentifier->staticStrategy == 0)
		ZRFREE(mapIdentifier->allocator, MAPID_STRATEGY(mapIdentifier));
}

ZRMUSTINLINE
static inline void destroy(ZRIdentifier *identifier, void (*fdone)(ZRIdentifier*))
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	fdone(identifier);
	ZRFREE(mapIdentifier->allocator, mapIdentifier);
}

// ============================================================================

static void allocObject(MapIdentifier *mapIdentifier, void *objInPool_p)
{
	ZRObjectP *objInPool = objInPool_p;
	void *newObject = ZRAALLOC(mapIdentifier->allocator, ZROBJINFOS_ALIGNMENT_SIZE(objInPool->infos));
	memcpy(newObject, objInPool->object, objInPool->infos.size);
	objInPool->object = newObject;
}

static void freeObject(MapIdentifier *mapIdentifier, MapBucket bucket)
{
	ZRFREE(mapIdentifier->allocator, ZROBJECTP(bucket.objInPool)->object);
}

static void freeObjects(MapIdentifier *mapIdentifier)
{
	ZRMapKeyVal kv[256];
	size_t const kv_nb = ZRCARRAY_NBOBJ(kv);

	for (size_t offset = 0;;)
	{
		size_t nb = ZRMAP_CPYKEYVALPTR(mapIdentifier->map, kv, offset, kv_nb);

		for (size_t i = 0; i < nb; i++)
		{
			MapBucket *bucket = kv[i].val;
			ZRFREE(mapIdentifier->allocator, ZROBJECTP(bucket->objInPool)->object);
		}
		if (nb < kv_nb)
			break;

		offset += nb;
	}
}

MapBucket* getBucket_unknown(MapIdentifier *mapIdentifier, void *obj)
{
	return getBucket(mapIdentifier, obj, allocObject);
}

static ZRID fgetID_unknown(ZRIdentifier *identifier, void *obj)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = getBucket_unknown(mapIdentifier, obj);
	return found->id;
}

static
void* fintern_unknown(ZRIdentifier *identifier, void *obj)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = getBucket_unknown(mapIdentifier, obj);
	return ZROBJECTP(found->objInPool)->object;
}

static
void* ffromID_unknown(ZRIdentifier *identifier, ZRID id)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = (MapBucket*)ZRMAP_GET(mapIdentifier->map_ID, &id);

	if (found == NULL)
		return NULL ;

	return ZROBJECTP(found->objInPool)->object;
}

static ZRObjectP fobjectP_unknown(ZRIdentifier *identifier, ZRID id)
{

	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = (MapBucket*)ZRMAP_GET(mapIdentifier->map_ID, &id);

	if (found == NULL)
		return ZROBJECTP_DEF0();

	return *ZROBJECTP(found->objInPool);
}

static
bool frelease_unknown(ZRIdentifier *identifier, void *obj)
{
	return release(identifier, obj, freeObject);
}

static
bool freleaseID_unknown(ZRIdentifier *identifier, ZRID id)
{
	return releaseID(identifier, id, freeObject);
}

static
bool freleaseAll_unknown(ZRIdentifier *identifier)
{
	return releaseAll(identifier, freeObjects);
}

static
void fdone_unknown(ZRIdentifier *identifier)
{
	done(identifier, freeObjects);
}

static
void fdestroy_unknown(ZRIdentifier *identifier)
{
	destroy(identifier, fdone_unknown);
}

// ============================================================================

ZRMUSTINLINE
static inline MapBucket* _getBucket(MapIdentifier *mapIdentifier, void *obj)
{
	return getBucket(mapIdentifier, obj, NULL);
}

static ZRID fgetID(ZRIdentifier *identifier, void *obj)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = _getBucket(mapIdentifier, obj);
	return found->id;
}

static
void* fintern(ZRIdentifier *identifier, void *obj)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = _getBucket(mapIdentifier, obj);
	return found->objInPool;
}

static
void* ffromID(ZRIdentifier *identifier, ZRID id)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = (MapBucket*)ZRMAP_GET(mapIdentifier->map_ID, &id);

	if (found == NULL)
		return NULL ;

	return found->objInPool;
}

static ZRObjectP fobjectP(ZRIdentifier *identifier, ZRID id)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	MapBucket *found = (MapBucket*)ZRMAP_GET(mapIdentifier->map_ID, &id);

	if (found == NULL)
		return ZROBJECTP_DEF0();

	return ZROBJECTP_DEF(ZRMPOOL_BLOCKINFOS(mapIdentifier->pool), found->objInPool);
}

static
bool fcontains(ZRIdentifier *identifier, void *obj)
{
	MapIdentifier *const mapIdentifier = MAPID(identifier);
	return NULL != ZRMAP_GET(mapIdentifier->map, &obj);
}

static
bool frelease(ZRIdentifier *identifier, void *obj)
{
	return release(identifier, obj, NULL);
}

static
bool freleaseID(ZRIdentifier *identifier, ZRID id)
{
	return releaseID(identifier, id, NULL);
}

static
bool freleaseAll(ZRIdentifier *identifier)
{
	return releaseAll(identifier, NULL);
}

static
void fdone(ZRIdentifier *identifier)
{
	done(identifier, NULL);
}

static
void fdestroy(ZRIdentifier *identifier)
{
	destroy(identifier, fdone);
}

// ============================================================================

typedef struct
{
	alignas(max_align_t) char generatorInfos[1024];
	ZRObjAlignInfos infos[MAPIDENTIFIERINFOS_NB];
	ZRObjInfos objInfos;
	ZRAllocator *allocator;
	zrfuhash *fuhash;
	zrfucmp fucmp;
	size_t nbfhash;
	void (*fdestroy)(ZRIdentifier*);
	unsigned int staticStrategy :1;
} MapIdentifierInitInfos;

static void _MapIdentifierInfos(ZRObjAlignInfos *infos, MapIdentifierInitInfos *initInfos)
{
	ZRObjInfos generatorInfos = ZRIDGenerator_objInfos(initInfos->generatorInfos);
	infos[MapIdentifierInfos_base] = ZRTYPE_OBJALIGNINFOS(MapIdentifier);
	infos[MapIdentifierInfos_generator] = ZROBJINFOS_CPYOBJALIGNINFOS(generatorInfos);
	infos[MapIdentifierInfos_strategy] = initInfos->staticStrategy ? ZRTYPE_OBJALIGNINFOS(MapIdentifierStrategy) : ZROBJALIGNINFOS_DEF_AS(0, 0);
	infos[MapIdentifierInfos_struct] = ZROBJALIGNINFOS_DEF_AS(0, 0);
	ZRStruct_bestOffsetsPos(MAPIDENTIFIERINFOS_NB - 1, infos, 1);
}

ZRObjInfos ZRMapIdentifierInfos_objInfos(void)
{
	return ZRTYPE_OBJINFOS(MapIdentifierInitInfos);
}

ZRObjInfos ZRMapIdentifier_objInfos(void *infos)
{
	MapIdentifierInitInfos *initInfos = (MapIdentifierInitInfos*)infos;
	return ZROBJALIGNINFOS_CPYOBJINFOS(initInfos->infos[MapIdentifierInfos_struct]);
}

static void ZRMapIdentifierInfos_validate(void *infos)
{
	MapIdentifierInitInfos *initInfos = (MapIdentifierInitInfos*)infos;
	_MapIdentifierInfos(initInfos->infos, initInfos);
	initInfos->fdestroy = ZROBJINFOS_ISUNKNOWN(initInfos->objInfos)
									? fdestroy_unknown : fdestroy;
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
	ZRIDGeneratorInfos(initInfos->generatorInfos, allocator);
	ZRMapIdentifierInfos_validate(initInfos);
}

void ZRMapIdentifierInfos_fucmp(void *infos_out, zrfucmp fucmp)
{
	MapIdentifierInitInfos *initInfos = (MapIdentifierInitInfos*)infos_out;
	initInfos->fucmp = fucmp;
}

void ZRMapIdentifierInfos_staticStrategy(void *infos_out)
{
	MapIdentifierInitInfos *initInfos = (MapIdentifierInitInfos*)infos_out;
	initInfos->staticStrategy = 1;
	ZRIDGeneratorInfos_staticStrategy(initInfos->generatorInfos);
	ZRMapIdentifierInfos_validate(initInfos);
}

void ZRMapIdentifierStrategy_init(MapIdentifierStrategy *strategy, MapIdentifierInitInfos *infos)
{
	if (ZROBJINFOS_ISUNKNOWN(infos->objInfos))
		*strategy = (MapIdentifierStrategy )
			{ //
			.identifier = (ZRIdentifierStrategy )
				{ //
				.fgetID = fgetID_unknown,
				.fintern = fintern_unknown,
				.ffromID = ffromID_unknown,
				.fobjectP = fobjectP_unknown,

				.fcontains = fcontains,
				.frelease = frelease_unknown,
				.freleaseID = freleaseID_unknown,
				.freleaseAll = freleaseAll_unknown,

				.fdone = fdone_unknown,
				.fdestroy = fdone_unknown,
				} ,
			};
	else
		*strategy = (MapIdentifierStrategy )
			{ //
			.identifier = (ZRIdentifierStrategy )
				{ //
				.fgetID = fgetID,
				.fintern = fintern,
				.ffromID = ffromID,
				.fobjectP = fobjectP,

				.fcontains = fcontains,
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
	ZRInitInfos_t infoBuffer;
	MapIdentifier *mapIdentifier = MAPID(identifier);
	MapIdentifierInitInfos *initInfos = (MapIdentifierInitInfos*)infos;

	bool objInfosUnknown = ZROBJINFOS_ISUNKNOWN(initInfos->objInfos);
	MapIdentifierStrategy *strategy;
	ZRMap *map, *map_ID;
	ZRMemoryPool *pool;

	ZRObjInfos objInfos = objInfosUnknown ? ZRTYPE_OBJINFOS(ZRObjectP) : initInfos->objInfos;

	if (initInfos->staticStrategy)
		strategy = ZRARRAYOP_GET(identifier, 1, initInfos->infos[MapIdentifierInfos_strategy].offset);
	else
		strategy = ZRALLOC(initInfos->allocator, sizeof(MapIdentifierStrategy));

	/* Map init */
	{
		ZRObjInfos init_objInfos = ZRHashTableInfos_objInfos();

		ZRHashTableInfos(infoBuffer, objInfos, ZRTYPE_OBJINFOS(MapBucket), initInfos->fuhash, initInfos->nbfhash, NULL, initInfos->allocator);
		ZRHashTableInfos_fucmp(infoBuffer, initInfos->fucmp);
		ZRHashTableInfos_dereferenceKey(infoBuffer);

		if (initInfos->staticStrategy)
			ZRHashTableInfos_staticStrategy(infoBuffer);

		map = ZRHashTable_new(infoBuffer);

		ZRHashTableInfos(infoBuffer, ZRTYPE_OBJINFOS(ZRID), ZRTYPE_OBJINFOS(MapBucket), NULL, 0, NULL, initInfos->allocator);

		if (initInfos->staticStrategy)
			ZRHashTableInfos_staticStrategy(infoBuffer);

		map_ID = ZRHashTable_new(infoBuffer);
	}
	/* Pool init */
	{
		ZRMPoolDSInfos(infoBuffer, objInfos, initInfos->allocator);

		if (initInfos->staticStrategy)
			ZRMPoolDSInfos_staticStrategy(infoBuffer);

		pool = ZRMPoolDS_new(infoBuffer);
	}
	/* Generator init */
	ZRIDGenerator *generator = ZRARRAYOP_GET(mapIdentifier, 1, initInfos->infos[MapIdentifierInfos_generator].offset);
	ZRIDGenerator_init(generator, initInfos->generatorInfos);

	ZRMapIdentifierStrategy_init(strategy, initInfos);
	*mapIdentifier = (MapIdentifier ) { //
		.identifier = (ZRIdentifier ) { //
			.strategy = (ZRIdentifierStrategy*)strategy,
			},
		.generator = generator,
		.allocator = initInfos->allocator,
		.map = map,
		.map_ID = map_ID,
		.pool = pool,
		.staticStrategy = initInfos->staticStrategy,
		};
}

ZRIdentifier* ZRMapIdentifier_new(void *infos)
{
	MapIdentifierInitInfos *initInfos = (MapIdentifierInitInfos*)infos;
	MapIdentifier *ret = ZRALLOC(initInfos->allocator, initInfos->infos[MapIdentifierInfos_struct].size);

	ZRMapIdentifier_init(MAPID_ID(ret), infos);
	MAPIDSTRATEGY_ID(MAPID_STRATEGY(ret))->fdestroy = initInfos->fdestroy;
	return MAPID_ID(ret);
}
