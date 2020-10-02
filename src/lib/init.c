#include <zrlib/lib/init.h>
#include <zrlib/base/Vector/Vector.h>

#include <zrlib/base/Allocator/CAllocator.h>
#include <zrlib/base/Vector/Vector2SideStrategy.h>

#include <zrlib/base/Identifier/MapIdentifier.h>
#include <zrlib/base/Map/HashTable.h>

#include <stdlib.h>
#include <threads.h>

thread_local struct
{
	ZRAllocator allocator;
	ZRMap *identifiers;
} T_DATA;

void zrlib_initCurrentThread(void)
{
	ZRCAllocator_init(&T_DATA.allocator);
	ZRObjInfos infos_infos = ZRHashTableInfos_objInfos();
	alignas(max_align_t) char bufferInfos[infos_infos.size];
	ZRHashTableInfos(bufferInfos, ZRTYPE_OBJINFOS(ZRObjInfos), ZRTYPE_OBJINFOS(ZRIdentifier*), NULL, 0, NULL, &T_DATA.allocator);
	ZRHashTableInfos_staticStrategy(bufferInfos);
	T_DATA.identifiers = ZRHashTable_new(bufferInfos);
}

void zrlib_endCurrentThread(void)
{
	ZRMapKeyVal kv[32];
	size_t nb, offset = 0;

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
		ZRMapIdentifierInfos(bufferInfos, objInfos, NULL, 0, &T_DATA.allocator);
		ZRMapIdentifierInfos_staticStrategy(bufferInfos);
		ZRIdentifier *identifier = ZRMapIdentifier_new(bufferInfos);
		*(ZRIdentifier**)ref_p = identifier;
	}
	return *(ZRIdentifier**)ref_p;
}

void* zrlib_intern(void *obj, ZRObjInfos objInfos)
{
	ZRIdentifier *identifier = zrlib_getObjIdentifier(objInfos);
	return ZRIdentifier_intern(identifier, obj);
}
