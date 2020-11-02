/**
 * @author zuri
 * @date mardi 19 novembre 2019, 19:40:53 (UTC+0100)
 */

#ifndef ZRHASHTABLE_H
#define ZRHASHTABLE_H

#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/ResizeOp.h>
#include <zrlib/base/Map/Map.h>
#include <zrlib/base/Vector/Vector.h>
#include <zrlib/base/struct.h>

// ============================================================================

void ZRHashTable_growStrategy(ZRMap *map, zrflimit fupLimit, zrfincrease fincrease);
void ZRHashTable_shrinkStrategy(ZRMap *map, zrflimit fdownLimit, zrfdecrease fdecrease);

ZRObjInfos ZRHashTableIInfosObjInfos(void);

void ZRHashTableIInfos( //
	void *iinfos, //
	ZRObjInfos key, ZRObjInfos obj,
	zrfuhash fhash[], //
	size_t nbfhash
	);
void ZRHashTableIInfos_allocator(void *iinfos, ZRAllocator *allocator);
void ZRHashTableIInfos_dereferenceKey(void *iinfos);
void ZRHashTableIInfos_staticStrategy(void *iinfos);
void ZRHashTableIInfos_fucmp(void *iinfos, zrfucmp fucmp);

ZRObjInfos ZRHashTable_objInfos(void *iinfos);
void ZRHashTable_init(ZRMap *map, void *iinfos);
ZRMap* ZRHashTable_new(void *iinfos);

ZRMap* ZRHashTable_create(
	ZRObjInfos key, ZRObjInfos obj,
	zrfuhash fhash[], //
	size_t nbfhash, //
	ZRAllocator *allocator //
	);

#endif
