/**
 * @author zuri
 * @date mardi 19 novembre 2019, 22:37:41 (UTC+0100)
 */

ZRMap* ZRHashTable_alloc(size_t nbfhash, ZRAllocator *allocator)
{
	TYPEDEF_SDATA(nbfhash);
	return ZRALLOC(allocator, sizeof(ZRMap) + sizeof(ZRHashTableData));
}

size_t ZRHashTable_bucketSize(size_t keySize, size_t valueSize)
{
	return sizeof(STRUCT_BUCKET(keySize, valueSize));
}

ZRMap* ZRHashTable_create(size_t keySize, size_t objSize, size_t nbfhash, fhash_t fhash[nbfhash], ZRVector *table, void (*ftable_destroy)(ZRVector*), ZRAllocator *allocator)
{
	if (table == NULL)
	{
		table = ZRVector2SideStrategy_createDynamic(1024, ZRHashTable_bucketSize(keySize, objSize), allocator);
		ftable_destroy = ZRVector2SideStrategy_destroy;
	}
	ZRMapStrategy *strategy = ZRALLOC(allocator, sizeof(ZRHashTableStrategy));
	ZRHashTableStrategy_init(strategy, ftable_destroy, allocator);
	ZRMap *htable = ZRHashTable_alloc(nbfhash, allocator);

	ZRHashTable_init(htable, nbfhash, fhash, table);
	ZRMap_init(htable, keySize, objSize, strategy);
	return htable;
}

void ZRHashTable_destroy(ZRMap *htable)
{
	ZRMap_done(htable);
	ZRAllocator *allocator = ZRHASHTABLE_STRATEGY(htable)->allocator;
	ZRFREE(allocator, htable->strategy);
	ZRFREE(allocator, htable);
}
