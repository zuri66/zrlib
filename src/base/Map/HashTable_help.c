/**
 * @author zuri
 * @date mardi 19 novembre 2019, 22:37:41 (UTC+0100)
 */

ZRMap* ZRHashTable_alloc(size_t nbfhash, ZRAllocator *allocator)
{
	return ZRALLOC(allocator, sizeof(ZRMap) + ZRHASHTABLEDATA_SIZE(nbfhash));
}

ZRMap* ZRHashTable_create(size_t keySize, size_t keyAlignment, size_t objSize, size_t objAlignment, size_t nbfhash, fhash_t fhash[nbfhash], ZRVector *table, ZRAllocator *allocator)
{
	ZRMapStrategy *strategy = ZRALLOC(allocator, sizeof(ZRHashTableStrategy));
	ZRHashTableStrategy_init(strategy);
	strategy->fdestroy = ZRHashTable_destroy;
	ZRMap *htable = ZRHashTable_alloc(nbfhash, allocator);

	ZRHashTable_init(keySize, keyAlignment, objSize, objAlignment, htable, nbfhash, fhash, table, allocator);
	ZRMap_init(htable, keySize, objSize, strategy);
	return htable;
}

void ZRHashTable_destroy(ZRMap *htable)
{
	ZRMap_done(htable);
	ZRAllocator *allocator = ZRHASHTABLE_DATA(htable)->allocator;
	ZRFREE(allocator, htable->strategy);
	ZRFREE(allocator, htable);
}
