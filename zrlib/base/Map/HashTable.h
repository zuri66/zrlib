/**
 * @author zuri
 * @date mardi 19 novembre 2019, 19:40:53 (UTC+0100)
 */

#ifndef ZRHASHTABLE_H
#define ZRHASHTABLE_H

#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Map/Map.h>
#include <zrlib/base/Vector/Vector.h>

#define ZRHASHTABLE_CREATEMAXALIGN(KS,OS,NBFH,FH,TB,AL) ZRHashTable_create(KS, alignof(max_align_t), OS, alignof(max_align_t), NBFH FH, TB, AL)

typedef size_t (*fhash_t)(void *key);

ZRMap* ZRHashTable_alloc(size_t nbfhash, ZRAllocator *allocator);
ZRMap* ZRHashTable_create( //
	size_t keySize, size_t keyAlignment, //
	size_t objSize, size_t objAlignment, //
	fhash_t fhash[], //
	size_t nbfhash, //
	ZRVector *table, //
	ZRAllocator *allocator //
	);
size_t ZRHashTable_bucketSize(size_t keySize, size_t valueSize);

void ZRHashTable_destroy(ZRMap *htable);

#endif
