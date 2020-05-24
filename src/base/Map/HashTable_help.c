/**
 * @author zuri
 * @date mardi 19 novembre 2019, 22:37:41 (UTC+0100)
 */

ZRMap* ZRHashTable_alloc(size_t nbfhash, ZRAllocator *allocator)
{
	return ZRALLOC(allocator, ZRHASHTABLEDATA_SIZE(nbfhash));
}

ZRMap* ZRHashTable_create(size_t keySize, size_t keyAlignment, size_t objSize, size_t objAlignment, fhash_t fhash[], size_t nbfhash, ZRVector *table, ZRAllocator *allocator)
{
	ZRMapStrategy *strategy = ZRALLOC(allocator, sizeof(ZRHashTableStrategy));
	ZRHashTableStrategy_init(strategy);
	strategy->fdestroy = ZRHashTable_destroy;
	ZRMap *htable = ZRHashTable_alloc(nbfhash, allocator);

	ZRHashTable_init(keySize, keyAlignment, objSize, objAlignment, htable, fhash,nbfhash,  table, allocator);
	ZRMap_init(htable, keySize, objSize, strategy);
	return htable;
}

void ZRHashTable_destroy(ZRMap *htable)
{
	ZRMap_done(htable);
	ZRAllocator *allocator = ZRHASHTABLE(htable)->allocator;
	ZRFREE(allocator, htable->strategy);
	ZRFREE(allocator, htable);
}
