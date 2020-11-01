#include <zrlib/lib/init.h>
#include <zrlib/base/Vector/Vector.h>

#include <zrlib/base/Algorithm/hash.h>
#include <zrlib/base/Allocator/CAllocator.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <zrlib/base/Identifier/MapIdentifier.h>
#include <zrlib/base/Map/HashTable.h>

#include <stdlib.h>
#include <threads.h>

#define X(ID,NAME) { 0, NAME },
ZRServiceID ZRSERVICESID[ZRSERVICES_NB] = {
	ZRSERVICE_XLIST()
};
#undef X

thread_local struct
{
	ZRAllocator allocator;
	ZRMap *identifiers;
	ZRVector *services;
	ZRIdentifier *serviceIdentifier;
} T_DATA;

typedef struct
{
	ZRObjectP service;

} Service;

static size_t hash_obj(void *obj, void *data)
{
	ZRObjectP *objectP = (ZRObjectP*)obj;
	return zrhash_jenkins_one_at_a_time(objectP->object, objectP->infos.size);
}

static int cmp_obj(void *ap, void *bp, void *data)
{
	ZRObjectP *a = (ZRObjectP*)ap;
	ZRObjectP *b = (ZRObjectP*)bp;

	if (!ZROBJINFOS_EQ(a->infos, b->infos))
		return 1;

	return memcmp(a->object, b->object, a->infos.size);
}


static void zrlib_initServices(void)
{
	ZRSERVICE_ID(ZRService_allocator) = zrlib_registerService(ZRSERVICE_NAME(ZRService_allocator), &ZRPTYPE_OBJECTP(&T_DATA.allocator));
}

void zrlib_initCurrentThread(void)
{
	ZRCAllocator_init(&T_DATA.allocator);

	ZRInitInfos_t bufferInfos;

	ZRHashTableInfos(bufferInfos, ZRTYPE_OBJINFOS(ZRObjInfos), ZRTYPE_OBJINFOS(ZRIdentifier*), NULL, 0);
	ZRHashTableInfos_allocator(bufferInfos, &T_DATA.allocator);
	ZRHashTableInfos_staticStrategy(bufferInfos);
	T_DATA.identifiers = ZRHashTable_new(bufferInfos);

	ZRVector2SideStrategyIInfos(bufferInfos, ZRTYPE_OBJINFOS(Service));
	ZRVector2SideStrategyIInfos_allocator(bufferInfos, &T_DATA.allocator);
	T_DATA.services = ZRVector2SideStrategy_new(bufferInfos);

	zrfuhash hash_a[] = { hash_obj };
	ZRMapIdentifierInfos(bufferInfos, ZROBJINFOS_DEF_UNKNOWN(), hash_a, 1);
	ZRMapIdentifierInfos_allocator(bufferInfos, &T_DATA.allocator);
	ZRMapIdentifierInfos_fucmp(bufferInfos, cmp_obj);
	T_DATA.serviceIdentifier = ZRMapIdentifier_new(bufferInfos);

	zrlib_initServices();
}

void zrlib_endCurrentThread(void)
{
	ZRMapKeyVal kv[32];
	size_t nb, offset = 0;

	for (size_t i = 0; i < ZRVECTOR_NBOBJ(T_DATA.services); i++)
	{
		ZRObjectP *object = ZRVECTOR_GET(T_DATA.services, i);
		ZRFREE(&T_DATA.allocator, object->object);
	}
	ZRVector_destroy(T_DATA.services);
	ZRIdentifier_destroy(T_DATA.serviceIdentifier);

	while (0 < (nb = ZRMap_cpyKeyValPtr(T_DATA.identifiers, kv, offset, ZRCARRAY_NBOBJ(kv))))
	{
		offset += nb;

		for (size_t i = 0; i < nb; i++)
			ZRIdentifier_destroy(*(ZRIdentifier**)kv[i].val);
	}
	ZRMap_destroy(T_DATA.identifiers);
}

void zrlib_initObj(void *obj, ZRObjInfos objInfos)
{
	memset(obj, 0, objInfos.size);
}

ZRIdentifier* zrlib_getObjIdentifier(ZRObjInfos objInfos)
{
	void *ref_p;

	if (ZRMap_putIfAbsentThenGet(T_DATA.identifiers, &objInfos, NULL, &ref_p))
	{
		ZRObjInfos infos_infos = ZRMapIdentifierInfos_objInfos();
		alignas(max_align_t) char bufferInfos[infos_infos.size];
		ZRMapIdentifierInfos(bufferInfos, objInfos, NULL, 0);
		ZRMapIdentifierInfos_allocator(bufferInfos, &T_DATA.allocator);
		ZRMapIdentifierInfos_staticStrategy(bufferInfos);
		ZRIdentifier *identifier = ZRMapIdentifier_new(bufferInfos);
		*(ZRIdentifier**)ref_p = identifier;
	}
	return *(ZRIdentifier**)ref_p;
}

ZRObjectP zrlib_getService(char *name)
{
	ZRObjectP object = ZRSTRING_OBJECTP(name);

	if (!ZRIdentifier_contains(T_DATA.serviceIdentifier, &object))
		return ZROBJECTP_DEF0();

	return zrlib_getServiceFromID(ZRIdentifier_getID(T_DATA.serviceIdentifier, &object));
}

ZRObjectP zrlib_getServiceFromID(ZRID id)
{
	if (id >= ZRVECTOR_NBOBJ(T_DATA.services))
		return ZROBJECTP_DEF0();

	Service *service = ZRVECTOR_GET(T_DATA.services, (size_t)id);
	return service->service;
}

ZRID zrlib_registerService(char *name, ZRObjectP *object)
{
	ZRObjectP stringObject = ZRSTRING_OBJECTP(name);
	bool registered = ZRIdentifier_contains(T_DATA.serviceIdentifier, &stringObject);
	ZRID id = ZRIdentifier_getID(T_DATA.serviceIdentifier, &stringObject);
	Service service = { .service = *object };

	service.service.object = ZROBJALLOC(&T_DATA.allocator, object->infos);
	memcpy(service.service.object, object->object, object->infos.size);

	if (registered)
		ZRVECTOR_SET(T_DATA.services, id, &service);
	else
		ZRVECTOR_INSERT(T_DATA.services, id, &service);

	return id;
}

void* zrlib_intern(void *obj, ZRObjInfos objInfos)
{
	ZRIdentifier *identifier = zrlib_getObjIdentifier(objInfos);
	return ZRIdentifier_intern(identifier, obj);
}
