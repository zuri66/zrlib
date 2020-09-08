/**
 * @author zuri
 * @date mardi 19 novembre 2019, 21:52:23 (UTC+0100)
 */

#include <zrlib/base/Map/Map.h>

void ZRMap_init(ZRMap *map, ZRObjInfos key, ZRObjInfos obj, ZRMapStrategy *strategy)
{
	ZRMAP_INIT(map, key, obj, strategy);
}

void ZRMap_done(ZRMap *map)
{
	ZRMAP_DONE(map);
}

void ZRMap_destroy(ZRMap *map)
{
	ZRMAP_DESTROY(map);
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
	return ZRMAP_REPLACE(map, key, value);
}

bool ZRMap_delete(ZRMap *map, void *key)
{
	return ZRMAP_DELETE(map, key);
}

void* ZRMap_get(ZRMap *map, void *key)
{
	return ZRMAP_GET(map, key);
}
