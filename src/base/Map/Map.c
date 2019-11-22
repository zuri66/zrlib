/**
 * @author zuri
 * @date mardi 19 novembre 2019, 21:52:23 (UTC+0100)
 */

#include <zrlib/base/Map/Map.h>

void ZRMap_init(ZRMap *map, size_t keySize, size_t objSize, ZRMapStrategy *strategy)
{
	ZRMAP_INIT(map, keySize, objSize, strategy);
}

void ZRMap_done(ZRMap *map)
{
	ZRMAP_DONE(map);
}

size_t ZRMap_nbObj(ZRMap *map)
{
	return ZRMAP_NBOBJ(map);
}

size_t ZRMap_keySize(ZRMap *map)
{
	return ZRMAP_KEYSIZE(map);
}

size_t ZRMap_objSize(ZRMap *map)
{
	return ZRMAP_OBJSIZE(map);
}

void ZRMap_put(ZRMap *map, void *key, void *value)
{
	ZRMAP_PUT(map, key, value);
}

bool ZRMap_putIfAbsent(ZRMap *map, void *key, void *value)
{
	return ZRMAP_PUTIFABSENT(map, key, value);
}

bool ZRMap_replace(ZRMap *map, void *key, void *value)
{
	ZRMAP_REPLACE(map, key, value);
}

bool ZRMap_delete(ZRMap *map, void *key)
{
	return ZRMAP_DELETE(map, key);
}

void* ZRMap_get(ZRMap *map, void *key)
{
	return ZRMAP_GET(map, key);
}
