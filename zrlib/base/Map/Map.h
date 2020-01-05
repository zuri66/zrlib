/**
 * @author zuri
 * @date mardi 19 novembre 2019, 19:40:53 (UTC+0100)
 */

#ifndef ZRMAP_H
#define ZRMAP_H

#include <zrlib/syntax_pad.h>
#include <zrlib/config.h>

#include <stddef.h>
#include <stdbool.h>

// ============================================================================

typedef struct ZRMapS ZRMap;
typedef struct ZRMapStrategyS ZRMapStrategy;

// ============================================================================

struct ZRMapStrategyS
{
	size_t (*fsdataSize)(ZRMap *map);
	size_t (*fstrategySize)();

	/**
	 * (optional)
	 */
	void (*finitMap)(ZRMap *map);

	/*
	 * The insert/delete functions are responsible to update properly the map.nbObj value.
	 */
	void (*fput)(_______ ZRMap *map, void *key, void *value);
	bool (*fputIfAbsent)(ZRMap *map, void *key, void *value);
	bool (*freplace)(___ ZRMap *map, void *key, void *value);

	void* (*fget)(ZRMap *map, void *key);

	bool (*fdelete)(ZRMap *map, void *key);

	/**
	 * Clean the memory used by the map.
	 * The map MUST NOT be used after this call.
	 */
	void (*fdone)(ZRMap *map);

	/**
	 * (optional)
	 */
	void (*fdestroy)(ZRMap *map);
};

struct ZRMapS
{
	size_t keySize;
	size_t objSize;
	size_t nbObj;

	/*
	 * The strategy for memory management and insertion/deletion routines.
	 */
	ZRMapStrategy *strategy;

	/*
	 * Data for Strategy purpose.
	 */
	char sdata[];
};

// ============================================================================

static inline void ZRMAP_INIT(ZRMap *map, size_t keySize, size_t objSize, ZRMapStrategy *strategy)
{
	*map = (ZRMap)
		{
		.keySize = keySize,
			.objSize = objSize,
			.nbObj = 0,
			.strategy = strategy,
		}
	;

	if (strategy->finitMap)
		strategy->finitMap(map);
}

static inline void ZRMAP_DONE(ZRMap *map)
{
	map->strategy->fdone(map);
}

static inline void ZRMAP_DESTROY(ZRMap *map)
{
	map->strategy->fdestroy(map);
}

static inline size_t ZRMAP_NBOBJ(ZRMap *map)
{
	return map->nbObj;
}

static inline size_t ZRMAP_KEYSIZE(ZRMap *map)
{
	return map->keySize;
}

static inline size_t ZRMAP_OBJSIZE(ZRMap *map)
{
	return map->objSize;
}

static inline void* ZRMAP_GET(ZRMap *map, void *key)
{
	return map->strategy->fget(map, key);
}

static inline void ZRMAP_PUT(ZRMap *map, void *key, void *value)
{
	map->strategy->fput(map, key, value);
}

static inline bool ZRMAP_PUTIFABSENT(ZRMap *map, void *key, void *value)
{
	return map->strategy->fputIfAbsent(map, key, value);
}

static inline bool ZRMAP_REPLACE(ZRMap *map, void *key, void *value)
{
	return map->strategy->freplace(map, key, value);
}

static inline bool ZRMAP_DELETE(ZRMap *map, void *key)
{
	return map->strategy->fdelete(map, key);
}

// ============================================================================

void ZRMap_init(ZRMap *map, size_t keySize, size_t objSize, ZRMapStrategy *strategy);
void ZRMap_done(ZRMap *map);
void ZRMap_destroy(ZRMap *map);

size_t ZRMap_nbObj(_ ZRMap *map);
size_t ZRMap_keySize(ZRMap *map);
size_t ZRMap_objSize(ZRMap *map);

void * ZRMap_get(_______ ZRMap *map, void *key);
void _ ZRMap_put(_______ ZRMap *map, void *key, void *value);
bool _ ZRMap_putIfAbsent(ZRMap *map, void *key, void *value);
bool _ ZRMap_replace(___ ZRMap *map, void *key, void *value);
bool _ ZRMap_delete(____ ZRMap *map, void *key);

#endif
