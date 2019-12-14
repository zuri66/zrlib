/**
 * @author zuri
 * @date mardi 19 novembre 2019, 19:40:53 (UTC+0100)
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <zrlib/base/Allocator/Allocator.h>
#include <zrlib/base/Map/Map.h>
#include <zrlib/base/Vector/Vector.h>

typedef size_t (*fhash_t)(void *key);

ZRMap* ZRHashTable_alloc(size_t nbfhash, ZRAllocator *allocator);
ZRMap* ZRHashTable_create( //
	size_t keySize, //
	size_t valueSize, //
	size_t nbfhash, //
	fhash_t fhash[nbfhash], //
	ZRVector *table, //
	void (*ftable_destroy)(ZRVector*), //
	ZRAllocator *allocator //
	);
size_t ZRHashTable_bucketSize(size_t keySize, size_t valueSize);

void ZRHashTable_destroy(ZRMap *htable);

#endif
