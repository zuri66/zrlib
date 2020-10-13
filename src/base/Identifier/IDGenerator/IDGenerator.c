#include <zrlib/base/Identifier/IDGenerator/IDGenerator.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>
#include <zrlib/base/ArrayOp.h>

typedef struct IDGeneratorS IDGenerator;

struct IDGeneratorS
{
	ZRIDGenerator generator;
	ZRVector *unusedIDs;
	ZRID nextID;

	ZRAllocator *allocator;
	void (*fdestroy)(IDGenerator*);
};

#define IDGENERATOR(G) (IDGenerator*)(G)
#define IDGENERATOR_ZR(G) (&(G)->generator)

typedef struct
{
	ZRAllocator *allocator;
	unsigned staticStrategy :1;
	unsigned changefdestroy;
} IDGeneratorInitInfos;

static int cmpID(void *a, void *b, void *data)
{
	return *(ZRID*)a - *(ZRID*)b;
}

/* ========================================================================= */

size_t ZRIDGenerator_nbGenerated(ZRIDGenerator *generator)
{
	return ZRIDGENERATOR_NBGENERATED(generator);
}

ZRID ZRIDGenerator_nextID(ZRIDGenerator *generator_p)
{
	IDGenerator *generator = IDGENERATOR(generator_p);
	return generator->nextID;
}

bool ZRIDGenerator_present(ZRIDGenerator *generator_p, ZRID id)
{
	IDGenerator *generator = IDGENERATOR(generator_p);
	ZRVector *unusedIDs = generator->unusedIDs;
	return id < generator->nextID && SIZE_MAX == ZRARRAYOP_BSEARCH_POS(ZRVECTOR_ARRAYP(unusedIDs), ZRVECTOR_OBJSIZE(unusedIDs), ZRVECTOR_NBOBJ(unusedIDs), &id, cmpID, NULL);
}

ZRID ZRIDGenerator_generate(ZRIDGenerator *generator_p)
{
	IDGenerator *generator = IDGENERATOR(generator_p);
	IDGENERATOR_ZR(generator)->nbGenerated++;

	if (0 < ZRVECTOR_NBOBJ(generator->unusedIDs))
	{
		ZRID ret;
		ZRVector_popFirst(generator->unusedIDs, &ret);
		return ret;
	}
	return generator->nextID++;
}

void ZRIDGenerator_release(ZRIDGenerator *generator_p, ZRID id)
{
	IDGenerator *generator = IDGENERATOR(generator_p);
	ZRVector *unusedIDs = generator->unusedIDs;

	if (id >= generator->nextID)
		return;

	ZRID lastID = generator->nextID - 1;

	if (id == lastID)
	{
		generator->nextID--;
		lastID--;
		size_t pos = ZRVECTOR_NBOBJ(unusedIDs) - 1;

		for (;;)
		{
			id = *(ZRID*)ZRVECTOR_GET(unusedIDs, pos);

			if (lastID != id)
				break;

			ZRVECTOR_DEC(unusedIDs);
			generator->nextID--;
			lastID--;

			if (pos == 0)
				break;

			pos--;
		}
	}
	else
	{
		size_t pos = ZRARRAYOP_BINSERT_POS_FIRST(ZRVECTOR_ARRAYP(unusedIDs), ZRVECTOR_OBJSIZE(generator->unusedIDs), ZRVECTOR_NBOBJ(generator->unusedIDs), &id, cmpID, NULL);
		ZRVECTOR_INSERT(unusedIDs, pos, &id);
	}
	IDGENERATOR_ZR(generator)->nbGenerated--;
}

void ZRIDGenerator_releaseAll(ZRIDGenerator *generator_p)
{
	IDGenerator *generator = IDGENERATOR(generator_p);
	generator->nextID = 0;
	IDGENERATOR_ZR(generator)->nbGenerated = 0;
	ZRVECTOR_DELETE_ALL(generator->unusedIDs);
}

static void fdone(IDGenerator *generator)
{
	ZRVECTOR_DESTROY(generator->unusedIDs);
}

static void fdestroy(IDGenerator *generator)
{
	fdone(generator);
	ZRFREE(generator->allocator, generator);
}

void ZRIDGenerator_destroy(ZRIDGenerator *generator_p)
{
	IDGenerator *generator = IDGENERATOR(generator_p);
	generator->fdestroy(generator);
}

/* ========================================================================= */

ZRObjInfos ZRIDGeneratorInfos_objInfos(void)
{
	return ZRTYPE_OBJINFOS(IDGeneratorInitInfos);
}

void ZRIDGeneratorInfos(void *infos_p, ZRAllocator *allocator)
{
	IDGeneratorInitInfos *infos = (IDGeneratorInitInfos*)infos_p;
	*infos = (IDGeneratorInitInfos ) { /**/
		.allocator = allocator,
		};
}

void ZRIDGeneratorInfos_staticStrategy(void *infos_p)
{
	IDGeneratorInitInfos *infos = (IDGeneratorInitInfos*)infos_p;
	infos->staticStrategy = 1;
}

ZRObjInfos ZRIDGenerator_objInfos(void *infos)
{
	return ZRTYPE_OBJINFOS(IDGenerator);
}

void ZRIDGenerator_init(ZRIDGenerator *generator_p, void *infos_p)
{
	IDGeneratorInitInfos *infos = (IDGeneratorInitInfos*)infos_p;
	IDGenerator *generator = IDGENERATOR(generator_p);

	alignas(max_align_t) char vectorInfos[ZRVector2SideStrategyInfos_objInfos().size];
	ZRVector2SideStrategyInfos(vectorInfos, 200, 500, ZRTYPE_SIZE_ALIGNMENT(ZRID), infos->allocator, false);

	if (infos->staticStrategy)
		ZRVector2SideStrategyInfos_staticStrategy(vectorInfos);

	*generator = (IDGenerator ) { /**/
		.unusedIDs = ZRVector2SideStrategy_new(vectorInfos),
		.allocator = infos->allocator,
		.fdestroy = (infos->changefdestroy ? fdestroy : fdone),
		};
}

ZRIDGenerator* ZRIDGenerator_new(void *infos_p)
{
	IDGeneratorInitInfos *infos = (IDGeneratorInitInfos*)infos_p;
	ZRIDGenerator *generator = ZRALLOC(infos->allocator, sizeof(IDGenerator));

	infos->changefdestroy = 1;
	ZRIDGenerator_init(generator, infos);
	infos->changefdestroy = 0;

	return generator;
}
