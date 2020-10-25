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

ZRObjInfos ZRHashTableInfos_objInfos(void);
ZRObjInfos ZRHashTable_objInfos(void *infos);

void ZRHashTableInfos( //
	void *infos_out, //
	ZRObjInfos key, ZRObjInfos obj,
	zrfuhash fhash[], //
	size_t nbfhash
	);
void ZRHashTableInfos_allocator(void *infos_out, ZRAllocator *allocator);
void ZRHashTableInfos_dereferenceKey(void *infos_out);
void ZRHashTableInfos_staticStrategy(void *infos_out);
void ZRHashTableInfos_fucmp(void *infos_out, zrfucmp fucmp);

void ZRHashTable_init(ZRMap *map, void *initInfos);
ZRMap* ZRHashTable_new(void *initInfos);

ZRMap* ZRHashTable_create(
	ZRObjInfos key, ZRObjInfos obj,
	zrfuhash fhash[], //
	size_t nbfhash, //
	ZRAllocator *allocator //
	);

#endif
