/**
 * @author zuri
 * @date mardi 19 novembre 2019, 19:40:53 (UTC+0100)
 */

#ifndef ZRMAP_H
#define ZRMAP_H

#include <zrlib/config.h>
#include <zrlib/base/struct.h>

#include <stdalign.h>
#include <stddef.h>
#include <stdbool.h>

// ============================================================================

typedef struct ZRMapS ZRMap;
typedef struct ZRMapStrategyS ZRMapStrategy;

// ============================================================================

struct ZRMapStrategyS
{
	/**
	 * (optional)
	 */
	void (*finitMap)(ZRMap *map);

	/*
	 * The insert/delete functions are responsible to update properly the map.nbObj value.
	 *
	 * @parameter
	 * **out a pointer to the added object, may be NULL
	 */
	void (*fput)(_______ ZRMap *map, void *key, void *value, void **out);
	bool (*fputIfAbsent)(ZRMap *map, void *key, void *value, void **out);
	bool (*freplace)(___ ZRMap *map, void *key, void *value, void **out);

	void* (*fget)(ZRMap *map, void *key);

	bool (*fdelete)(ZRMap *map, void *key, void *cpy_out);
	void (*fdeleteAll)(ZRMap *map);

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
	ZRObjInfos keyInfos;
	ZRObjInfos objInfos;
	size_t nbObj;

	/*
	 * The strategy for memory management and insertion/deletion routines.
	 */
	ZRMapStrategy *strategy;
};

#define ZRMAP(M) ((ZRMap*)(M))

// ============================================================================

ZRMUSTINLINE
static inline void ZRMAP_INIT(ZRMap *map, ZRObjInfos key, ZRObjInfos obj, ZRMapStrategy *strategy)
{
	ZRMap cpy = { //
		.keyInfos = key,
		.objInfos = obj,
		.nbObj = 0,
		.strategy = strategy,
		}
	;
	*map = cpy;

	if (strategy->finitMap)
		strategy->finitMap(map);
}

ZRMUSTINLINE
static inline void ZRMAP_DONE(ZRMap *map)
{
	map->strategy->fdone(map);
}

ZRMUSTINLINE
static inline void ZRMAP_DESTROY(ZRMap *map)
{
	map->strategy->fdestroy(map);
}

ZRMUSTINLINE
static inline void* ZRMAP_GET(ZRMap *map, void *key)
{
	return map->strategy->fget(map, key);
}

ZRMUSTINLINE
static inline void ZRMAP_PUT(ZRMap *map, void *key, void *value)
{
	map->strategy->fput(map, key, value, NULL);
}

ZRMUSTINLINE
static inline bool ZRMAP_PUTIFABSENT(ZRMap *map, void *key, void *value)
{
	return map->strategy->fputIfAbsent(map, key, value, NULL);
}

ZRMUSTINLINE
static inline bool ZRMAP_REPLACE(ZRMap *map, void *key, void *value)
{
	return map->strategy->freplace(map, key, value, NULL);
}

ZRMUSTINLINE
static inline void ZRMAP_PUTTHENGET(ZRMap *map, void *key, void *value, void **out)
{
	map->strategy->fput(map, key, value, out);
}

ZRMUSTINLINE
static inline bool ZRMAP_PUTIFABSENTTHENGET(ZRMap *map, void *key, void *value, void **out)
{
	return map->strategy->fputIfAbsent(map, key, value, out);
}

ZRMUSTINLINE
static inline bool ZRMAP_REPLACETHENGET(ZRMap *map, void *key, void *value, void **out)
{
	return map->strategy->freplace(map, key, value, out);
}

ZRMUSTINLINE
static inline bool ZRMAP_CPYTHENDELETE(ZRMap *map, void *key, void *cpy_out)
{
	return map->strategy->fdelete(map, key, cpy_out);
}

ZRMUSTINLINE
static inline bool ZRMAP_DELETE(ZRMap *map, void *key)
{
	return map->strategy->fdelete(map, key, NULL);
}

ZRMUSTINLINE
static inline void ZRMAP_DELETEALL(ZRMap *map)
{
	map->strategy->fdeleteAll(map);
}

// Help

ZRMUSTINLINE
static inline size_t ZRMAP_NBOBJ(ZRMap *map)
{
	return map->nbObj;
}

ZRMUSTINLINE
static inline size_t ZRMAP_KEYSIZE(ZRMap *map)
{
	return map->keyInfos.size;
}

ZRMUSTINLINE
static inline size_t ZRMAP_OBJSIZE(ZRMap *map)
{
	return map->objInfos.size;
}

// ============================================================================

void ZRMap_init(ZRMap *map, ZRObjInfos key, ZRObjInfos obj, ZRMapStrategy *strategy);
void ZRMap_done(ZRMap *map);
void ZRMap_destroy(ZRMap *map);

void ZRMap_putThenGet(_______ ZRMap *map, void *key, void *value, void **out);
bool ZRMap_putIfAbsentThenGet(ZRMap *map, void *key, void *value, void **out);
bool ZRMap_replaceThenGet(___ ZRMap *map, void *key, void *value, void **out);
bool ZRMap_cpyThenDelete(____ ZRMap *map, void *key, void *cpy_out);

void* ZRMap_get(_______ ZRMap *map, void *key);
void _ ZRMap_put(_______ ZRMap *map, void *key, void *value);
bool _ ZRMap_putIfAbsent(ZRMap *map, void *key, void *value);
bool _ ZRMap_replace(___ ZRMap *map, void *key, void *value);
bool _ ZRMap_delete(____ ZRMap *map, void *key);
void _ ZRMap_deleteAll(_ ZRMap *map);

#endif
